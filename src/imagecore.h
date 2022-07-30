#ifndef IMAGECORE_H
#define IMAGECORE_H

#include <QObject>
#include <QImageReader>
#include <QPixmap>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QOpenGLContext>
#include <QCache>

#define THUMBNAIL_WIDE 112
#define THUMBNAIL_HEIGHT 96

#define THUMBNAIL_WIDE_N 192
#define THUMBNAIL_HEIGHT_N 162

#define ICON_WIDE 48
#define ICON_HEIGHT 48

struct ImageReadData
{
    QPixmap pixmap;
    QFileInfo fileInfo;
    QString suffix;
    uint64_t hash = 0;
};

// 自定义数据类型需注册才能放入QVariant
Q_DECLARE_METATYPE(ImageReadData);

class QMimeDatabase;


class ImageCore : public QObject
{
    Q_OBJECT

public:
    explicit ImageCore(QObject* parent = nullptr);
    ~ImageCore();

    //************************************
    // Method:    loadFile
    // Returns:   void
    // Parameter: const QString & fileName
    // 图片异步返回
    //************************************
    void loadFile(const QString& fileName, const QSize& targetSize = QSize(THUMBNAIL_WIDE, THUMBNAIL_HEIGHT));

    ImageReadData* readFile(const QString& fileName, const QSize& targetSize = QSize(THUMBNAIL_WIDE, THUMBNAIL_HEIGHT));

    QPixmap readWeImage(const QString& fileName, long long fileSize, QString& extension, const QSize& targetSize);

    void loadPixmap(const ImageReadData* readData);

    void addToCache(const ImageReadData& readImageAndFileInfo);

    bool isImageFile(const QFileInfo& fileInfo);

    QStringList imageNames();

    bool isWeChatImage(const QFileInfo& fileInfo);

    QPixmap scaled(const QPixmap& originPixmap, const QSize& targetSize);

    QPixmap flipImage(const QPixmap originPixmap, bool horizontal = true, int dir = 1);

    QPixmap rotateImage(const QPixmap& originPixmap, bool right = true, int dir = 1);

    int exportWeChatImage(const QFileInfo& soureFile, const QString& targetPath);
signals:
    void imageLoaded(ImageReadData* readData);
private:
    QCache<uint64_t, ImageReadData>* _imageReadDataCache;

    QMimeDatabase* _mineDb;

    QFutureWatcher<ImageReadData*> loadFutureWatcher;

    BYTE* datConverImage(const QString& datFileName, long long fileSize, QString* extension);

    void XOR(BYTE* v_pbyBuf, DWORD v_dwBufLen, BYTE byXOR);

    bool findImageReadData(uint64_t& hash, const QString& absoluteFilePath, const QSize& targetSize);
    ImageReadData* getImageReadData(uint64_t hash);
};

#endif // IMAGECORE_H
