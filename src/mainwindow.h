#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QShortcut>
#include <QCloseEvent>


#define MAX_TOOLBAR_COUNT           10

#define OBJECTNAME_NAV_DOCK         "Navigation Bar"
#define OBJECTNAME_FILE_DOCK        "File Dock"
#define OBJECTNAME_TOOLBAR          "Quick Button"

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
    QToolBar* toolBar;
    QStringList toolBarList;

    NavDockWidget *navDock;
    QFileSystemModel *fileModel;

    void fileModelInit();

    void setupToolBar();
    void setupMenuBar();
    void setupWidgets();
    void toolBarAddAction(bool addDir = true);

    void connectShortcut(QWidget *widget);

    void loadWindowInfo();

    void saveWindowInfo();

    ImageCore* imageCore;

public slots:
    void about();

private slots:
    void onToolBarActionTriggered(QAction *action);
    void toolBarOnTextMenu(const QPoint &pos);
};
#endif // MAINWINDOW_H
