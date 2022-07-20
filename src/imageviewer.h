#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>

class ImageCore;
class ImageSwitcher;
class QLabel;
class QScrollArea;

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit ImageViewer(ImageCore* imageCore, ImageSwitcher* imageSwitcher, QWidget* parent = nullptr);
    ~ImageViewer();

private slots:
    //void on_selectHomeDir_clicked();
    //void on_selectImageFile_clicked();
    void on_readPrevImage_clicked();
    void on_readNextImage_clicked();
    //void on_refreshImage_clicked();
    //void on_rotateImage_clicked();
    //void on_flipImage_clicked();
    //void on_zoomInImage_clicked();
    //void on_zoomOutImage_clicked();
    //void on_extendImage_clicked();

private:
    //界面对象
    QLabel *imgArea;//图片显示区域
    QScrollArea *scrollArea;//图片展示窗口
    QLabel *fileIndexLabel;//文件索引Label
    QLabel *filePathLabel;//文件路径Label
    QLabel *fileSizeLabel;//文件大小Label
    QLabel *fileModDateLabel;//文件大小Label
    QLabel *imageSizeLabel;//图片尺寸Label
    QLabel *imageScaleLabel;//图片缩放比Label

    ImageSwitcher* imageSwitcher;
    ImageCore* imageCore;

    //QSize imgSize;//图片尺寸
    //double scaleVar;//缩放比
    //UI创建及初始化
    void initUI();
    //初始化工具栏
    void initToolBar();
    //初始化状态栏
    void initStatusBar();

    void displayImage(QString absoluteFilePath);

    double computeScaleWithView(const QPixmap& pixmap);

    //void zoomInImage();
    //void zoomOutImage();
    //void extendImage();

    void keyPressEvent(QKeyEvent *event);//按键事件
protected:
    virtual void showEvent(QShowEvent* event);
};
#endif // IMAGEVIEWER_H
