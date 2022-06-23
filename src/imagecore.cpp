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

ImageCore::ImageCore(QObject *parent) : QObject(parent)
{
// Set allocation limit to 8 GiB on Qt6
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QImageReader::setAllocationLimit(8192);
#endif

    isLoopFoldersEnabled = true;
    preloadingMode = 1;
    sortMode = 0;
    sortDescending = false;

    randomSortSeed = 0;

    currentRotation = 0;

    QPixmapCache::setCacheLimit(51200);

    connect(&loadedMovie, &QMovie::updated, this, &ImageCore::animatedFrameChanged);

    connect(&loadFutureWatcher, &QFutureWatcher<ReadData>::finished, this, [this](){
        loadPixmap(loadFutureWatcher.result(), false);
    });

    largestDimension = 0;
    const auto screenList = QGuiApplication::screens();
    for (auto const &screen : screenList)
    {
        int largerDimension;
        if (screen->size().height() > screen->size().width())
        {
            largerDimension = screen->size().height();
        }
        else
        {
            largerDimension = screen->size().width();
        }

        if (largerDimension > largestDimension)
        {
            largestDimension = largerDimension;
        }
    }

    waitingOnLoad = false;

    // Connect to settings signal
//    connect(&qvApp->getSettingsManager(), &SettingsManager::settingsUpdated, this, &QVImageCore::settingsUpdated);
    settingsUpdated();
}

void ImageCore::loadFile(const QString &fileName)
{
    if (waitingOnLoad)
    {
        return;
    }

    QString sanitaryFileName = fileName;

    //sanitize file name if necessary
    QUrl sanitaryUrl = QUrl(fileName);
    if (sanitaryUrl.isLocalFile())
        sanitaryFileName = sanitaryUrl.toLocalFile();

    QFileInfo fileInfo(sanitaryFileName);
    sanitaryFileName = fileInfo.absoluteFilePath();

    // Pause playing movie because it feels better that way
    setPaused(true);

    currentFileDetails.isLoadRequested = true;
    waitingOnLoad = true;


    //check if cached already before loading the long way
    //auto previouslyRecordedFileSize = qvApp->getPreviouslyRecordedFileSize(sanitaryFileName);
    auto *cachedPixmap = new QPixmap();
    if (QPixmapCache::find(sanitaryFileName, cachedPixmap) &&
        !cachedPixmap->isNull())
    {
        ReadData readData = {
            matchCurrentRotation(*cachedPixmap),
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
    QFileInfo fileInfo(fileName);
    auto* cachedPixmap = new QPixmap();
    if (QPixmapCache::find(fileName, cachedPixmap) && !cachedPixmap->isNull())
    {
        ReadData readData = {
            matchCurrentRotation(*cachedPixmap),
            fileInfo,
            cachedPixmap->size()
        };
        loadPixmap(readData, true);
        return readData;
    }

    QPixmap readPixmap;
    QSize size;

    QString imageFileName(fileName);

    if (fileInfo.suffix() == "dat") {
        // wechat picture
        BYTE* imageData = datConverImage(fileName, fileInfo.size());
        if (readPixmap.loadFromData(imageData, fileInfo.size())) {
            if (readPixmap.isNull()) {
                return {};
            }
        }
    } else {
        QImageReader imageReader;
        imageReader.setDecideFormatFromContent(true);
        imageReader.setAutoTransform(true);

        imageReader.setFileName(imageFileName);


        if (imageReader.format() == "svg" || imageReader.format() == "svgz")
        {
            // Render vectors into a high resolution
            QIcon icon;
            icon.addFile(imageFileName);
            readPixmap = icon.pixmap(largestDimension);
            // If this fails, try reading the normal way so that a proper error message is given
            if (readPixmap.isNull())
                readPixmap = QPixmap::fromImageReader(&imageReader);
        }
        else
        {
            readPixmap = QPixmap::fromImageReader(&imageReader);
        }
        size = imageReader.size();
        // Only error out when not loading for cache
        if (readPixmap.isNull() && !forCache)
        {
            emit readError(imageReader.error(), imageReader.errorString(), fileInfo.fileName());
        }
    }

    ReadData readData = {
        readPixmap,
        QFileInfo(imageFileName),
        size,
    };
    if (forCache)
    {
        addToCache(readData);
    }
    return readData;
}

void ImageCore::loadPixmap(const ReadData &readData, bool fromCache)
{
    // Do this first so we can keep folder info even when loading errored files
    currentFileDetails.fileInfo = readData.fileInfo;
    //updateFolderInfo();

    // Reset mechanism to avoid stalling while loading
    waitingOnLoad = false;

    if (readData.pixmap.isNull())
        return;

    loadedPixmap = matchCurrentRotation(readData.pixmap);

    // Set file details
    currentFileDetails.isPixmapLoaded = true;
    currentFileDetails.baseImageSize = readData.size;
    currentFileDetails.loadedPixmapSize = loadedPixmap.size();
    if (currentFileDetails.baseImageSize == QSize(-1, -1))
    {
        qInfo() << "QImageReader::size gave an invalid size for " + currentFileDetails.fileInfo.fileName() + ", using size from loaded pixmap";
        currentFileDetails.baseImageSize = currentFileDetails.loadedPixmapSize;
    }

    // If this image isnt originally from the cache, add it to the cache
    if (!fromCache)
        addToCache(readData);

    // Animation detection
    loadedMovie.setFormat("");
    loadedMovie.stop();
    loadedMovie.setFileName(currentFileDetails.fileInfo.absoluteFilePath());

    // APNG workaround
    if (loadedMovie.format() == "png")
    {
        loadedMovie.setFormat("apng");
        loadedMovie.setFileName(currentFileDetails.fileInfo.absoluteFilePath());
    }

    currentFileDetails.isMovieLoaded = loadedMovie.isValid() && loadedMovie.frameCount() != 1;

    if (currentFileDetails.isMovieLoaded)
        loadedMovie.start();
    else if (auto device = loadedMovie.device())
        device->close();

    emit fileDataChanged(readData.pixmap);

//    QtConcurrent::run(&QVImageCore::requestCaching, this);
}

void ImageCore::closeImage()
{
    loadedPixmap = QPixmap();
    loadedMovie.stop();
    loadedMovie.setFileName("");
    currentFileDetails = {
        QFileInfo(),
        currentFileDetails.folderFileInfoList,
        currentFileDetails.loadedIndexInFolder,
        false,
        false,
        false,
        QSize(),
        QSize()
    };

    emit fileChanged();
}

// All file logic, sorting, etc should be moved to a different class or file
QFileInfoList ImageCore::getCompatibleFiles()
{
    QFileInfoList fileInfoList;

//    QMimeDatabase mimeDb;
//    const auto &regs = qvApp->getFilterRegExpList();
//    const auto &mimeTypes = qvApp->getMimeTypeNameList();
//
//    const QFileInfoList currentFolder = currentFileDetails.fileInfo.dir().entryInfoList();
//    for (const QFileInfo &fileInfo : currentFolder)
//    {
//        bool matched = false;
//        const QString name = fileInfo.fileName();
//        for (const QRegularExpression &reg : regs)
//        {
//            if (reg.match(name).hasMatch()) {
//                matched = true;
//                break;
//            }
//        }
//        if (matched || mimeTypes.contains(mimeDb.mimeTypeForFile(fileInfo).name().toUtf8()))
//        {
//            fileInfoList.append(fileInfo);
//        }
//    }

    return fileInfoList;
}

void ImageCore::updateFolderInfo()
{
    if (!currentFileDetails.fileInfo.isFile())
        return;

    QPair<QString, uint> dirInfo = {currentFileDetails.fileInfo.absoluteDir().path(),
                                    currentFileDetails.fileInfo.dir().count()};
    // If the current folder changed since the last image, assign a new seed for random sorting
    if (lastDirInfo != dirInfo)
    {
        randomSortSeed = std::chrono::system_clock::now().time_since_epoch().count();
    }
    lastDirInfo = dirInfo;


    currentFileDetails.folderFileInfoList = getCompatibleFiles();

    // Sorting

    if (sortMode == 0) // Natural sorting
    {
        QCollator collator;
        collator.setNumericMode(true);
        std::sort(currentFileDetails.folderFileInfoList.begin(),
                  currentFileDetails.folderFileInfoList.end(),
                  [&collator, this](const QFileInfo &file1, const QFileInfo &file2)
        {
            if (sortDescending)
                return collator.compare(file1.fileName(), file2.fileName()) > 0;
            else
                return collator.compare(file1.fileName(), file2.fileName()) < 0;
        });
    }
    else if (sortMode == 1) // last modified
    {
        std::sort(currentFileDetails.folderFileInfoList.begin(),
                  currentFileDetails.folderFileInfoList.end(),
                  [this](const QFileInfo &file1, const QFileInfo &file2)
        {
            if (sortDescending)
                return file1.lastModified() < file2.lastModified();
            else
                return file1.lastModified() > file2.lastModified();
        });
    }
    else if (sortMode == 2) // size
    {
        std::sort(currentFileDetails.folderFileInfoList.begin(),
                  currentFileDetails.folderFileInfoList.end(),
                  [this](const QFileInfo &file1, const QFileInfo &file2)
        {
            if (sortDescending)
                return file1.size() < file2.size();
            else
                return file1.size() > file2.size();
        });
    }
    else if (sortMode == 3) // type
    {
        QMimeDatabase mimeDb;

        QCollator collator;
        std::sort(currentFileDetails.folderFileInfoList.begin(),
                  currentFileDetails.folderFileInfoList.end(),
                  [&mimeDb, &collator, this](const QFileInfo &file1, const QFileInfo &file2)
        {
            QMimeType mime1 = mimeDb.mimeTypeForFile(file1);
            QMimeType mime2 = mimeDb.mimeTypeForFile(file2);

            if (sortDescending)
                return collator.compare(mime1.name(), mime2.name()) > 0;
            else
                return collator.compare(mime1.name(), mime2.name()) < 0;
        });
    }
    else if (sortMode == 4) // Random
    {
        std::shuffle(currentFileDetails.folderFileInfoList.begin(), currentFileDetails.folderFileInfoList.end(), std::default_random_engine(randomSortSeed));
    }

    // Set current file index variable
    currentFileDetails.loadedIndexInFolder = currentFileDetails.folderFileInfoList.indexOf(currentFileDetails.fileInfo);
}

void ImageCore::requestCaching()
{
    if (preloadingMode == 0)
    {
        QPixmapCache::clear();
        return;
    }

    int preloadingDistance = 1;

    if (preloadingMode > 1)
        preloadingDistance = 4;

    QStringList filesToPreload;
    for (int i = currentFileDetails.loadedIndexInFolder-preloadingDistance; i <= currentFileDetails.loadedIndexInFolder+preloadingDistance; i++)
    {
        int index = i;

        // Don't try to cache the currently loaded image
        if (index == currentFileDetails.loadedIndexInFolder)
            continue;

        //keep within index range
        if (isLoopFoldersEnabled)
        {
            if (index > currentFileDetails.folderFileInfoList.length()-1)
                index = index-(currentFileDetails.folderFileInfoList.length());
            else if (index < 0)
                index = index+(currentFileDetails.folderFileInfoList.length());
        }

        //if still out of range after looping, just cancel the cache for this index
        if (index > currentFileDetails.folderFileInfoList.length()-1 || index < 0 || currentFileDetails.folderFileInfoList.isEmpty())
            continue;

        QString filePath = currentFileDetails.folderFileInfoList[index].absoluteFilePath();
        filesToPreload.append(filePath);

        requestCachingFile(filePath);
    }
    lastFilesPreloaded = filesToPreload;

}

void ImageCore::requestCachingFile(const QString &filePath)
{
    //check if image is already loaded or requested
    if (QPixmapCache::find(filePath, nullptr) || lastFilesPreloaded.contains(filePath))
        return;

    QFile imgFile(filePath);
    if (imgFile.size() > QPixmapCache::cacheLimit()/2)
        return;

    auto *cacheFutureWatcher = new QFutureWatcher<ReadData>();
    connect(cacheFutureWatcher, &QFutureWatcher<ReadData>::finished, this, [cacheFutureWatcher, this](){
        addToCache(cacheFutureWatcher->result());
        cacheFutureWatcher->deleteLater();
    });
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    cacheFutureWatcher->setFuture(QtConcurrent::run(this, &ImageCore::readFile, filePath, true));
#else
    cacheFutureWatcher->setFuture(QtConcurrent::run(&ImageCore::readFile, this, filePath, true));
#endif
}

void ImageCore::addToCache(const ReadData &readData)
{
    if (readData.pixmap.isNull())
        return;

    QPixmapCache::insert(readData.fileInfo.absoluteFilePath(), readData.pixmap);

//    auto *size = new qint64(readData.fileInfo.size());
//    qvApp->setPreviouslyRecordedFileSize(readData.fileInfo.absoluteFilePath(), size);
//    qvApp->setPreviouslyRecordedImageSize(readData.fileInfo.absoluteFilePath(), new QSize(readData.size));
}

void ImageCore::jumpToNextFrame()
{
    if (currentFileDetails.isMovieLoaded)
        loadedMovie.jumpToNextFrame();
}

void ImageCore::setPaused(bool desiredState)
{
    if (currentFileDetails.isMovieLoaded)
        loadedMovie.setPaused(desiredState);
}

void ImageCore::setSpeed(int desiredSpeed)
{
    if (desiredSpeed < 0)
        desiredSpeed = 0;

    if (desiredSpeed > 1000)
        desiredSpeed = 1000;

    if (currentFileDetails.isMovieLoaded)
        loadedMovie.setSpeed(desiredSpeed);
}

void ImageCore::rotateImage(int rotation)
{
        currentRotation += rotation;

        // normalize between 360 and 0
        currentRotation = (currentRotation % 360 + 360) % 360;
        QTransform transform;

        QImage transformedImage;
        if (currentFileDetails.isMovieLoaded)
        {
            transform.rotate(currentRotation);
            transformedImage = loadedMovie.currentImage().transformed(transform);
        }
        else
        {
            transform.rotate(rotation);
            transformedImage = loadedPixmap.toImage().transformed(transform);
        }

        loadedPixmap.convertFromImage(transformedImage);

        currentFileDetails.loadedPixmapSize = QSize(loadedPixmap.width(), loadedPixmap.height());
        emit updateLoadedPixmapItem();
}

QImage ImageCore::matchCurrentRotation(const QImage &imageToRotate)
{
    if (!currentRotation)
        return imageToRotate;

    QTransform transform;
    transform.rotate(currentRotation);
    return imageToRotate.transformed(transform);
}

QPixmap ImageCore::matchCurrentRotation(const QPixmap &pixmapToRotate)
{
    if (!currentRotation)
        return pixmapToRotate;

    return QPixmap::fromImage(matchCurrentRotation(pixmapToRotate.toImage()));
}

QPixmap ImageCore::scaleExpensively(const int desiredWidth, const int desiredHeight)
{
    return scaleExpensively(QSizeF(desiredWidth, desiredHeight));
}

QPixmap ImageCore::scaleExpensively(const QSizeF desiredSize)
{
    if (!currentFileDetails.isPixmapLoaded)
        return QPixmap();

    QSize size = QSize(loadedPixmap.width(), loadedPixmap.height());
    size.scale(desiredSize.toSize(), Qt::KeepAspectRatio);

    // Get the current frame of the animation if this is an animation
    QPixmap relevantPixmap;
    if (!currentFileDetails.isMovieLoaded)
    {
        relevantPixmap = loadedPixmap;
    }
    else
    {
        relevantPixmap = loadedMovie.currentPixmap();
        relevantPixmap = matchCurrentRotation(relevantPixmap);
    }

    // If we are really close to the original size, just return the original
    if (abs(desiredSize.width() - relevantPixmap.width()) < 1 &&
        abs(desiredSize.height() - relevantPixmap.height()) < 1)
    {
        return relevantPixmap;
    }

    return relevantPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);;
}


void ImageCore::settingsUpdated()
{
//    auto &settingsManager = qvApp->getSettingsManager();
//
//    //loop folders
//    isLoopFoldersEnabled = settingsManager.getBoolean("loopfoldersenabled");
//
//    //preloading mode
//    preloadingMode = settingsManager.getInteger("preloadingmode");
//    switch (preloadingMode) {
//    case 1:
//    {
//        QPixmapCache::setCacheLimit(51200);
//        break;
//    }
//    case 2:
//    {
//        QPixmapCache::setCacheLimit(204800);
//        break;
//    }
//    }
//
//    //sort mode
//    sortMode = settingsManager.getInteger("sortmode");
//
//    //sort ascending
//    sortDescending = settingsManager.getBoolean("sortdescending");
//
//    //update folder info to re-sort
//    updateFolderInfo();
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
