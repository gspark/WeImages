#include "mainwindow.h"
#include "filewidget.h"
#include "config.h"
#include "navdockwidget.h"
#include "filesystemhelperfunctions.h"

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
#include <QSettings>
#include <QStandardPaths>


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(tr("WeImages"));

    this->imageCore = new ImageCore();

    fileModel = new QFileSystemModel();
    navDock = new NavDockWidget(fileModel, imageCore);

    fileModelInit();
    setupMenuBar();
    setupWidgets();
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
    fileModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::System | QDir::Hidden);
    fileModel->setNameFilters(this->imageCore->imageFileNames());
    fileModel->setReadOnly(true);
}

void MainWindow::initStatusBar()
{
    fileIndexLabel = new QLabel();
    filePathLabel = new QLabel();
    fileSizeLabel = new QLabel();

    statusBar()->addWidget(fileIndexLabel, 0);
    statusBar()->addWidget(filePathLabel, 1);
    statusBar()->addWidget(fileSizeLabel, 0);
}

void MainWindow::setupWidgets() {
    initStatusBar();

    // central widget
    auto *widget = new FileWidget(/*this->fileModel, */this->imageCore, this);
 
    setCentralWidget(widget);

    // dock
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    // navigation dock
    //navDock->setObjectName(OBJECTNAME_NAV_DOCK);
    // show in the dock
    // navDock->setWindowTitle(tr("Navigation Bar"));
    navDock->setMinimumWidth(210);
    addDockWidget(Qt::LeftDockWidgetArea, navDock);
    connect(navDock, &NavDockWidget::treeViewClicked, widget, &FileWidget::onTreeViewClicked);
    connect(widget, &FileWidget::onCdDir, navDock, &NavDockWidget::onCdDir);
    connect(this, &MainWindow::onCdDir, widget, &FileWidget::onTreeViewClicked);
    connect(this, &MainWindow::onCdDir, navDock, &NavDockWidget::onCdDir);
    connect(widget, &FileWidget::onCdDir, this, &MainWindow::onCdDired);
    connect(navDock, &NavDockWidget::treeViewClicked, this, &MainWindow::onCdDired);

    connect(this, &MainWindow::showed, this, &MainWindow::onShowed, Qt::QueuedConnection);
    connect(this->imageCore, &ImageCore::imageLoaded, this, &MainWindow::imageLoaded);
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
    aboutAct->setToolTip(tr("Show the application's About box"));
}

// show about message
void MainWindow::about() {
    static const char message[] =
            "<p><b>WeImages</b></p>"

            "<p>Version:&nbsp;0.1(x64)</p>"
            "<p>Author:&nbsp;&nbsp;shrill</p>"
            "<p>Date:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2022/03/07</p>"

            "<p></p>"
        //"<p>Project:&nbsp;&nbsp;<a href=\"https://github.com/Jawez/FileManager\">Github repository</a>"
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

void MainWindow::onCdDired(const QString path)
{
    ConfigIni::getInstance().iniWrite(QStringLiteral("Main/path"), path);
    filePathLabel->setText(path);
}

void MainWindow::loadWindowInfo() {
    QVariant geometry = ConfigIni::getInstance().iniRead(QStringLiteral("Main/geometry"), "0");
//    qDebug() << geometry;
    if (geometry.isValid() && geometry.toInt() != 0) {
        bool result = restoreGeometry(geometry.toByteArray());
    } else {
        // resize window
        QSize aSize = qGuiApp->primaryScreen()->availableSize();
        resize(aSize * 0.618);
    }
    emit showed();
}

QString MainWindow::getWeChatImagePath()
{
    QSettings setting("HKEY_CURRENT_USER\\SOFTWARE\\Tencent\\WeChat", QSettings::NativeFormat);
    QString fileSavePath = setting.value("FileSavePath").toString();
    if (fileSavePath == "MyDocument:")
    {
        QStringList list = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (list.isEmpty())
        {
            return fileSavePath;
        }

        QString path = list[0] + QDir::separator() + "WeChat Files";
        QFileInfo link(path);
        if (link.isDir() && link.isJunction())
        {
            path = link.junctionTarget();
        }
        QList<QFileInfo> files = QDir{ path }.entryInfoList(QDir::Dirs | QDir::Hidden | QDir::System, QDir::NoSort);
        for (const auto& f : files)
        {
            if (f.baseName().startsWith("wxid_"))
            {
                return f.absoluteFilePath() + QDir::separator() + "FileStorage" + QDir::separator() + "Image";
            }
        }
    }
    return fileSavePath;
}

void MainWindow::onShowed()
{
    QString path = ConfigIni::getInstance().iniRead(QStringLiteral("Main/path"), "").toString();
    if (path.isEmpty()) {
        path = getWeChatImagePath();
    }
    ConfigIni::getInstance().iniWrite(QStringLiteral("Main/path"), path);
    filePathLabel->setText(path);
    emit onCdDir(path);
}

void MainWindow::imageLoaded(ImageReadData* readData)
{
    filePathLabel->setText(readData->fileInfo.absoluteFilePath());
    fileSizeLabel->setText(fileSizeToString(readData->fileInfo.size()));
}

