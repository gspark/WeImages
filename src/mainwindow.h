#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class NavDockWidget;
class QFileSystemModel;
class ImageCore;
class QStatusBar;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    NavDockWidget *navDock;
    QFileSystemModel *fileModel;

    void fileModelInit();

    void setupMenuBar();
    void setupWidgets();

    void connectShortcut(QWidget *widget);

    void loadWindowInfo();

    void saveWindowInfo();

    ImageCore* imageCore;

public slots:
    void about();

private slots:

};
#endif // MAINWINDOW_H
