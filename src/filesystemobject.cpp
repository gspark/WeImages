#include "filesystemobject.h"
#include "util\hash\fasthash.h"
#include "filesystemhelperfunctions.h"
#include "util\qdatetime_helpers.hpp"

#ifdef CFILESYSTEMOBJECT_TEST
#define QFileInfo QFileInfo_Test
#define QDir QDir_Test
#endif

#include <assert.h>
#include <errno.h>

#if defined __linux__ || defined __APPLE__ || defined __FreeBSD__
#include <unistd.h>
#include <sys/stat.h>
#include <wordexp.h>
#include <dirent.h>
#elif defined _WIN32

#include <Windows.h>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib") // This lib would have to be added not just to the top level application, but every plugin as well, so using #pragma instead
#endif

static QString expandEnvironmentVariables(const QString& string)
{
#ifdef _WIN32
    if (!string.contains('%'))
        return string;

    WCHAR source[16384 + 1];
    WCHAR result[16384 + 1];

    static_assert (sizeof(WCHAR) == 2);
    const auto length = string.toWCharArray(source);
    source[length] = 0;
    if (const auto resultLength = ExpandEnvironmentStringsW(source, result, static_cast<DWORD>(std::size(result))); resultLength > 0)
        return toPosixSeparators(QString::fromWCharArray(result, resultLength - 1));
    else
        return string;
#else
    QString result = string;
    if (result.startsWith('~'))
        result.replace(0, 1, getenv("HOME"));

    if (result.contains('$'))
    {
        wordexp_t p;
        wordexp("$HOME/bin", &p, 0);
        const auto w = p.we_wordv;
        if (p.we_wordc > 0)
            result = w[0];

        wordfree(&p);
    }

    return result;
#endif
}

FileSystemObject::FileSystemObject(const QFileInfo &fileInfo) : _fileInfo(fileInfo) {
    refreshInfo();
}

FileSystemObject::FileSystemObject(const QString& path) : _fileInfo(expandEnvironmentVariables(path))
{
    refreshInfo();
}

static QString parentForAbsolutePath(QString absolutePath) {
    if (absolutePath.endsWith('/'))
        absolutePath.chop(1);

    const int lastSlash = absolutePath.lastIndexOf('/');
    if (lastSlash <= 0)
        return {};

    absolutePath.truncate(lastSlash + 1); // Keep the slash as it signifies a directory rather than a file.
    return absolutePath;
}

FileSystemObject &FileSystemObject::operator=(const QString &path) {
    setPath(path);
    return *this;
}

void FileSystemObject::refreshInfo() {
#ifndef _WIN32
    // TODO: is this always correct?
    // Should there be a special "Symlink" object type? Then it could be handled properly (e. g. delete = unlink)
    _properties.exists = !_fileInfo.isSymLink() ? _fileInfo.exists() : true;
#else
    _properties.exists = _fileInfo.exists();
#endif

    _properties.fullPath = _fileInfo.absoluteFilePath();

    // QFileInfo::isShortcut() is quiate a heavy call on Windows - disabled temporarily for better performance enumerating large folders
    // Time to first update for C:\Windows\WinSxS\ goes from 1900 to 3900 ms

    if (_fileInfo.isShortcut()) // This is Windows-specific, place under #ifdef?
    {
        _properties.exists = true;
        _properties.type = File;
    }
    else if (_fileInfo.isFile()) {
        _properties.type = File;
    } else if (_fileInfo.isDir()) {
        // Normalization - very important for hash calculation and equality checking
        // C:/1/ must be equal to C:/1
        if (!_properties.fullPath.endsWith('/'))
            _properties.fullPath.append('/');

#ifdef __APPLE__
        _properties.type = _fileInfo.isBundle() ? Bundle : Directory;
#else
        _properties.type = Directory;
#endif
    } else if (!_properties.exists && _properties.fullPath.endsWith('/'))
        _properties.type = Directory;
    else {
#ifdef _WIN32
        if (_properties.exists)
            qInfo() << _properties.fullPath << " is neither a file nor a dir";
#else
        // TODO: is this always correct?
        // Should there be a special "Symlink" object type? Then it could be handled properly (e. g. delete = unlink)
        if (_fileInfo.isSymLink())
            _properties.type = File;
#endif
    }

    // TODO
    _properties.hash = fasthash64(_properties.fullPath.constData(),
                                  static_cast<uint64_t>(_properties.fullPath.size()) * sizeof(QChar), 0);
#if 1
    if (_properties.type == File) {
        _properties.extension = _fileInfo.suffix();
        _properties.completeBaseName = _fileInfo.completeBaseName();
    } else if (_properties.type == Directory) {
        _properties.completeBaseName = _fileInfo.baseName();
        const QString suffix = _fileInfo.completeSuffix();
        if (!suffix.isEmpty())
            _properties.completeBaseName = _properties.completeBaseName % '.' % suffix;

        // Ugly temporary bug fix for #141
        if (_properties.completeBaseName.isEmpty() && _properties.fullPath.endsWith('/')) {
            const QFileInfo tmpInfo = QFileInfo(_properties.fullPath.left(_properties.fullPath.length() - 1));
            _properties.completeBaseName = tmpInfo.baseName();
            const QString sfx = tmpInfo.completeSuffix();
            if (!sfx.isEmpty())
                _properties.completeBaseName = _properties.completeBaseName % '.' % sfx;
        }
    } else if (_properties.type == Bundle) {
        _properties.extension = _fileInfo.suffix();
        _properties.completeBaseName = _fileInfo.completeBaseName();
    }

    _properties.fullName = _properties.type == Directory ? _properties.completeBaseName : _fileInfo.fileName();
#endif
    _properties.isCdUp = _properties.fullName == QLatin1String("..");
#if 1
    // QFileInfo::canonicalPath() / QFileInfo::absolutePath are undefined for non-files
    _properties.parentFolder = parentForAbsolutePath(_properties.fullPath);

    if (!_properties.exists)
        return;

    _properties.creationDate = static_cast<time_t>(_fileInfo.birthTime().toMSecsSinceEpoch() / 1000LL);
    _properties.modificationDate = static_cast<time_t>(_fileInfo.lastModified().toMSecsSinceEpoch() / 1000LL);
    _properties.size = _properties.type == File ? static_cast<uint64_t>(_fileInfo.size()) : 0ULL;
#endif
}

void FileSystemObject::setPath(const QString &path) {
    if (path.isEmpty()) {
        *this = FileSystemObject();
        return;
    }

    _rootFileSystemId = UINT64_MAX;

    _fileInfo.setFile(expandEnvironmentVariables(path));

    refreshInfo();
}

bool FileSystemObject::operator==(const FileSystemObject &other) const {
    return hash() == other.hash();
}


// Information about this object
bool FileSystemObject::isValid() const {
    return hash() != 0;
}

bool FileSystemObject::exists() const {
    return _properties.exists;
}

const FileSystemObjectProperties &FileSystemObject::properties() const {
    return _properties;
}

FileSystemObjectType FileSystemObject::type() const {
    return _properties.type;
}

bool FileSystemObject::isFile() const {
    return _properties.type == File;
}

bool FileSystemObject::isDir() const {
    return _properties.type == Directory || _properties.type == Bundle;
}

bool FileSystemObject::isBundle() const {
    return _properties.type == Bundle;
}

bool FileSystemObject::isEmptyDir() const {
    if (!isDir())
        return false;

#ifdef _WIN32
    WCHAR path[32768];
    const int nChars = _properties.fullPath.toWCharArray(path);
    path[nChars] = 0;
    return PathIsDirectoryEmptyW(path) != 0;
#else
    // TODO: use getdents64 on Linux
    DIR *dir = ::opendir(_properties.fullPath.toLocal8Bit().constData());
    if (dir == nullptr) // Not a directory or doesn't exist
        return false;

    struct dirent *d = nullptr;
    int n = 0;
    while ((d = ::readdir(dir)) != nullptr)
    {
        if(++n > 2)
            break;
    }

    ::closedir(dir);
    return n <= 2; // Only '.' and '..' ?
#endif
}

bool FileSystemObject::isCdUp() const {
    return _properties.isCdUp;
}

bool FileSystemObject::isExecutable() const {
    return _fileInfo.permission(QFile::ExeUser) || _fileInfo.permission(QFile::ExeOwner) ||
           _fileInfo.permission(QFile::ExeGroup) || _fileInfo.permission(QFile::ExeOther);
}

bool FileSystemObject::isReadable() const {
    return _fileInfo.isReadable();
}

// Apparently, it will return false for non-existing files
bool FileSystemObject::isWriteable() const {
    return _fileInfo.isWritable();
}

bool FileSystemObject::isHidden() const {
    return _fileInfo.isHidden();
}

QString FileSystemObject::fullAbsolutePath() const {
    assert(_properties.type != Directory || _properties.fullPath.isEmpty() || _properties.fullPath.endsWith('/'));
    return _properties.fullPath;
}

QString FileSystemObject::parentDirPath() const {
    assert(_properties.parentFolder.isEmpty() || _properties.parentFolder.endsWith('/'));
    return _properties.parentFolder;
}

uint64_t FileSystemObject::size() const {
    return _properties.size;
}

uint64_t FileSystemObject::hash() const {
    return _properties.hash;
}

const QFileInfo &FileSystemObject::qFileInfo() const {
    return _fileInfo;
}

uint64_t FileSystemObject::rootFileSystemId() const {
    if (_rootFileSystemId == UINT64_MAX) {
#ifdef _WIN32
        WCHAR drivePath[32768];
        const auto pathLength = _properties.fullPath.toWCharArray(drivePath);
        drivePath[pathLength] = 0;
        const auto driveNumber = PathGetDriveNumberW(drivePath);
        if (driveNumber != -1)
            _rootFileSystemId = static_cast<uint64_t>(driveNumber);
#else
        struct stat info;
        const int ret = stat(_properties.fullPath.toUtf8().constData(), &info);
        if (ret == 0 || errno == ENOENT)
            _rootFileSystemId = (uint64_t) info.st_dev;
        else
        {
            qInfo() << __FUNCTION__ << "Failed to query device ID for" << _properties.fullPath;
            qInfo() << strerror(errno);
        }
#endif
    }

    return _rootFileSystemId;
}

bool FileSystemObject::isNetworkObject() const {
#ifdef _WIN32
    if (!_properties.fullPath.startsWith("//"))
        return false;

    WCHAR wPath[MAX_PATH];
    const int nSymbols = toNativeSeparators(_properties.fullPath).toWCharArray(wPath);
    wPath[nSymbols] = 0;
    return PathIsNetworkPathW(wPath);
#else
    return false;
#endif
}

bool FileSystemObject::isSymLink() const {
    return _fileInfo.isSymLink();
}

QString FileSystemObject::symLinkTarget() const {
    return _fileInfo.symLinkTarget();
}

bool FileSystemObject::isMovableTo(const FileSystemObject &dest) const {
    if (!isValid() || !dest.isValid())
        return false;

    const auto fileSystemId = rootFileSystemId();
    const auto otherFileSystemId = dest.rootFileSystemId();

    return fileSystemId == otherFileSystemId && fileSystemId != UINT64_MAX;
}

// A hack to store the size of a directory after it's calculated
void FileSystemObject::setDirSize(uint64_t size) {
    _properties.size = size;
}

// File name without suffix, or folder name. Same as QFileInfo::completeBaseName.
QString FileSystemObject::name() const {
    return _properties.completeBaseName;
}

// Filename + suffix for files, same as name() for folders
QString FileSystemObject::fullName() const {
    return _properties.fullName;
}

QString FileSystemObject::extension() const {
    return _properties.extension;
}

QString FileSystemObject::sizeString() const {
    return _properties.type == File ? fileSizeToString(_properties.size) : QString();
}

QString FileSystemObject::modificationDateString() const {
    return fromTime_t(_properties.modificationDate).toLocalTime().toString(QLatin1String("dd.MM.yyyy hh:mm"));
}


// Return the list of consecutive full paths leading from the specified target to its root.
// E. g. C:/Users/user/Documents/ -> {C:/Users/user/Documents/, C:/Users/user/, C:/Users/, C:/}
std::vector<QString> pathHierarchy(const QString &path) {

    if (path.isEmpty())
        return {};
    else if (path == '/')
        return {path};

    QString pathItem = path.endsWith('/') ? path.left(path.length() - 1) : path;
    std::vector<QString> result{path};
    while ((pathItem = QFileInfo(pathItem).absolutePath()).length() < result.back().length()) {
        if (pathItem.endsWith('/'))
            result.emplace_back(pathItem);
        else
            result.emplace_back(pathItem + '/');
    }

    return result;
}
