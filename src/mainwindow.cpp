#include "mainwindow.h"
#include "filewidget.h"
#include "config.h"
#include "navdockwidget.h"

#include <QtDebug>
#include <QApplication>
#include <QTranslator>
#include <QProcess>
#include <QAction>
#include <QScreen>
#include <QFileIconProvider>
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
    setWindowTitle(tr("WeChatImages"));

    this->imageCore = new ImageCore();

    fileModel = new QFileSystemModel();
    navDock = new NavDockWidget(fileModel, imageCore);

    fileModelInit();
    setupWidgets();
    setupToolBar();
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
    fileModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs | QDir::System/* | QDir::Hidden*/);
    fileModel->setNameFilters(QStringList() << "*.jpg" << "*.gif" << "*.png" << "*.dat");
    fileModel->setReadOnly(true);
}

void MainWindow::setupWidgets() {
    statusBar()->showMessage(tr("Ready"));

    //auto mainLayout = new QVBoxLayout();

    // central widget
    auto *widget = new FileWidget(this->fileModel, this->imageCore, this);
    connectShortcut(widget);

    //mainLayout->addWidget(widget);
    //mainLayout->addWidget(statusBar);
    setCentralWidget(widget);
    //setLayout(mainLayout);

    // dock
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    // navigation dock
    navDock->setObjectName(OBJECTNAME_NAV_DOCK);
    //navDock->setWindowTitle(tr("Navigation Bar"));  // show in the dock
    addDockWidget(Qt::LeftDockWidgetArea, navDock);
    connect(navDock, &NavDockWidget::navDockClicked, widget, &FileWidget::onNavigateBarClicked);
}

void MainWindow::setupToolBar() {
    toolBar = addToolBar(tr("Quick Button"));
    toolBar->setObjectName(OBJECTNAME_TOOLBAR);
//    toolBar->setIconSize(QSize(20, 30));
//    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolBar->setIconSize(QSize(15, 20));
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(toolBar, &QToolBar::customContextMenuRequested, this, &MainWindow::toolBarOnTextMenu);


//    foreach (QFileInfo info, QDir::drives()) {
//        QString path = info.absolutePath();     // example "C:/", file's path absolute path. This doesn't include the file name
//        path = QDir::toNativeSeparators(path);  // example "C:\\"
//        toolBar->addAction(fileModel->iconProvider()->icon(info), path);
//    }
//    toolBar->addAction("+");

    connect(toolBar, &QToolBar::actionTriggered, this, &MainWindow::onToolBarActionTriggered);
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

    toolBar->toggleViewAction()->setText(tr("&Toolbar"));
    toolBar->toggleViewAction()->setShortcut(Qt::CTRL | Qt::Key_B);
    viewMenu->addAction(toolBar->toggleViewAction());

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


void MainWindow::toolBarAddAction(bool addDir) {
    if (toolBarList.count() >= MAX_TOOLBAR_COUNT) {
        return;
    }

    QString path;
    QString dialogName = tr("Add Button");
    if (addDir)
        path = QFileDialog::getExistingDirectory(this, dialogName);
    else
        path = QFileDialog::getOpenFileName(this, dialogName);
    path = QDir::toNativeSeparators(path);

    if (QFileInfo::exists(path) && !toolBarList.contains(path, Qt::CaseInsensitive)) {
        qDebug() << "add quick path: " << path;

        QFileInfo info(path);
        QString text = QDir::toNativeSeparators(info.fileName().isEmpty() ? path : info.fileName());
        QAction *newAct = new QAction;
        newAct->setIcon(fileModel->iconProvider()->icon(info));
        newAct->setText(text);
        newAct->setToolTip(path);

        // get the "+" action
        QAction *action = toolBar->actions().at(toolBarList.count());
        toolBar->insertAction(action, newAct);

        toolBarList.append(path);
        if (toolBarList.count() >= MAX_TOOLBAR_COUNT) {
            action->setEnabled(false);
        }
    } else {
        qDebug() << "quick path exist: " << path;
    }
}

void MainWindow::onToolBarActionTriggered(QAction *action) {
    QString path = action->toolTip();
    if (QFileInfo::exists(path)) {
        qDebug() << QString("tool bar clicked %1").arg(path);
//        ((FileWidget *)centralWidget())->onNavigateBarClicked(path);
        ((FileWidget *) centralWidget())->onItemActivated(path);
    } else {
        toolBarAddAction();
    }
}

void MainWindow::toolBarOnTextMenu(const QPoint &pos) {
    QMenu menu;
    QAction *action;

    // menu actions
    QAction *addDirAction = nullptr;
    QAction *addFileAction = nullptr;
//    QAction *deleteAction = nullptr;
    QAction *deleteAllAction = nullptr;

    addDirAction = menu.addAction(tr("&Add Directory"));
    if (toolBarList.count() >= MAX_TOOLBAR_COUNT) {
        addDirAction->setEnabled(false);
    }
    addFileAction = menu.addAction(tr("Add &File"));
    if (toolBarList.count() >= MAX_TOOLBAR_COUNT) {
        addFileAction->setEnabled(false);
    }

    menu.addSeparator();

    QMenu *deleteMenu = menu.addMenu(tr("&Delete"));
    deleteAllAction = menu.addAction(tr("D&elete All"));
    if (toolBarList.isEmpty()) {
        deleteMenu->setEnabled(false);
        deleteAllAction->setEnabled(false);
    } else {
        for (int i = 0; i < toolBarList.count(); i++) {
            QString path = toolBarList.at(i);
            QAction *deleteAction = deleteMenu->addAction(path);
            deleteAction->setData(i);
        }
    }


    qDebug() << "toolBarOnTextMenu";

    action = menu.exec(toolBar->mapToGlobal(pos));
    if (!action)
        return;

    // handle all selected items
    if (action == addDirAction) {
        toolBarAddAction();

    } else if (action == addFileAction) {
        toolBarAddAction(false);

    } else if (action == deleteAllAction) {
        toolBar->clear();
        toolBar->addAction("+");
        toolBarList.clear();
//        if (toolBarList.count() < MAX_TOOLBAR_COUNT) {
        QAction *action = toolBar->actions().at(toolBarList.count());
        action->setEnabled(true);
//        }

    } else if (action != NULL) {
        int index = action->data().toInt();
        qDebug() << QString("delete [%1]: %2").arg(index).arg(action->text());

        toolBar->removeAction(toolBar->actions().at(index));
        toolBarList.removeAt(index);
        if (toolBarList.count() < MAX_TOOLBAR_COUNT) {
            QAction *action = toolBar->actions().at(toolBarList.count());
            action->setEnabled(true);
        }
    }
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
    QVariant geometry = ConfigIni::getInstance().iniRead(CONFIG_GROUP_WINDOW, CONFIG_WIN_GEOMETRY);
//    qDebug() << geometry;
    if (geometry.isValid()) {
        bool result = restoreGeometry(geometry.toByteArray());
        qDebug() << QString("restoreGeometry result %1").arg(result);
    } else {
        // resize window
        QSize aSize = qGuiApp->primaryScreen()->availableSize();
        qDebug() << aSize;
        resize(aSize * 0.618);
    }
    QVariant state = ConfigIni::getInstance().iniRead(CONFIG_GROUP_WINDOW, CONFIG_WIN_STATE);
//    qDebug() << state;
    if (state.isValid()) {
        bool result = restoreState(state.toByteArray());
        qDebug() << QString("restoreState result %1").arg(result);
    }

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

    QStringList pathList;// = readArraySettings(CONFIG_GROUP_TOOLBAR);
    qDebug() << "pathList: " << pathList;
    if (pathList.isEmpty()) {
                foreach (QFileInfo info, QDir::drives()) {
                QString path = info.absolutePath();     // example "C:/", file's path absolute path. This doesn't include the file name
                path = QDir::toNativeSeparators(path);  // example "C:\\"
                toolBar->addAction(fileModel->iconProvider()->icon(info), path);

                toolBarList.append(path);
            }
    } else {
                foreach (QString path, pathList) {
                path = QDir::toNativeSeparators(path);
//            toolBar->addAction(fileModel->iconProvider()->icon(QFileInfo(path)), path);

                QFileInfo info(path);
                QString text = QDir::toNativeSeparators(info.fileName().isEmpty() ? path : info.fileName());
                QAction *newAct = new QAction;
                newAct->setIcon(fileModel->iconProvider()->icon(info));
                newAct->setText(text);
                newAct->setToolTip(path);
                toolBar->addAction(newAct);

                toolBarList.append(path);
            }
    }
//    toolBar->addSeparator();
    toolBar->addAction("+");
}

void MainWindow::saveWindowInfo() {
    navDock->saveDockInfo();
}

