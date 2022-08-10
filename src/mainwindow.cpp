#include "mainwindow.h"
#include "filewidget.h"
#include "config.h"
#include "navdockwidget.h"
#include "filesystemhelperfunctions.h"
#include "aboutdialog.h"
#include "iconhelper.h"

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
#include <QtConcurrent/QtConcurrent>


MainWindow::MainWindow(QWidget* parent) : WxWindow(parent)
{
    setWindowTitle(tr("WeImages"));
    // must be created first
    this->imageCore = new ImageCore();
    fileModelInit();
    setupWidgets();
    setupMenuBar();
    setupToolBar();
    loadWindowInfo();
}

MainWindow::~MainWindow() {
    savaWindowInfo();
    navDock->deleteLater();
    fileModel->deleteLater();
    delete this->imageCore;
    this->deleteLater();
}

void MainWindow::fileModelInit() {
    fileModel = new QFileSystemModel();
#if DISABLE_FILE_WATCHER
    fileModel->setOptions(QFileSystemModel::DontWatchForChanges);
#endif
    fileModel->setRootPath("");
    fileModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::System | QDir::Hidden);
    fileModel->setNameFilters(this->imageCore->imageNames());
    fileModel->setReadOnly(true);
}

void MainWindow::setupWidgets() {
    // dock
    navDock = new NavDockWidget(fileModel, this->imageCore);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    navDock->setMinimumWidth(220);
    navDock->setMaximumWidth(540);
    addDockWidget(Qt::LeftDockWidgetArea, navDock);

    // central widget
    auto *widget = new FileWidget(this->imageCore, this);
    setCentralWidget(widget);

    initStatusBar();

    connect(this, &MainWindow::setPath, navDock, &NavDockWidget::onSetPath);

    connect(navDock, &NavDockWidget::treeViewClicked, widget, &FileWidget::onTreeViewClicked);
    connect(navDock, &NavDockWidget::treeViewClicked, this, &MainWindow::onCdDir);

    connect(widget, &FileWidget::cdDir, navDock, &NavDockWidget::onCdDir);
    connect(widget, &FileWidget::cdDir, this, &MainWindow::onCdDir);

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

    // help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setToolTip(tr("Show the application's About box"));
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


// show about message
void MainWindow::about() {
    QString strProductVersion;
    QString strFileVersion;
    if (!getFileVersionInfo(QCoreApplication::applicationFilePath(), strProductVersion, strFileVersion))
    {
        strProductVersion = "0.8";
        strFileVersion = "0.8";
    }
    AboutDialog::show(this, tr("About") + " WeImages ", "WeImages", strProductVersion);
}

void MainWindow::onCdDir(const QString path)
{
    ConfigIni::getInstance().iniWrite(QStringLiteral("Main/path"), path);
    filePathLabel->setText(path);
}

void MainWindow::loadWindowInfo() {

    QVariant geometry = ConfigIni::getInstance().iniRead(QStringLiteral("Main/geometry"), QVariant());
    if (geometry.isValid()) {
        bool result = restoreGeometry(geometry.toByteArray());
    }
    else {
        // resize window
        QSize aSize = qGuiApp->primaryScreen()->availableSize();
        resize(aSize * 0.618);
    }

    weChatPathFuture.setFuture(QtConcurrent::run([this]() -> QString {
        QString path = ConfigIni::getInstance().iniRead(QStringLiteral("Main/path"), "").toString();
        if (path.isEmpty()) {
            path = getWeChatImagePath();
        }
        QFileInfo f(path);
        if (!f.isDir())
        {
            path = getWeChatImagePath();
        }
        if (path.isEmpty())
        {
            path = QCoreApplication::applicationDirPath();
        }
        ConfigIni::getInstance().iniWrite(QStringLiteral("Main/path"), path);
        return path;
        }));

    connect(&weChatPathFuture, &QFutureWatcher<QString>::finished, this, [this]() {
        emit setPath(weChatPathFuture.result());
        });
}

void MainWindow::savaWindowInfo()
{
    ConfigIni::getInstance().iniWrite(QStringLiteral("Main/geometry"), this->saveGeometry());
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

void MainWindow::imageLoaded(ImageReadData* readData)
{
    if (nullptr == readData)
    {
        return;
    }
    filePathLabel->setText(readData->fileInfo.absoluteFilePath());
    fileSizeLabel->setText(fileSizeToString(readData->fileInfo.size()));
}

void MainWindow::setupToolBar()
{
    QToolBar* toolBar = this->toolBar();
    toolBar->setContentsMargins(0, 0, 0, 0);
    IconHelper::StyleColor styleColor;

    QAction* detailAction = toolBar->addAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 61574, 12, 16, 16)), tr("weixin"));
    connect(detailAction, &QAction::triggered, this, &MainWindow::onCdWechatImage);

    //toolBar = new QToolBar;
    //QAction* detailAction1 = toolBar->addAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 61498, 12, 16, 16)), tr("detail1"));
    //this->setToolBar(toolBar);
}

void MainWindow::onCdWechatImage()
{
    QString wechat = getWeChatImagePath();
    if (!wechat.isEmpty())
    {
        emit setPath(wechat);
    }
}

