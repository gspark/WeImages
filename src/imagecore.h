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

struct ImageReadData
{
    QPixmap pixmap;
    QFileInfo fileInfo;
    QString suffix;
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

    ImageReadData readFile(const QString& fileName, bool forCache, const QSize& targetSize = QSize(THUMBNAIL_WIDE, THUMBNAIL_HEIGHT));
    //ReadData readFileSize(const QString& fileName, bool forCache, const QSize& targetSize);
    void loadPixmap(const ImageReadData& readData, bool fromCache);
    void addToCache(const ImageReadData& readImageAndFileInfo);

    bool isImageFile(const QFileInfo& fileInfo);

    QStringList imageFileNames();

    bool isWeChatImage(const QFileInfo& fileInfo);

signals:
    void imageLoaded(ImageReadData* readData);
private:
    QMimeDatabase* _mineDb;

    QFutureWatcher<ImageReadData> loadFutureWatcher;

    BYTE* datConverImage(const QString& datFileName, long long fileSize, QString* extension);

    void XOR(BYTE* v_pbyBuf, DWORD v_dwBufLen, BYTE byXOR);
};

#endif // IMAGECORE_H
