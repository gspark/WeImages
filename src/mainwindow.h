#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "component/wxwindow.h"

class NavDockWidget;
class QFileSystemModel;
class ImageCore;
class QStatusBar;
class QLabel;
class ImageReadData;

class MainWindow : public WxWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    NavDockWidget *navDock;
    QFileSystemModel *fileModel;

    //文件索引Label
    QLabel* fileIndexLabel;
    //文件路径Label
    QLabel* filePathLabel;
    //文件大小Label
    QLabel* fileSizeLabel;


    ImageCore* imageCore;

    void fileModelInit();

    //初始化状态栏
    void initStatusBar();

    void setupMenuBar();
    void setupWidgets();

    void loadWindowInfo();
    void savaWindowInfo();

    QString getWeChatImagePath();

    void setupToolBar();

private slots:
    void about();
    void onCdDir(const QString path);
    void imageLoaded(ImageReadData* readData);
    void onCdWechatImage();
signals:
    void setPath(const QString path);
};
#endif // MAINWINDOW_H
