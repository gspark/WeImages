#pragma once

#include <QString>
#include <QDir>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif


[[nodiscard]] inline constexpr char nativeSeparator() noexcept {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

[[nodiscard]] inline QString toNativeSeparators(QString path) {
#ifdef _WIN32
    return path.replace('/', nativeSeparator());
#else
    return path;
#endif
}

[[nodiscard]] inline QString toPosixSeparators(QString path) {
#ifdef _WIN32
    return path.replace(nativeSeparator(), '/');
#else
    assert_debug_only(!path.contains('\\'));
    return path;
#endif
}


[[nodiscard]] inline QString cleanPath(QString path) {
    return path.replace(QStringLiteral("\\\\"), QStringLiteral("\\")).replace(QStringLiteral("//"),
                                                                              QStringLiteral("/"));
}

[[nodiscard]] inline QString
fileSizeToString(uint64_t size, const char maxUnit = '\0', const QString &spacer = QString()) {
    const unsigned int KB = 1024;
    const unsigned int MB = 1024 * KB;
    const unsigned int GB = 1024 * MB;

    const std::map<char, unsigned int> unitCodes{{'B', 0},
                                                 {'K', KB},
                                                 {'M', MB}};
    const unsigned int maxUnitSize =
            unitCodes.count(maxUnit) > 0 ? unitCodes.at(maxUnit) : (std::numeric_limits<unsigned int>::max)();

    QString str;
    float n = 0.0f;
    if (size >= GB && maxUnitSize >= GB) {
        n = size / float(GB);
        str = QStringLiteral("%1 GiB").arg(QString::number(n, 'f', 1));
    } else if (size >= MB && maxUnitSize >= MB) {
        n = size / float(MB);
        str = QStringLiteral("%1 MiB").arg(QString::number(n, 'f', 1));
    } else if (size >= KB && maxUnitSize >= KB) {
        n = size / float(KB);
        str = QStringLiteral("%1 KiB").arg(QString::number(n, 'f', 1));
    } else {
        n = (float) size;
        str = QStringLiteral("%1 B").arg(size);
    }

    if (!spacer.isEmpty() && n > 0.0f) {
        for (int spacerPos = (int) std::log10(n) - 3; spacerPos > 0; spacerPos -= 3)
            str.insert(spacerPos + 1, spacer);
    }

    return str;
}

[[nodiscard]] inline constexpr bool caseSensitiveFilesystem() noexcept {
#if defined _WIN32
    return false;
#elif defined __APPLE__
    return false;
#elif defined __linux__
    return true;
#elif defined __FreeBSD__
    return true;
#else
#error "Unknown operating system"
    return true;
#endif
}

[[nodiscard]] inline bool isDriveRootPath(const QString& path) {
    return (path.length() == 3
        && path.at(0).isLetter() && path.at(1) == QLatin1Char(':')
        && path.at(2) == QLatin1Char('/'));
}

#ifdef Q_OS_WIN
static bool isUncRoot(const QString& server)
{
    QString localPath = QDir::toNativeSeparators(server);
    if (!localPath.startsWith(QLatin1String("\\\\")))
        return false;

    int idx = localPath.indexOf(QLatin1Char('\\'), 2);
    if (idx == -1 || idx + 1 == localPath.length())
        return true;

    return QStringView{ localPath }.right(localPath.length() - idx - 1).trimmed().isEmpty();
}
#endif

[[nodiscard]] inline bool isRootPath(const QString& path) noexcept {
    if (path == QLatin1String("/")
#if defined(Q_OS_WIN)
        || isDriveRootPath(path)
        || isUncRoot(path)
#endif
        )
        return true;

    return false;
}

[[nodiscard]] inline bool isDrive(const QFileInfo& fileInfo) noexcept {
    if (isRootPath(fileInfo.absoluteFilePath()))
    {
        return true;
    }
    return false;
}

#ifdef _WIN32
[[nodiscard]] inline bool getFileVersionInfo(const QString& fileName, QString& strProductVersion, QString& strFileVersion) noexcept {
    std::wstring wPath = fileName.toStdWString();
    LPCWSTR str_path = wPath.c_str();
    DWORD vHandle = 0;
    DWORD dwLen = GetFileVersionInfoSize(str_path, &vHandle);
    if (0 == dwLen) {
        return false;
    }
    char* lpData = (char*)malloc(dwLen + 1);
    if (NULL == lpData) {
        return false;
    }
    bool ret = false;
    do {
        // ProductVersion
        BOOL bSuccess = GetFileVersionInfo(str_path, 0, dwLen + 1, lpData);
        if (!bSuccess) {
            break;
        }

        LPVOID lpBuffer = NULL;
        UINT uLen = 0;

        //获得语言和代码页(language and code page)
        bSuccess = VerQueryValue(lpData,
            (TEXT("\\VarFileInfo\\Translation")),
            &lpBuffer,
            &uLen);
        if (!bSuccess) {
            break;
        }
        QString strTranslation, str1, str2;
        unsigned short int* p = (unsigned short int*)lpBuffer;
        str1.setNum(*p, 16);
        str1 = "000" + str1;
        strTranslation += str1.mid(str1.size() - 4, 4);
        str2.setNum(*(++p), 16);
        str2 = "000" + str2;
        strTranslation += str2.mid(str2.size() - 4, 4);

        // 获得文件说明：FileDescription ...
        // 获得 ProductVersion
        QString code = "\\StringFileInfo\\" + strTranslation + "\\ProductVersion";
        bSuccess = VerQueryValue(lpData,
            (code.toStdWString().c_str()),
            &lpBuffer,
            &uLen);
        if (!bSuccess) {
            break;
        }
        strProductVersion = QString::fromUtf16((const unsigned short int*)lpBuffer);

        // FileVersion
        lpBuffer = NULL;
        uLen = 0;
        code = "\\StringFileInfo\\" + strTranslation + "\\FileVersion";
        bSuccess = VerQueryValue(lpData, (code.toStdWString().c_str()), &lpBuffer, &uLen);
        if (!bSuccess) {
            //DWORD err = GetLastError();
            break;
        }
        strFileVersion = QString::fromUtf16((const unsigned short int*)lpBuffer);
        ret = true;
    } while (false);
    free(lpData);
    lpData = NULL;
    return ret;
}

#endif

