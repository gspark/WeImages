#include "mainwindow.h"
#include "filewidget.h"
#include "config.h"
#include "navdockwidget.h"

#include <QApplication>
#include <QAction>
#include <QScreen>
#include <QFileDialog>
#include <QDockWidget>
#include <QMessageBox>
#include <QSplitter>
#include <QToolBar>
#include <QMenuBar>
#include <QWidget>
#include <QMenu>
#include <QActionGroup>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QFileSystemModel>


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(tr("WeImages"));

    this->imageCore = new ImageCore();

    fileModel = new QFileSystemModel();
    navDock = new NavDockWidget(fileModel, imageCore);

    fileModelInit();
    setupWidgets();
    setupMenuBar();

    connect(qApp, &QCoreApplication::aboutToQuit, this, &MainWindow::saveWindowInfo);

    loadWindowInfo();
}

MainWindow::~MainWindow() {

    navDock->deleteLater();
    fileModel->deleteLater();
    delete this->imageCore;
}


void MainWindow::fileModelInit() {
#if DISABLE_FILE_WATCHER
    fileModel->setOptions(QFileSystemModel::DontWatchForChanges);
#endif

    fileModel->setRootPath("");
    /*
     * default QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs
     * add QDir::System will show all shortcut(.lnk file) and system files
     * add QDir::Hidden will show some files that not visible on Windows, these files may be modified by mistake.
     */
    //fileModel->setFilter(QDir::AllEntries | QDir::NoDot | QDir::AllDirs | QDir::System/* | QDir::Hidden*/);
    fileModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::System | QDir::Hidden);
    //fileModel->setNameFilters(QStringList() << "*.jpg" << "*.gif" << "*.png" << "*.dat");
    fileModel->setNameFilters(this->imageCore->imageFileNames());
    fileModel->setReadOnly(true);
}

void MainWindow::setupWidgets() {
    statusBar()->showMessage(tr("Ready"));

    // central widget
    auto *widget = new FileWidget(/*this->fileModel, */this->imageCore, this);
    connectShortcut(widget);

    //mainLayout->addWidget(widget);
    //mainLayout->addWidget(statusBar);
    setCentralWidget(widget);
    //setLayout(mainLayout);

    // dock
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    // navigation dock
    navDock->setObjectName(OBJECTNAME_NAV_DOCK);
    // show in the dock
    // navDock->setWindowTitle(tr("Navigation Bar"));
    navDock->setMinimumWidth(210);
    addDockWidget(Qt::LeftDockWidgetArea, navDock);
    connect(navDock, &NavDockWidget::treeViewClicked, widget, &FileWidget::onTreeViewClicked);
    connect(widget, &FileWidget::onCdDir, navDock, &NavDockWidget::onCdDir);
}

void MainWindow::setupMenuBar() {
    // file menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *exitAction = fileMenu->addAction(tr("E&xit"), qApp, &QCoreApplication::quit, Qt::QueuedConnection);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setShortcut(Qt::CTRL | Qt::Key_Q);

    // view menu
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    navDock->toggleViewAction()->setText(tr("Navi&gation Bar"));
    navDock->toggleViewAction()->setShortcut(Qt::CTRL | Qt::Key_G);
    viewMenu->addAction(navDock->toggleViewAction());

    viewMenu->addSeparator();

//    FileWidget *widget = (FileWidget *)((QSplitter *)centralWidget())->widget(0);
    auto *widget = (FileWidget *) centralWidget();
  
    auto *areaActions = new QActionGroup(this);
    areaActions->setExclusive(true);
    // help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
//    aboutAct->setToolTip(tr("Show the application's About box"));
    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
//    aboutQtAct->setToolTip(tr("Show the Qt library's About box"));
}


void MainWindow::connectShortcut(QWidget *widget) {

}


// show about message
void MainWindow::about() {
    static const char message[] =
            "<p><b>WeChatImages</b></p>"

            "<p>Version:&nbsp;0.1(x64)</p>"
            "<p>Author:&nbsp;&nbsp;shrill</p>"
            "<p>Date:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2022/03/07</p>"

            "<p></p>"
//        "<p>Project:&nbsp;&nbsp;<a href=\"https://github.com/Jawez/FileManager\">Github repository</a>"
//        "<p>Video:&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"https://www.bilibili.com/video/BV1ng411L7gx\">BiliBili video</a>"
    ;

//    QMessageBox::about(this, tr("About"), message);
    auto *msgBox = new QMessageBox(this);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setWindowTitle(tr("About"));
    msgBox->setText(message);
    QPixmap pm(QLatin1String(":/resources/icon_app_64.png"));
    if (!pm.isNull())
        msgBox->setIconPixmap(pm);

    msgBox->exec();
}

void MainWindow::loadWindowInfo() {
    QVariant geometry = ConfigIni::getInstance().iniRead(QStringLiteral("Main/geometry"), "0");
//    qDebug() << geometry;
    if (geometry.isValid() && geometry.toInt() != 0) {
        bool result = restoreGeometry(geometry.toByteArray());
        qDebug() << QString("restoreGeometry result %1").arg(result);
    } else {
        // resize window
        QSize aSize = qGuiApp->primaryScreen()->availableSize();
        qDebug() << "loadWindowInfo aSize:" << aSize;
        resize(aSize * 0.618);
    }
//    QVariant state = ConfigIni::getInstance().iniRead(QStringLiteral("/state"), "0");
////    qDebug() << state;
//    if (state.isValid()) {
//        bool result = restoreState(state.toByteArray());
//        qDebug() << QString("restoreState result %1").arg(result);
//    }

//    QVariant size = readSettings(CONFIG_GROUP_WINDOW, CONFIG_WIN_SIZE);
//    qDebug() << size;
//    if (size.isValid()) {
//        resize(size.toSize());
//    } else {
//        // resize window
//        QSize aSize = qGuiApp->primaryScreen()->availableSize();
//        qDebug() << aSize;
//        resize(aSize * 0.618);
//    }

//    QVariant pos = readSettings(CONFIG_GROUP_WINDOW, CONFIG_WIN_POS);
//    qDebug() << pos;
//    if (pos.isValid()) {
//        move(pos.toPoint());
//    }

}

void MainWindow::saveWindowInfo() {
    navDock->saveDockInfo();
}

