#ifndef IMAGECORE_H
#define IMAGECORE_H

#include <QObject>
#include <QImageReader>
#include <QPixmap>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QOpenGLContext>

#define THUMBNAIL_WIDE 112
#define THUMBNAIL_HEIGHT 96

#define THUMBNAIL_WIDE_N 192
#define THUMBNAIL_HEIGHT_N 162

#define ICON_WIDE 48
#define ICON_HEIGHT 48

class ImageCore : public QObject
{
    Q_OBJECT

public:
    //struct FileDetails
    //{
    //    QFileInfo fileInfo;
    //    QFileInfoList folderFileInfoList;
    //    int loadedIndexInFolder = -1;
    //    bool isLoadRequested = false;
    //    bool isPixmapLoaded = false;
    //    bool isMovieLoaded = false;
    //    QSize baseImageSize;
    //    QSize loadedPixmapSize;
    //};

    struct ReadData
    {
        QPixmap pixmap;
        QFileInfo fileInfo;
    };

    explicit ImageCore(QObject* parent = nullptr);

    //************************************
    // Method:    loadFile
    // Returns:   void
    // Parameter: const QString & fileName
    // 图片异步返回
    //************************************
    void loadFile(const QString& fileName, const QSize& targetSize = QSize(THUMBNAIL_WIDE, THUMBNAIL_HEIGHT));

    ReadData readFile(const QString& fileName, bool forCache, const QSize& targetSize = QSize(THUMBNAIL_WIDE, THUMBNAIL_HEIGHT));
    //ReadData readFileSize(const QString& fileName, bool forCache, const QSize& targetSize);
    void loadPixmap(const ReadData& readData, bool fromCache);
    void addToCache(const ReadData& readImageAndFileInfo);

    bool isImageFile(const QFileInfo& fileInfo);

    QStringList imageFileNames();

    bool isWeChatImage(const QString& extension, const QString& fileName);

signals:
    void imageLoaded(const QPixmap& readData);
private:
    QFutureWatcher<ReadData> loadFutureWatcher;

    BYTE* datConverImage(const QString& datFileName, long long fileSize);

    void XOR(BYTE* v_pbyBuf, DWORD v_dwBufLen, BYTE byXOR);
};

#endif // IMAGECORE_H
