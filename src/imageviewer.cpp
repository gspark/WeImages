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
    : QMainWindow(parent), _imageCore(imageCore), _imageSwitcher(imageSwitcher), _originImage(nullptr), _scale(1)
{
    this->setAttribute(Qt::WA_DeleteOnClose);

    initUI();

    QTimer::singleShot(1, this, &ImageViewer::on_delayLoadFile);
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

    QAction *prevImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 61751, 16, 16, 16)), tr("&Previous"), this);
    prevImageAct->setShortcuts(QKeySequence::New);
    connect(prevImageAct, &QAction::triggered, this, &ImageViewer::on_readPrevImage_clicked);
    fileToolBar->addAction(prevImageAct);

    //const QIcon nextImageIcon = QIcon::fromTheme("document-new", QIcon(":/images/Next.png"));
    QAction *nextImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 61752, 16, 16, 16)), tr("&Next"), this);
    nextImageAct->setShortcuts(QKeySequence::New);
    connect(nextImageAct, &QAction::triggered, this, &ImageViewer::on_readNextImage_clicked);
    fileToolBar->addAction(nextImageAct);

    fileToolBar->addSeparator();

    QAction* refreshImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62193, 16, 16, 16)), tr("&Refresh"), this);
    refreshImageAct->setShortcuts(QKeySequence::New);
    connect(refreshImageAct, &QAction::triggered, this, &ImageViewer::on_refreshImage_clicked);
    fileToolBar->addAction(refreshImageAct);

    fileToolBar->addSeparator();

    QAction* rotateImageActL = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62186, 16, 16, 16)), tr("Rotate &Left"), this);
    rotateImageActL->setShortcuts(QKeySequence::New);
    connect(rotateImageActL, &QAction::triggered, this, &ImageViewer::on_rotateImage_l_clicked);
    fileToolBar->addAction(rotateImageActL);

    QAction* rotateImageActR = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62201, 16, 16, 16)), tr("Rotate Right"), this);
    rotateImageActR->setShortcuts(QKeySequence::New);
    connect(rotateImageActR, &QAction::triggered, this, &ImageViewer::on_rotateImage_r_clicked);
    fileToolBar->addAction(rotateImageActR);

    QAction* flipImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62201, 16, 16, 16)), tr("Rotate 180 Degrees"), this);
    flipImageAct->setShortcuts(QKeySequence::New);
    connect(flipImageAct, &QAction::triggered, this, &ImageViewer::on_flipImage_clicked);
    fileToolBar->addAction(flipImageAct);

    fileToolBar->addSeparator();

    QAction* zoomInImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 61454, 16, 16, 16)), tr("Zoom &In"), this);
    zoomInImageAct->setShortcuts(QKeySequence::New);
    connect(zoomInImageAct, &QAction::triggered, this, &ImageViewer::on_zoomInImage_clicked);
    fileToolBar->addAction(zoomInImageAct);

    QAction* zoomOutImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 61456, 16, 16, 16)), tr("Zoom &Out"), this);
    zoomOutImageAct->setShortcuts(QKeySequence::New);
    connect(zoomOutImageAct, &QAction::triggered, this, &ImageViewer::on_zoomOutImage_clicked);
    fileToolBar->addAction(zoomOutImageAct);

    QAction* extendImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62238, 16, 16, 16)), tr("Don't Fit"), this);
    extendImageAct->setShortcuts(QKeySequence::New);
    connect(extendImageAct, &QAction::triggered, this, &ImageViewer::on_extendImage_clicked);
    fileToolBar->addAction(extendImageAct);

    fileToolBar->addSeparator();

    _exportImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62830, 16, 16, 16)), tr("&export"), this);
    connect(_exportImageAct, &QAction::triggered, this, &ImageViewer::on_exportImage_clicked);
    fileToolBar->addAction(_exportImageAct);
    _exportImageAct->setEnabled(false);

    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    fileToolBar->addWidget(spacer);

    QAction* closeAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62197, 16, 16, 16)), tr("&close"), this);
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

void ImageViewer::on_delayLoadFile()
{
    loadFile(_imageSwitcher->getImage().absoluteFilePath());
}

void ImageViewer::loadFile(const QString& absoluteFilePath)
{
    const ImageReadData* readData = this->_imageCore->readFile(absoluteFilePath, QSize());
    if (readData->pixmap.isNull())
    {
        return;
    }

    fileIndexLabel->setText(QString::number(this->_imageSwitcher->currIndex() + 1) + "/" + QString::number(this->_imageSwitcher->count()));
    filePathLabel->setText(readData->fileInfo.absoluteFilePath());
    fileModDateLabel->setText(readData->fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    fileSizeLabel->setText(fileSizeToString(readData->fileInfo.size()));

    _exportImageAct->setEnabled(this->_imageCore->isWeChatImage(readData->fileInfo));

    this->_originImage = const_cast<ImageReadData*>(readData);

    loadImage(ImageLoadType::normal);
}

void ImageViewer::loadImage(ImageLoadType loadType)
{
    if (nullptr != this->_originImage && !this->_originImage->pixmap.isNull()) {

        imageSizeLabel->setText(QString::number(_originImage->pixmap.width())
            + "x"
            + QString::number(_originImage->pixmap.height()));
        switch (loadType)
        {
        case normal:
            computeScaleWithView(_originImage->pixmap);
            displayImage(resizeImage());
            break;
        case flip:
            flipImage(_originImage->pixmap);
            computeScaleWithView(_originImage->pixmap);
            displayImage(resizeImage());
            break;
        case rotateL:
            rotateImage(_originImage->pixmap, false);
            computeScaleWithView(_originImage->pixmap);
            displayImage(resizeImage());
            break;
        case rotateR:
            rotateImage(_originImage->pixmap);
            computeScaleWithView(_originImage->pixmap);
            displayImage(resizeImage());
            break;
        case zoomIn:
            zoomInImage();
            displayImage(resizeImage());
            break;
        case zoomOut:
            zoomOutImage();
            displayImage(resizeImage());
            break;
        case extend:
            extendImage();
            displayImage(resizeImage());
            break;
        default:
            break;
        }
        imageScaleLabel->setText(QString::number(((float)((int)((_scale + 0.005) * 100)))) + " %");
    }
}

QPixmap ImageViewer::resizeImage()
{
    if (nullptr == _originImage || _originImage->pixmap.isNull())
    {
        return QPixmap();
    }

    if (_scale == 1)
    {
        return _originImage->pixmap;
    }
    int width = floor(_originImage->pixmap.width() * _scale);
    int height = floor(_originImage->pixmap.height() * _scale);
    return this->_imageCore->scaled(_originImage->pixmap, QSize(width, height));
}

void ImageViewer::displayImage(const QPixmap pixmap) {
    imgArea->setPixmap(pixmap);
    imgArea->adjustSize();

    //double scaleVar = computeScaleWithView(pixmap);
    //imageScaleLabel->setText(QString::number(((float)((int)((scaleVar + 0.005) * 100)))) + " %");
    imageScaleLabel->setText(QString::number(((float)((int)((_scale + 0.005) * 100)))) + " %");
}

double ImageViewer::computeScaleWithView(const QPixmap pixmap) {
   
    double labelW = scrollArea->size().width() - 2;
    double labelH = scrollArea->size().height() - 2;
    if (pixmap.width() > labelW || pixmap.height() > labelH) {
        double imgW = pixmap.width();
        double imgH = pixmap.height();

        if (imgW > labelW && imgH < labelH) {
            _scale = imgW / labelW;
        } else if (imgW < labelW && imgH > labelH) {
            _scale = imgH / labelH;
        } else {
            double scaleVarW = imgW / labelW;
            double scaleVarH = imgH / labelH;
            if (scaleVarW > scaleVarH) {
                _scale = scaleVarW;
            }
            else {
                _scale = scaleVarH;
            }
        }
        _scale = 1 / _scale;
    }
    return _scale;
}

QPixmap ImageViewer::flipImage(const QPixmap originPixmap) {
    if (originPixmap.isNull())
    {
        return QPixmap();
    }
    QTransform transform;
    transform.rotate(180, Qt::XAxis);
    
    this->_originImage->pixmap = originPixmap.transformed(transform, Qt::TransformationMode::SmoothTransformation);
 
    return this->_originImage->pixmap;
}

QPixmap ImageViewer::rotateImage(const QPixmap& originPixmap, bool right) {
    if (originPixmap.isNull())
    {
        return QPixmap();
    }

    float  width = originPixmap.width(), height = originPixmap.height();
    const QPixmap tmp = originPixmap.transformed(QTransform()
        .translate(-width / 2, -height / 2)
        .rotate(right ? 90.0 : -90.0)
        .translate(width / 2, height / 2), Qt::TransformationMode::SmoothTransformation);
    if (!tmp.isNull())
    {
        this->_originImage->pixmap = tmp;
    }
    return this->_originImage->pixmap;
}


void ImageViewer::zoomInImage() {
    if (_scale <= 1.75) {
        _scale = _scale + 0.25;
    }
    else {
        _scale = 2;
    }
}

void ImageViewer::zoomOutImage() {
    if (_scale >= 0.35) {
        _scale = _scale - 0.25;
    }
    else {
        _scale = 0.1;
    }
}
void ImageViewer::extendImage() {
    _scale = 1;
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

void ImageViewer::on_readPrevImage_clicked(){
    _scale = 1;
    loadFile(_imageSwitcher->previous().absoluteFilePath());
}

void ImageViewer::on_readNextImage_clicked(){
    _scale = 1;
    loadFile(_imageSwitcher->next().absoluteFilePath());
}

void ImageViewer::on_refreshImage_clicked()
{
    _scale = 1;
    loadFile(_imageSwitcher->getImage().absoluteFilePath());
}

void ImageViewer::on_rotateImage_r_clicked()
{
    loadImage(ImageLoadType::rotateR);
}

void ImageViewer::on_rotateImage_l_clicked()
{
    loadImage(ImageLoadType::rotateL);
}

void ImageViewer::on_flipImage_clicked()
{
    loadImage(ImageLoadType::flip);
}

void ImageViewer::on_exportImage_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this,
        tr("open directory"),
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(directory != "") {
        QFileInfo fileInfo = _imageSwitcher->getImage();
        QString file;
        ImageReadData* readData = _originImage;
        if (nullptr == readData)
        {
            readData = this->_imageCore->readFile(fileInfo.absoluteFilePath(), QSize());
            file = directory + QDir::separator() + fileInfo.baseName() + "." + readData->suffix;
        }
        readData->pixmap.save(file);
    }
}

void ImageViewer::on_extendImage_clicked()
{
    if (_scale != 1) {
        loadImage(ImageLoadType::extend);
    }
}

void ImageViewer::on_zoomInImage_clicked()
{
    if (_scale < 2) {
        loadImage(ImageLoadType::zoomIn);
    }
}

void ImageViewer::on_zoomOutImage_clicked()
{
    if (_scale > 0.1) {
        loadImage(ImageLoadType::zoomOut);
    }
}

