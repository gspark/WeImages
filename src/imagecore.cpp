#include "imagecore.h"
#include <random>
#include <QDir>
#include <QUrl>
#include <QSettings>
#include <QCollator>
#include <QtConcurrent/QtConcurrentRun>
#include <QPixmapCache>
#include <QIcon>
#include <QGuiApplication>
#include <QScreen>
#include <QMimeDatabase>

#include "logger/Logger.h"

ImageCore::ImageCore(QObject* parent) : QObject(parent)
{
    // Set allocation limit to 8 GiB on Qt6
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QImageReader::setAllocationLimit(8192);
#endif
    QPixmapCache::setCacheLimit(512000);

    connect(&loadFutureWatcher, &QFutureWatcher<ReadData>::finished, this, [this]() {
        loadPixmap(loadFutureWatcher.result(), false);
        });
}

void ImageCore::loadFile(const QString& fileName, const QSize& targetSize)
{
    QString sanitaryFileName = fileName;

    //sanitize file name if necessary
    QUrl sanitaryUrl = QUrl(fileName);
    if (sanitaryUrl.isLocalFile())
        sanitaryFileName = sanitaryUrl.toLocalFile();

    QFileInfo fileInfo(sanitaryFileName);
    sanitaryFileName = fileInfo.absoluteFilePath();

    //check if cached already before loading the long way
  
    auto* cachedPixmap = new QPixmap();
    if (QPixmapCache::find(sanitaryFileName.append("_%1x%2").arg(targetSize.width()).arg(targetSize.height()), cachedPixmap) && !cachedPixmap->isNull())
    {
        ReadData readData = {
            *cachedPixmap,
            fileInfo
        };
        loadPixmap(readData, true);
    }
    else
    {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        loadFutureWatcher.setFuture(QtConcurrent::run(this, &ImageCore::readFile, fileInfo.absoluteFilePath(), false, targetSize));
#else
        loadFutureWatcher.setFuture(QtConcurrent::run(&ImageCore::readFile, this, fileInfo.absoluteFilePath(), false, targetSize));
#endif
    }
    delete cachedPixmap;
}


ImageCore::ReadData ImageCore::readFile(const QString& fileName, bool forCache, const QSize& targetSize)
{
    LOG_INFO << "ImageCore::readFile " << fileName << " QSize: " << targetSize;
    QFileInfo fileInfo(fileName);
    auto* cachedPixmap = new QPixmap();
    QString sanitaryFileName = fileName;
    if (QPixmapCache::find(sanitaryFileName.append("_%1x%2").arg(targetSize.width()).arg(targetSize.height()), cachedPixmap) && !cachedPixmap->isNull())
    {
        ReadData readData = {
            *cachedPixmap,
            fileInfo
        };
        return readData;
    }

    QPixmap readPixmap;

    if (fileInfo.suffix() == "dat") {
        // wechat picture
        DWORD start = GetTickCount();
        BYTE* imageData = datConverImage(fileName, fileInfo.size());
        LOG_INFO << " datConverImage time: " << GetTickCount() - start;

        start = GetTickCount();
        if (readPixmap.loadFromData(imageData, fileInfo.size())) {
            if (readPixmap.isNull()) {
                return {};
            }
        }
        if (targetSize.isValid())
        {
            readPixmap = readPixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        LOG_INFO << " loadFromData time: " << GetTickCount() - start;
        delete[] imageData;
    }
    else {
        QImageReader imageReader;
        imageReader.setDecideFormatFromContent(true);
        imageReader.setAutoTransform(true);

        imageReader.setFileName(fileName);
        if (targetSize.isValid())
        {
            auto targetScaledSize = imageReader.size().scaled(targetSize, Qt::KeepAspectRatio);
            imageReader.setScaledSize(targetSize);
        }

        if (imageReader.format() == "svg" || imageReader.format() == "svgz")
        {
            // Render vectors into a high resolution
            QIcon icon;
            icon.addFile(fileName);
            readPixmap = icon.pixmap(ICON_WIDE);
            // If this fails, try reading the normal way so that a proper error message is given
            if (readPixmap.isNull())
                readPixmap = QPixmap::fromImageReader(&imageReader);
        }
        else
        {
            DWORD start = GetTickCount();
            readPixmap = QPixmap::fromImageReader(&imageReader);
            LOG_INFO << " QPixmap::fromImageReader time: " << GetTickCount() - start;
        }
    }

    ReadData readData = {
        readPixmap,
        //QFileInfo(imageFileName),
        fileInfo
    };
    if (forCache)
    {
        addToCache(readData);
    }
    return readData;
}

//ImageCore::ReadData ImageCore::readFileSize(const QString& fileName, bool forCache, const QSize& targetSize)
//{
//
//}

void ImageCore::loadPixmap(const ReadData& readData, bool fromCache)
{
    if (readData.pixmap.isNull())
        return;

    // If this image isnt originally from the cache, add it to the cache
    if (!fromCache)
        addToCache(readData);
    emit imageLoaded(readData.pixmap);
}

void ImageCore::addToCache(const ReadData &readData)
{
    if (readData.pixmap.isNull()) {
        return;
    }
    QPixmapCache::insert(readData.fileInfo.absoluteFilePath().append("_%1x%2").arg(readData.pixmap.width()).arg(readData.pixmap.height()), readData.pixmap);
}

bool ImageCore::isImageFile(const QFileInfo& fileInfo)
{
    if (fileInfo.suffix() == "dat" && fileInfo.baseName().length() == 32)
    {
        // wechat picture
        return true;
    }
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(fileInfo);
    if (mime.name().startsWith("image/"))
    {
        return true;
    }
    return false;
}

QStringList ImageCore::imageFileNames()
{
    QStringList names;
    QMimeDatabase db;
    QList<QMimeType> mimeList = db.allMimeTypes();
    foreach(const QMimeType & mime, mimeList) {
        if (mime.name().startsWith(QStringLiteral("image/"))) {
            names << mime.preferredSuffix();
            if (!mime.preferredSuffix().isNull() && !mime.preferredSuffix().isEmpty())
            {
                names << "*." + mime.preferredSuffix();
            }
        }
    }
    // wechat
    names << "????????????????????????????????.dat";
    return names;
}

BYTE* ImageCore::datConverImage(const QString &datFileName, long long fileSize) {
    BYTE byJPG1 = 0xFF;
    BYTE byJPG2 = 0xD8;
    BYTE byGIF1 = 0x47;
    BYTE byGIF2 = 0x49;
    BYTE byPNG1 = 0x89;
    BYTE byPNG2 = 0x50;

    HANDLE hDatFile = INVALID_HANDLE_VALUE;

    BYTE* datBuf = new BYTE[fileSize];
    BYTE* tmpBuf = new BYTE[64 * 1024];
    do
    {
        hDatFile = CreateFile(datFileName.toStdWString().c_str(), GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (INVALID_HANDLE_VALUE == hDatFile)
            break;

        // 读取内容前2个字节
        DWORD dwReadLen = 0;
        BOOL bRet = ReadFile(hDatFile, tmpBuf, 2, &dwReadLen, NULL);
        if (!bRet || 2 != dwReadLen)
            break;

        // 开始异或判断
        BYTE byJ1 = byJPG1 ^ tmpBuf[0];
        BYTE byJ2 = byJPG2 ^ tmpBuf[1];
        BYTE byG1 = byGIF1 ^ tmpBuf[0];
        BYTE byG2 = byGIF2 ^ tmpBuf[1];
        BYTE byP1 = byPNG1 ^ tmpBuf[0];
        BYTE byP2 = byPNG2 ^ tmpBuf[1];

        // 判断异或值
        BYTE byXOR = 0;
        if (byJ1 == byJ2)
        {
//            imageFile += ".jpg";
            byXOR = byJ1;
        }
        else if (byG1 == byG2)
        {
//            imageFile += ".gif";
            byXOR = byG1;
        }
        else if (byP1 == byP2)
        {
//            imageFile += ".png";
            byXOR = byP1;
        }
        else
            break;

        SetFilePointer(hDatFile, 0, NULL, FILE_BEGIN); // 设置到文件头开始
//        hImageFile = CreateFile(imageFile.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
//        if (INVALID_HANDLE_VALUE == hImageFile)
//            break;

        DWORD dwWriteLen = 0;
        DWORD index = 0;
        do
        {
            dwReadLen = 0;
            bRet = ReadFile(hDatFile, tmpBuf, 64 * 1024, &dwReadLen, NULL);
            if (!bRet)
                break;

            XOR(tmpBuf, dwReadLen, byXOR);

//            bRet = WriteFile(hImageFile, byBuf, dwReadLen, &dwWriteLen, NULL);

            memcpy(datBuf + index, tmpBuf, dwReadLen);
            index += dwReadLen;
//            if (!bRet || dwReadLen != dwWriteLen)
            if (!bRet)
                break;

        } while (dwReadLen == 64*1024);
    } while (FALSE);

    delete[] tmpBuf;

    if (INVALID_HANDLE_VALUE != hDatFile)
    {
        CloseHandle(hDatFile);
        hDatFile = INVALID_HANDLE_VALUE;
    }
    return datBuf;
}

void ImageCore::XOR(BYTE *v_pbyBuf, DWORD v_dwBufLen, BYTE byXOR) {
    for (int i = 0; i < v_dwBufLen; i++)
    {
        v_pbyBuf[i] ^= byXOR;
    }
}
