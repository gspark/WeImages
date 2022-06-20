#pragma once

#ifdef CFILESYSTEMOBJECT_TEST
#define QFileInfo QFileInfo_Test
#define QDir QDir_Test

#include <QDir_Test>
#include <QFileInfo_Test>
#else

#include <QDir>
#include <QFileInfo>

#endif

#include <QString>
#include <QStringBuilder>

#include <cstdint>
#include <vector>

// Return the list of consecutive full paths leading from the specified target to its root.
// E. g. C:/Users/user/Documents/ -> {C:/Users/user/Documents/, C:/Users/user/, C:/Users/, C:/}
std::vector<QString> pathHierarchy(
        const QString &path); // Keeping this function here because it's covered by a FileSystemObject test and needs the QFileInfo_Test include

enum FileSystemObjectType {
    UnknownType, Directory, File, Bundle
};

struct FileSystemObjectProperties {
    uint64_t size = 0;
    uint64_t hash = 0;
    QString completeBaseName;
    QString extension;
    QString fullName;
    QString parentFolder;
    QString fullPath;
    time_t creationDate = std::numeric_limits<time_t>::max();
    time_t modificationDate = std::numeric_limits<time_t>::max();
    FileSystemObjectType type = UnknownType;
    bool isCdUp = false;
    bool exists = false;
};

class FileSystemObject {
public:
    FileSystemObject() = default;

    FileSystemObject(FileSystemObject &&) noexcept = default;

    FileSystemObject(const FileSystemObject &) = default;

    explicit FileSystemObject(const QFileInfo &fileInfo);

    explicit FileSystemObject(const QString& path);

    inline explicit FileSystemObject(const QDir &dir) : FileSystemObject(QString(dir.absolutePath())) {}

    template<typename T, typename U>
    explicit FileSystemObject(QStringBuilder<T, U> &&stringBuilder) : FileSystemObject(
            (QString) std::forward<QStringBuilder<T, U>>(stringBuilder)) {}

    FileSystemObject &operator=(FileSystemObject &&) = default;

    FileSystemObject &operator=(const FileSystemObject &) = default;

    FileSystemObject &operator=(const QString &path);

    virtual void refreshInfo();

    void setPath(const QString &path);

    bool operator==(const FileSystemObject &other) const;

// Information about this object
    bool isValid() const;

    virtual bool exists() const;

    const FileSystemObjectProperties &properties() const;

    FileSystemObjectType type() const;

    virtual bool isFile() const;

    virtual bool isDir() const;

    bool isBundle() const;

    bool isEmptyDir() const;

    bool isCdUp() const; // returns true if it's ".." item
    bool isExecutable() const;

    bool isReadable() const;

    // Apparently, it will return false for non-existing files
    bool isWriteable() const;

    virtual bool isHidden() const;

    QString fullAbsolutePath() const;

    QString parentDirPath() const;

    uint64_t size() const;

    virtual uint64_t hash() const;

    const QFileInfo &qFileInfo() const;

    uint64_t rootFileSystemId() const;

    bool isNetworkObject() const;

    bool isSymLink() const;

    QString symLinkTarget() const;

    bool isMovableTo(const FileSystemObject &dest) const;

    // A hack to store the size of a directory after it's calculated
    void setDirSize(uint64_t size);

    // File name without suffix, or folder name. Same as QFileInfo::completeBaseName.
    QString name() const;

    // Filename + suffix for files, same as name() for folders
    QString fullName() const;

    QString extension() const;

    QString sizeString() const;

    QString modificationDateString() const;

protected:
    FileSystemObjectProperties _properties;
private:
    // Can be used to determine whether two objects are on the same drive
    QFileInfo _fileInfo;
    mutable uint64_t _rootFileSystemId = std::numeric_limits<uint64_t>::max();
};

#undef QFileInfo
#undef QDir
