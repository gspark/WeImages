#include "imagecore.h"
#include <QDir>
#include <QUrl>
#include <QSettings>
#include <QCollator>
#include <QtConcurrent/QtConcurrentRun>
#include <QIcon>
#include <QGuiApplication>
#include <QScreen>
#include <QMimeDatabase>

#include "logger/Logger.h"
#include "util/fasthash.h"

ImageCore::ImageCore(QObject* parent) : QObject(parent)
{
    // Set allocation limit to 8 GiB on Qt6
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QImageReader::setAllocationLimit(8192);
#endif
    _imageReadDataCache = new QCache<uint64_t, ImageReadData>(5000);

    _mineDb = new QMimeDatabase;

    connect(&loadFutureWatcher, &QFutureWatcher<ImageReadData>::finished, this, [this]() {
        loadPixmap(loadFutureWatcher.result());
        });
}

ImageCore::~ImageCore()
{
    delete _mineDb;
    _imageReadDataCache->clear();
    delete _imageReadDataCache;
}

void ImageCore::loadFile(const QString& fileName, const QSize& targetSize)
{
    QString sanitaryFileName = fileName;

    //sanitize file name if necessary
    QUrl sanitaryUrl = QUrl(fileName);
    if (QUrl(fileName).isLocalFile())
        sanitaryFileName = sanitaryUrl.toLocalFile();

    QFileInfo fileInfo(sanitaryFileName);
    sanitaryFileName = fileInfo.absoluteFilePath();

    uint64_t hash = 0;
    if (findImageReadData(hash, fileName, targetSize))
    {
        ImageReadData* readData = this->getImageReadData(hash);
        loadPixmap(readData);
    }
    else
    {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        loadFutureWatcher.setFuture(QtConcurrent::run(this, &ImageCore::readFile, fileInfo.absoluteFilePath(), targetSize));
#else
        loadFutureWatcher.setFuture(QtConcurrent::run(&ImageCore::readFile, this, fileInfo.absoluteFilePath(), targetSize));
#endif
    }
}


ImageReadData* ImageCore::readFile(const QString& fileName, const QSize& targetSize)
{
    uint64_t hash = 0;
    if (findImageReadData(hash, fileName, targetSize))
    {
        ImageReadData* readData = this->getImageReadData(hash);
        return readData;
    }

    QPixmap readPixmap;
    QFileInfo fileInfo(fileName);
    QString extension = fileInfo.suffix();
    if (this->isWeChatImage(fileInfo)) {
        // wechat picture
        readPixmap = readWeImage(fileName, fileInfo.size(), extension, targetSize);
        LOG_INFO << "readWeImage extension: " << extension;
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
            readPixmap = QPixmap::fromImageReader(&imageReader);
        }
    }

    ImageReadData* readData = new ImageReadData{
        readPixmap,
        fileInfo,
        extension,
        hash
    };
    addToCache(*readData);
    return readData;
}

QPixmap ImageCore::readWeImage(const QString& fileName, long long fileSize, QString& extension, const QSize& targetSize)
{
    QPixmap readPixmap;
    BYTE* imageData = datConverImage(fileName, fileSize, &extension);
    DWORD start = GetTickCount();
    if (readPixmap.loadFromData(imageData, fileSize)) {
        if (!readPixmap.isNull() && targetSize.isValid())
        {
            readPixmap = readPixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
    LOG_INFO << "loadFromData time: " << GetTickCount() - start;
    delete[] imageData;
    return readPixmap;
}

void ImageCore::loadPixmap(const ImageReadData* readData)
{
    if (readData->pixmap.isNull())
        return;
    emit imageLoaded((ImageReadData*)readData);
}

void ImageCore::addToCache(const ImageReadData &readData)
{
    //QString key = readData.fileInfo.absoluteFilePath().append("_%1x%2").arg(readData.pixmap.width()).arg(readData.pixmap.height());
    //LOG_INFO << "addToCache key: " << key;
    //QPixmapCache::insert(key, readData.pixmap);
   
    this->_imageReadDataCache->insert(readData.hash, const_cast<ImageReadData*>(&readData));
}

bool ImageCore::isImageFile(const QFileInfo& fileInfo)
{
    if (isWeChatImage(fileInfo))
    {
        // wechat picture
        return true;
    }
    
    QMimeType mime = _mineDb->mimeTypeForFile(fileInfo);
    
    return mime.name().startsWith("image/");
}

bool ImageCore::isWeChatImage(const QFileInfo& fileInfo)
{
    //LOG_INFO << "isWeChatImage suffix:" << fileInfo.suffix() << " baseName: " << fileInfo.baseName();
    return fileInfo.suffix() == "dat" && fileInfo.baseName().length() == 32;
}

QPixmap ImageCore::scaled(const QPixmap& originPixmap, const QSize& targetSize)
{
    return originPixmap.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QPixmap ImageCore::flipImage(const QPixmap originPixmap, bool horizontal /*= true*/, int dir /*= 1*/)
{
    float  width = originPixmap.width(), height = originPixmap.height();
    return originPixmap.transformed(QTransform()
        .translate(-width / 2, -height / 2)
        .rotate(horizontal ? 180.0 * dir : -180.0 * dir, horizontal ? Qt::YAxis : Qt::XAxis)
        .translate(width / 2, height / 2), Qt::TransformationMode::SmoothTransformation);
}

QPixmap ImageCore::rotateImage(const QPixmap& originPixmap, bool right /*= true*/, int dir /*= 1*/)
{
    float  width = originPixmap.width(), height = originPixmap.height();
    return originPixmap.transformed(QTransform()
        .translate(-width / 2, -height / 2)
        .rotate(right ? 90.0 * dir : -90.0 * dir)
        .translate(width / 2, height / 2), Qt::TransformationMode::SmoothTransformation);
}

QStringList ImageCore::imageNames()
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

BYTE* ImageCore::datConverImage(const QString &datFileName, long long fileSize, QString* extension) {
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
            *extension = "jpg";
        }
        else if (byG1 == byG2)
        {
//            imageFile += ".gif";
            byXOR = byG1;
            *extension = "gif";
        }
        else if (byP1 == byP2)
        {
//            imageFile += ".png";
            byXOR = byP1;
            *extension = "png";
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

bool ImageCore::findImageReadData(uint64_t& hash, const QString& absoluteFilePath, const QSize& targetSize)
{
    QString key = absoluteFilePath;
    key = key.append("_%1x%2").arg(targetSize.width()).arg(targetSize.height());
    LOG_INFO << "findImageReadData key: " << key;
    hash = fasthash64(key.constData(), static_cast<uint64_t>(key.size()) * sizeof(QChar), 0);
    return this->_imageReadDataCache->contains(hash);
}

ImageReadData* ImageCore::getImageReadData(uint64_t hash)
{
    return this->_imageReadDataCache->object(hash);
}
