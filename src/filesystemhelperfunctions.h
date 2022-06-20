#pragma once

#include "filesystemobject.h"

#include <QStringBuilder>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

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

[[nodiscard]] inline QString escapedPath(QString path) {
    if (!path.contains(' '))
        return path;

#ifdef _WIN32
    return '\"' % path % '\"';
#else
    //assert_debug_only(path.count(' ') != path.count(R"(\ )")); // Already escaped!
    //return path.replace(' ', QLatin1String(R"(\ )"));

    assert_debug_only(!path.startsWith('\'')); // Already escaped!
    return '\'' % path % '\'';
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
            unitCodes.count(maxUnit) > 0 ? unitCodes.at(maxUnit) : std::numeric_limits<unsigned int>::max();

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
