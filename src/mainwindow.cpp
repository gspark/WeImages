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


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
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
    savaWindowInfo();
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
    auto *widget = new FileWidget(this->imageCore, this);
 
    setCentralWidget(widget);

    // dock
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    navDock->setMinimumWidth(210);
    navDock->setMaximumWidth(540);
    addDockWidget(Qt::LeftDockWidgetArea, navDock);

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

// show about message
void MainWindow::about() {
    static const char message[] =
        "<p><b>WeImages</b></p>"

        "<p>Version:&nbsp;0.1(x64)</p>"
        "<p>Author:&nbsp;&nbsp;shrill</p>"
        "<p>Date:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2022-03-07</p>"

        "<p></p>"
        "<p>Project:&nbsp;&nbsp;<a href=\"https://github.com/Jawez/FileManager\">Github repository</a>"
        ;

    auto *msgBox = new QMessageBox(this);
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setWindowTitle(tr("About"));
    msgBox->setText(message);
    msgBox->exec();
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
    } else {
        // resize window
        QSize aSize = qGuiApp->primaryScreen()->availableSize();
        resize(aSize * 0.618);
    }

    QString path = ConfigIni::getInstance().iniRead(QStringLiteral("Main/path"), "").toString();
    if (path.isEmpty()) {
        path = getWeChatImagePath();
    }
    QFileInfo f(path);
    if (!f.isDir())
    {
        path = getWeChatImagePath();
    }
    ConfigIni::getInstance().iniWrite(QStringLiteral("Main/path"), path);
    emit setPath(path);
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
    filePathLabel->setText(readData->fileInfo.absoluteFilePath());
    fileSizeLabel->setText(fileSizeToString(readData->fileInfo.size()));
}

