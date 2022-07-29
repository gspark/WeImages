#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>

class ImageCore;
class ImageSwitcher;
class QLabel;
class QScrollArea;
class QAction;
class ImageReadData;


enum ImageLoadType { normal, flip, rotate, zoomIn, zoomOut, extend };

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit ImageViewer(ImageCore* imageCore, ImageSwitcher* imageSwitcher, QWidget* parent = nullptr);

    ~ImageViewer();

private slots:
    void on_readPrevImage_clicked();
    void on_readNextImage_clicked();

    void on_refreshImage_clicked();

    void initImageParam();

    void on_rotateImage_r_clicked();
    void on_rotateImage_l_clicked();

    void on_vflipImage_clicked();

    void on_hflipImage_clicked();

    void on_exportImage_clicked();

    void on_extendImage_clicked();
    void on_zoomInImage_clicked();
    void on_zoomOutImage_clicked();
    void on_delayLoadFile();
private:
    typedef struct {
        // 是否水平
        bool horizontal;
        // 翻转次数
        int dirH;
        int dirV;
    } Flip;

    typedef struct {
        // 是否右旋
        bool right;
        // 旋转次数
        int dirR;
        int dirL;
    } Rotate;

    //界面对象
    QLabel *imgArea;//图片显示区域
    QScrollArea *scrollArea;//图片展示窗口
    QLabel *fileIndexLabel;//文件索引Label
    QLabel *filePathLabel;//文件路径Label
    QLabel *fileSizeLabel;//文件大小Label
    QLabel *fileModDateLabel;//文件大小Label
    QLabel *imageSizeLabel;//图片尺寸Label
    QLabel *imageScaleLabel;//图片缩放比Label

    QAction* _exportImageAct;

    ImageSwitcher* _imageSwitcher;
    ImageCore* _imageCore;

    ImageReadData* _originImage;

    QPixmap _currentPixmap;

    //缩放比
    double _scale;

    Flip* _flip;

    Rotate* _rotate;

    //UI创建及初始化
    void initUI();
    //初始化工具栏
    void initToolBar();
    //初始化状态栏
    void initStatusBar();

    void loadFile(const QString& absoluteFilePath);

    void loadImage(ImageLoadType loadType);

    QPixmap resizeImage(const QPixmap& pixmap);

    void displayImage(const QPixmap pixmap);

    double computeScaleWithView(const QPixmap pixmap);

    QPixmap flipImage(const QPixmap originPixmap);

    QPixmap rotateImage(const QPixmap& originPixmap);

    void keyPressEvent(QKeyEvent* event);//按键事件

    void initFlip();
    void initRotate();
protected:

};
#endif // IMAGEVIEWER_H
