#include "imageviewer.h"
#include "imagecore.h"
#include "models/imageswitcher.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QDateTime>
#include <QKeyEvent>
#include <QDragEnterEvent>
#include <QLabel>
#include <QScrollArea>
#include <QMimeData>
#include <QTimer>
//debug
//#include <QDebug>

ImageViewer::ImageViewer(ImageCore* imageCore, ImageSwitcher* imageSwitcher, QWidget* parent)
    : QMainWindow(parent)
{
    this->imageCore = imageCore;
    this->imageSwitcher = imageSwitcher;
    initUI();
}


ImageViewer::~ImageViewer()
{
}

void ImageViewer::initUI(){
    initToolBar();
    initStatusBar();

    imgArea = new QLabel();
    imgArea->setAlignment(Qt::AlignCenter);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imgArea);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->addWidget(scrollArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *window = new QWidget;
    window->setLayout(contentLayout);

    ////允许窗口外部拖入
    //// setAcceptDrops(true);
    setCentralWidget(window);
    //禁止工具栏右键菜单
    setContextMenuPolicy(Qt::NoContextMenu);
    setWindowTitle(tr("图片浏览器"));
    resize(1200, 800);
}

void ImageViewer::initToolBar(){
    QToolBar *fileToolBar = addToolBar(tr("工具栏"));
    fileToolBar->setMovable(false);//禁止工具栏拖动

    const QIcon openImageIcon = QIcon::fromTheme("document-new", QIcon(":/images/OpenImage.png"));
    QAction *openImageAct = new QAction(openImageIcon, tr("&选择图片"), this);
    openImageAct->setShortcuts(QKeySequence::New);
    //connect(openImageAct, &QAction::triggered, this, &ImageViewer::on_selectImageFile_clicked);
    fileToolBar->addAction(openImageAct);

    const QIcon openFolderIcon = QIcon::fromTheme("document-new", QIcon(":/images/OpenFolder.png"));
    QAction *openFolderAct = new QAction(openFolderIcon, tr("&选择目录"), this);
    openFolderAct->setShortcuts(QKeySequence::New);
    //connect(openFolderAct, &QAction::triggered, this, &ImageViewer::on_selectHomeDir_clicked);
    fileToolBar->addAction(openFolderAct);

    fileToolBar->addSeparator();

    const QIcon prevImageIcon = QIcon::fromTheme("document-new", QIcon(":/images/Back.png"));
    QAction *prevImageAct = new QAction(prevImageIcon, tr("&上一张"), this);
    prevImageAct->setShortcuts(QKeySequence::New);
    connect(prevImageAct, &QAction::triggered, this, &ImageViewer::on_readPrevImage_clicked);
    fileToolBar->addAction(prevImageAct);


    const QIcon nextImageIcon = QIcon::fromTheme("document-new", QIcon(":/images/Next.png"));
    QAction *nextImageAct = new QAction(nextImageIcon, tr("&下一张"), this);
    nextImageAct->setShortcuts(QKeySequence::New);
    connect(nextImageAct, &QAction::triggered, this, &ImageViewer::on_readNextImage_clicked);
    fileToolBar->addAction(nextImageAct);

    fileToolBar->addSeparator();

    const QIcon refreshImageIcon = QIcon::fromTheme("document-new", QIcon(":/images/Refresh.png"));
    QAction * refreshImageAct = new QAction(refreshImageIcon, tr("&刷新"), this);
    refreshImageAct->setShortcuts(QKeySequence::New);
    //connect(refreshImageAct, &QAction::triggered, this, &ImageViewer::on_refreshImage_clicked);
    fileToolBar->addAction(refreshImageAct);

    fileToolBar->addSeparator();

    const QIcon rotateImageIcon = QIcon::fromTheme("document-new", QIcon(":/images/Rotate.png"));
    QAction * rotateImageAct = new QAction(rotateImageIcon, tr("&旋转"), this);
    rotateImageAct->setShortcuts(QKeySequence::New);
    //connect(rotateImageAct, &QAction::triggered, this, &ImageViewer::on_rotateImage_clicked);
    fileToolBar->addAction(rotateImageAct);

    const QIcon flipImageIcon = QIcon::fromTheme("document-new", QIcon(":/images/Flip.png"));
    QAction * flipImageAct = new QAction(flipImageIcon, tr("&镜像"), this);
    flipImageAct->setShortcuts(QKeySequence::New);
    //connect(flipImageAct, &QAction::triggered, this, &ImageViewer::on_flipImage_clicked);
    fileToolBar->addAction(flipImageAct);

    fileToolBar->addSeparator();

    const QIcon extendImageIcon = QIcon::fromTheme("document-new", QIcon(":/images/Extend.png"));
    QAction * extendImageAct = new QAction(extendImageIcon, tr("&原始尺寸"), this);
    extendImageAct->setShortcuts(QKeySequence::New);
    //connect(extendImageAct, &QAction::triggered, this, &ImageViewer::on_extendImage_clicked);
    fileToolBar->addAction(extendImageAct);

    const QIcon zoomInImageIcon = QIcon::fromTheme("document-new", QIcon(":/images/ZoomIn.png"));
    QAction * zoomInImageAct = new QAction(zoomInImageIcon, tr("&放大"), this);
    zoomInImageAct->setShortcuts(QKeySequence::New);
    //connect(zoomInImageAct, &QAction::triggered, this, &ImageViewer::on_zoomInImage_clicked);
    fileToolBar->addAction(zoomInImageAct);

    const QIcon zoomOutImageIcon = QIcon::fromTheme("document-new", QIcon(":/images/ZoomOut.png"));
    QAction * zoomOutImageAct = new QAction(zoomOutImageIcon, tr("&缩小"), this);
    zoomOutImageAct->setShortcuts(QKeySequence::New);
    //connect(zoomOutImageAct, &QAction::triggered, this, &ImageViewer::on_zoomOutImage_clicked);
    fileToolBar->addAction(zoomOutImageAct);
}

void ImageViewer::initStatusBar(){
    fileIndexLabel = new QLabel();
    filePathLabel = new QLabel();
    fileSizeLabel = new QLabel();
    imageScaleLabel = new QLabel();
    imageSizeLabel = new QLabel();
    fileModDateLabel = new QLabel();

    statusBar()->addWidget(fileIndexLabel,0);
    statusBar()->addWidget(filePathLabel,1);
    statusBar()->addWidget(fileSizeLabel,0);
    statusBar()->addWidget(imageSizeLabel,0);
    statusBar()->addWidget(imageSizeLabel,0);
    statusBar()->addWidget(fileModDateLabel,0);
}


void ImageViewer::displayImage(QString absoluteFilePath) {

    const ImageCore::ReadData& readData = this->imageCore->readFile(absoluteFilePath, true, QSize(this->width() - 2, this->height() - 2));
    imgArea->setPixmap(readData.pixmap);
    imgArea->adjustSize();
}

/**************************************
 * 图片处理函数
 *************************************/
void ImageViewer::zoomInImage(){
    if(scaleVar <= 1.75){
        scaleVar = scaleVar+0.25;
    }else{
        scaleVar = 2;
    }
}

void ImageViewer::zoomOutImage(){
    if(scaleVar >= 0.35){
        scaleVar = scaleVar-0.25;
    }else{
        scaleVar = 0.1;
    }
}
void ImageViewer::extendImage(){
    scaleVar = 1;
}

/**************************************
 * 按钮事件
**************************************/
void ImageViewer::keyPressEvent(QKeyEvent *event){
    if(event->modifiers() == Qt::NoModifier){
        if(event->key() == Qt::Key_Left){
            on_readPrevImage_clicked();
        }else if(event->key() == Qt::Key_Right){
            on_readNextImage_clicked();
        }
    }
}

void ImageViewer::showEvent(QShowEvent* event)
{
    displayImage(imageSwitcher->getImage().absoluteFilePath());
}

//void ImageViewer::on_selectHomeDir_clicked(){
//    QString selectDir;
//    selectDir = QFileDialog::getExistingDirectory(this,
//                                                  "选择图片目录",
//                                                  "",
//                                                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
//    if( selectDir != "" ) {
//        readDir(selectDir);
//    }
//}

//void ImageViewer::on_selectImageFile_clicked(){
//    QString selectFile;
//    QFileInfo fi;
//    selectFile = QFileDialog::getOpenFileName(this,
//                                              "选择图片",
//                                              "*.jpg;*.jpeg;*.bmp;*.png");
//    if(selectFile != ""){
//        readFile(selectFile);
//    }
//}

void ImageViewer::on_readPrevImage_clicked(){
    displayImage(imageSwitcher->previous().absoluteFilePath());
}

void ImageViewer::on_readNextImage_clicked(){
    displayImage(imageSwitcher->next().absoluteFilePath());
}

//void ImageViewer::on_refreshImage_clicked(){
//    readFile(fileInfo.absoluteFilePath());
//}
//void ImageViewer::on_rotateImage_clicked(){
//    loadImage("rotate");
//}
//void ImageViewer::on_flipImage_clicked(){
//    loadImage("flip");
//}
//void ImageViewer::on_zoomInImage_clicked(){
//    if(scaleVar < 2){
//        loadImage("zoomIn");
//    }
//}
//void ImageViewer::on_zoomOutImage_clicked(){
//    if(scaleVar > 0.1){
//        loadImage("zoomOut");
//    }
//}
//void ImageViewer::on_extendImage_clicked(){
//    if(scaleVar != 1){
//        loadImage("extend");
//    }
//}
