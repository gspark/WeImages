#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class NavDockWidget;
class QFileSystemModel;
class ImageCore;
class QStatusBar;
class QLabel;

class MainWindow : public QMainWindow
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

    void saveWindowInfo();

    QString getWeChatImagePath();

public slots:
    void about();

private slots:

signals:
    void treeViewClicked(const QString path);
};
#endif // MAINWINDOW_H
