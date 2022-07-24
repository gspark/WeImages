#include "imageviewer.h"
#include "imagecore.h"
#include "models/imageswitcher.h"
#include "iconhelper.h"
#include "filesystemhelperfunctions.h"

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

ImageViewer::ImageViewer(ImageCore* imageCore, ImageSwitcher* imageSwitcher, QWidget* parent)
    : QMainWindow(parent), _imageCore(imageCore), _imageSwitcher(imageSwitcher)
{
    initUI();
}

ImageViewer::~ImageViewer()
{
    delete _imageSwitcher;
    this->deleteLater();
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

    setCentralWidget(scrollArea);
    //禁止工具栏右键菜单
    setContextMenuPolicy(Qt::NoContextMenu);
    setWindowTitle(tr("viewer"));
    resize(1200, 800);
}

void ImageViewer::initToolBar(){
    QToolBar *fileToolBar = addToolBar(tr("toolbar"));
    //QToolBar* fileToolBar = new QToolBar;
    //禁止工具栏拖动
    fileToolBar->setMovable(false);

    QWidget* spacerL = new QWidget(this);
    spacerL->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    fileToolBar->addWidget(spacerL);

    IconHelper::StyleColor styleColor;

    QAction *prevImageAct = new QAction(QIcon(IconHelper::getPixmap(styleColor.normalBgColor, 61751, 16, 16, 16)), tr("&Previous"), this);
    prevImageAct->setShortcuts(QKeySequence::New);
    connect(prevImageAct, &QAction::triggered, this, &ImageViewer::on_readPrevImage_clicked);
    fileToolBar->addAction(prevImageAct);

    //const QIcon nextImageIcon = QIcon::fromTheme("document-new", QIcon(":/images/Next.png"));
    QAction *nextImageAct = new QAction(QIcon(IconHelper::getPixmap(styleColor.normalBgColor, 61752, 16, 16, 16)), tr("&Next"), this);
    nextImageAct->setShortcuts(QKeySequence::New);
    connect(nextImageAct, &QAction::triggered, this, &ImageViewer::on_readNextImage_clicked);
    fileToolBar->addAction(nextImageAct);

    fileToolBar->addSeparator();

    _exportImageAct = new QAction(QIcon(IconHelper::getPixmap(styleColor.normalBgColor, 62830, 16, 16, 16)), tr("&export"), this);
    connect(_exportImageAct, &QAction::triggered, this, &ImageViewer::on_exportImage_clicked);
    fileToolBar->addAction(_exportImageAct);
    _exportImageAct->setEnabled(false);

    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    fileToolBar->addWidget(spacer);

    QAction* closeAct = new QAction(QIcon(IconHelper::getPixmap(styleColor.normalBgColor, 61527, 16, 16, 16)), tr("&close"), this);
    closeAct->setShortcuts(QKeySequence::New);
    connect(closeAct, &QAction::triggered, this, &ImageViewer::close);
    fileToolBar->addAction(closeAct);
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
    statusBar()->addWidget(imageScaleLabel,0);
    statusBar()->addWidget(imageSizeLabel,0);
    statusBar()->addWidget(fileModDateLabel,0);
}

void ImageViewer::displayImage(QString absoluteFilePath) {
    const ImageReadData& readData = this->_imageCore->readFile(absoluteFilePath, true, QSize());

    fileIndexLabel->setText(QString::number(this->_imageSwitcher->currIndex() + 1) + "/" + QString::number(this->_imageSwitcher->count()));
    filePathLabel->setText(readData.fileInfo.absoluteFilePath());
    fileModDateLabel->setText(readData.fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    fileSizeLabel->setText(fileSizeToString(readData.fileInfo.size()));

    imageSizeLabel->setText(QString::number(readData.pixmap.width())
        + "x"
        + QString::number(readData.pixmap.height()));
    imgArea->setPixmap(readData.pixmap);
    imgArea->adjustSize();

    double scaleVar = computeScaleWithView(readData.pixmap);
    imageScaleLabel->setText(QString::number(((float)((int)((scaleVar + 0.005) * 100)))) + " %");

    _exportImageAct->setEnabled(this->_imageCore->isWeChatImage(readData.fileInfo));
}

double ImageViewer::computeScaleWithView(const QPixmap &pixmap) {
    double scaleVar = 1;
    double labelW = scrollArea->size().width() - 2;
    double labelH = scrollArea->size().height() - 2;
    if (pixmap.width() > labelW || pixmap.height() > labelH) {
        double imgW = pixmap.width();
        double imgH = pixmap.height();

        if (imgW > labelW && imgH < labelH) {
            scaleVar = imgW / labelW;
        }
        else if (imgW < labelW && imgH > labelH) {
            scaleVar = imgH / labelH;
        }
        else {
            double scaleVarW = imgW / labelW;
            double scaleVarH = imgH / labelH;
            if (scaleVarW > scaleVarH) {
                scaleVar = scaleVarW;
            }
            else {
                scaleVar = scaleVarH;
            }
        }
        scaleVar = 1 / scaleVar;
    }
    return scaleVar;
}

/**************************************
 * 按钮事件
**************************************/
void ImageViewer::keyPressEvent(QKeyEvent* event) {
    if (event->modifiers() == Qt::NoModifier) {
        if (event->key() == Qt::Key_Left) {
            on_readPrevImage_clicked();
        }
        else if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Space) {
            on_readNextImage_clicked();
        }
    }
}

void ImageViewer::showEvent(QShowEvent* event)
{
    displayImage(_imageSwitcher->getImage().absoluteFilePath());
}

void ImageViewer::on_readPrevImage_clicked(){
    displayImage(_imageSwitcher->previous().absoluteFilePath());
}

void ImageViewer::on_readNextImage_clicked(){
    displayImage(_imageSwitcher->next().absoluteFilePath());
}

void ImageViewer::on_exportImage_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this,
        tr("open directory"),
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(directory != "" ) {
        QFileInfo fileInfo = _imageSwitcher->getImage();
        const ImageReadData& readData = this->_imageCore->readFile(fileInfo.absoluteFilePath(), true, QSize());
        QString file = directory + QDir::separator() + fileInfo.baseName() + "." + readData.suffix;
        readData.pixmap.save(file);
    }
}

