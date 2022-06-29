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

ImageCore::ImageCore(QObject *parent) : QObject(parent)
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


void ImageCore::loadFile(const QString& fileName)
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
    if (QPixmapCache::find(sanitaryFileName, cachedPixmap) &&
        !cachedPixmap->isNull())
    {
        ReadData readData = {
            *cachedPixmap,
            fileInfo,
            cachedPixmap->size()
        };
        loadPixmap(readData, true);
    }
    else
    {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        loadFutureWatcher.setFuture(QtConcurrent::run(this, &ImageCore::readFile, sanitaryFileName, false));
#else
        loadFutureWatcher.setFuture(QtConcurrent::run(&ImageCore::readFile, this, sanitaryFileName, false));
#endif
    }
    delete cachedPixmap;
}


ImageCore::ReadData ImageCore::readFile(const QString &fileName, bool forCache)
{
    return readFileSize(fileName, forCache, QSize());
}

ImageCore::ReadData ImageCore::readFileSize(const QString& fileName, bool forCache, const QSize& targetSize)
{
    LOG_INFO << "ImageCore::readFile " << fileName;
    QFileInfo fileInfo(fileName);
    auto* cachedPixmap = new QPixmap();
    if (QPixmapCache::find(fileName, cachedPixmap) && !cachedPixmap->isNull())
    {
        ReadData readData = {
            *cachedPixmap,
            fileInfo,
            cachedPixmap->size()
        };
        return readData;
    }
    LOG_INFO << "fileInfo::absoluteFilePath " << fileInfo.absoluteFilePath();

    QPixmap readPixmap;
    QSize size;

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
            size = readPixmap.size();
        }
        LOG_INFO << " loadFromData time: " << GetTickCount() - start;
        delete imageData;
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
            readPixmap = icon.pixmap(48);
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
        size = imageReader.size();
    }

    ReadData readData = {
        readPixmap,
        //QFileInfo(imageFileName),
        fileInfo,
        size,
    };
    if (forCache)
    {
        addToCache(readData);
    }
    return readData;
}

void ImageCore::loadPixmap(const ReadData& readData, bool fromCache)
{
    if (readData.pixmap.isNull())
        return;

    // If this image isnt originally from the cache, add it to the cache
    if (!fromCache)
        addToCache(readData);
    emit fileDataChanged(readData.pixmap);
}

void ImageCore::addToCache(const ReadData &readData)
{
    if (readData.pixmap.isNull()) {
        return;
    }
    QPixmapCache::insert(readData.fileInfo.absoluteFilePath(), readData.pixmap);

//    auto *size = new qint64(readData.fileInfo.size());
//    qvApp->setPreviouslyRecordedFileSize(readData.fileInfo.absoluteFilePath(), size);
//    qvApp->setPreviouslyRecordedImageSize(readData.fileInfo.absoluteFilePath(), new QSize(readData.size));
}

BYTE* ImageCore::datConverImage(const QString &datFileName, long long fileSize) {
    BYTE byJPG1 = 0xFF;
    BYTE byJPG2 = 0xD8;
    BYTE byGIF1 = 0x47;
    BYTE byGIF2 = 0x49;
    BYTE byPNG1 = 0x89;
    BYTE byPNG2 = 0x50;

    HANDLE hDatFile = INVALID_HANDLE_VALUE;
//    HANDLE hImageFile = INVALID_HANDLE_VALUE;

//    QString imageFile(imageFileName);
    BYTE* datBuf = new BYTE[fileSize];
    do
    {
        hDatFile = CreateFile(datFileName.toStdWString().c_str(), GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (INVALID_HANDLE_VALUE == hDatFile)
            break;

        // 读取内容前2个字节
        BYTE byBuf[64*1024] = { 0 };
        DWORD dwReadLen = 0;
        BOOL bRet = ReadFile(hDatFile, byBuf, 2, &dwReadLen, NULL);
        if (!bRet || 2 != dwReadLen)
            break;

        // 开始异或判断
        BYTE byJ1 = byJPG1 ^ byBuf[0];
        BYTE byJ2 = byJPG2 ^ byBuf[1];
        BYTE byG1 = byGIF1 ^ byBuf[0];
        BYTE byG2 = byGIF2 ^ byBuf[1];
        BYTE byP1 = byPNG1 ^ byBuf[0];
        BYTE byP2 = byPNG2 ^ byBuf[1];

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
            bRet = ReadFile(hDatFile, byBuf, 64 * 1024, &dwReadLen, NULL);
            if (!bRet)
                break;

            XOR(byBuf, dwReadLen, byXOR);

//            bRet = WriteFile(hImageFile, byBuf, dwReadLen, &dwWriteLen, NULL);

            memcpy(datBuf + index, byBuf, dwReadLen);
            index += dwReadLen;
//            if (!bRet || dwReadLen != dwWriteLen)
            if (!bRet)
                break;

        } while (dwReadLen == 64*1024);
    } while (FALSE);

    if (INVALID_HANDLE_VALUE != hDatFile)
    {
        CloseHandle(hDatFile);
        hDatFile = INVALID_HANDLE_VALUE;
    }

//    if (INVALID_HANDLE_VALUE != hImageFile)
//    {
//        CloseHandle(hImageFile);
//        hImageFile = INVALID_HANDLE_VALUE;
//    }
    return datBuf;
}

void ImageCore::XOR(BYTE *v_pbyBuf, DWORD v_dwBufLen, BYTE byXOR) {
    for (int i = 0; i < v_dwBufLen; i++)
    {
        v_pbyBuf[i] ^= byXOR;
    }
}
