#ifndef IMAGECORE_H
#define IMAGECORE_H

#include <QObject>
#include <QImageReader>
#include <QPixmap>
#include <QMovie>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QTimer>
#include <QCache>
#include <QOpenGLContext>


class ImageCore : public QObject
{
    Q_OBJECT

public:
    struct FileDetails
    {
        QFileInfo fileInfo;
        QFileInfoList folderFileInfoList;
        int loadedIndexInFolder = -1;
        bool isLoadRequested = false;
        bool isPixmapLoaded = false;
        bool isMovieLoaded = false;
        QSize baseImageSize;
        QSize loadedPixmapSize;
    };

    struct ReadData
    {
        QPixmap pixmap;
        QFileInfo fileInfo;
        QSize size;
    };

    explicit ImageCore(QObject* parent = nullptr);

    void loadFile(const QString& fileName);

    ReadData readFile(const QString& fileName, bool forCache);
    ReadData readFileSize(const QString& fileName, bool forCache, const QSize& targetSize);
    void loadPixmap(const ReadData& readData, bool fromCache);
    void addToCache(const ReadData& readImageAndFileInfo);
signals:
    void fileDataChanged(const QPixmap& readData);
private:
    QFutureWatcher<ReadData> loadFutureWatcher;

    BYTE* datConverImage(const QString& datFileName, long long fileSize);

    void XOR(BYTE* v_pbyBuf, DWORD v_dwBufLen, BYTE byXOR);
};

#endif // IMAGECORE_H
