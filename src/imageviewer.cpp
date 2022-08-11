#include "imageviewer.h"
#include "imagecore.h"
#include "models/imageswitcher.h"
#include "iconhelper.h"
#include "filesystemhelperfunctions.h"
#include "component\shscreen.h"

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
    : WxWindow(parent), _imageCore(imageCore), _imageSwitcher(imageSwitcher), _originImage(nullptr), _scale(1.0)
    , _flip(nullptr), _rotate(nullptr)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    // 主窗体关闭也关闭此窗体
    this->setAttribute(Qt::WA_QuitOnClose, false);

    initUI();

    QTimer::singleShot(1, this, &ImageViewer::on_delayLoadFile);
}

ImageViewer::~ImageViewer()
{
    delete _imageSwitcher;
    if (nullptr != _flip)
    {
        delete _flip;
    }
    if (nullptr != _rotate)
    {
        delete _rotate;
    }
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
    QRect geom = ShScreen::normalRect();
    resize(geom.width(), geom.height());
}

void ImageViewer::initToolBar(){
    QToolBar *fileToolBar = addToolBar(tr("toolbar"));
    fileToolBar->setStyleSheet("QToolBar {border-bottom: none; border-top: none;}");
    
    //QToolBar* fileToolBar = new QToolBar;
    //禁止工具栏拖动
    fileToolBar->setMovable(false);

    QWidget* spacerL = new QWidget(this);
    spacerL->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    fileToolBar->addWidget(spacerL);

    IconHelper::StyleColor styleColor;

    QAction *prevImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 61751, 12, 16, 16)), tr("&Previous"), this);
    prevImageAct->setShortcuts(QKeySequence::New);
    connect(prevImageAct, &QAction::triggered, this, &ImageViewer::on_readPrevImage_clicked);
    fileToolBar->addAction(prevImageAct);

    //const QIcon nextImageIcon = QIcon::fromTheme("document-new", QIcon(":/images/Next.png"));
    QAction *nextImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 61752, 12, 16, 16)), tr("&Next"), this);
    nextImageAct->setShortcuts(QKeySequence::New);
    connect(nextImageAct, &QAction::triggered, this, &ImageViewer::on_readNextImage_clicked);
    fileToolBar->addAction(nextImageAct);

    fileToolBar->addSeparator();

    QAction* refreshImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62193, 12, 16, 16)), tr("&Refresh"), this);
    refreshImageAct->setShortcuts(QKeySequence::New);
    connect(refreshImageAct, &QAction::triggered, this, &ImageViewer::on_refreshImage_clicked);
    fileToolBar->addAction(refreshImageAct);

    fileToolBar->addSeparator();

    QAction* rotateImageActL = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62186, 12, 16, 16)), tr("Rotate &Left"), this);
    rotateImageActL->setShortcuts(QKeySequence::New);
    connect(rotateImageActL, &QAction::triggered, this, &ImageViewer::on_rotateImage_l_clicked);
    fileToolBar->addAction(rotateImageActL);

    QAction* rotateImageActR = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62201, 12, 16, 16)), tr("Rotate Right"), this);
    rotateImageActR->setShortcuts(QKeySequence::New);
    connect(rotateImageActR, &QAction::triggered, this, &ImageViewer::on_rotateImage_r_clicked);
    fileToolBar->addAction(rotateImageActR);

    QAction* vFlipImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62450, 12, 16, 16)), tr("Vertical Flip"), this);
    vFlipImageAct->setShortcuts(QKeySequence::New);
    connect(vFlipImageAct, &QAction::triggered, this, &ImageViewer::on_vflipImage_clicked);
    fileToolBar->addAction(vFlipImageAct);

    QAction* hFlipImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62448, 12, 16, 16)), tr("Horizontal Flip"), this);
    hFlipImageAct->setShortcuts(QKeySequence::New);
    connect(hFlipImageAct, &QAction::triggered, this, &ImageViewer::on_hflipImage_clicked);
    fileToolBar->addAction(hFlipImageAct);

    fileToolBar->addSeparator();

    QAction* zoomInImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 61454, 12, 16, 16)), tr("Zoom &in"), this);
    zoomInImageAct->setShortcuts(QKeySequence::New);
    connect(zoomInImageAct, &QAction::triggered, this, &ImageViewer::on_zoomInImage_clicked);
    fileToolBar->addAction(zoomInImageAct);

    QAction* zoomOutImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 61456, 12, 16, 16)), tr("Zoom &out"), this);
    zoomOutImageAct->setShortcuts(QKeySequence::New);
    connect(zoomOutImageAct, &QAction::triggered, this, &ImageViewer::on_zoomOutImage_clicked);
    fileToolBar->addAction(zoomOutImageAct);

    QAction* extendImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62238, 12, 16, 16)), tr("Don't Fit"), this);
    extendImageAct->setShortcuts(QKeySequence::New);
    connect(extendImageAct, &QAction::triggered, this, &ImageViewer::on_extendImage_clicked);
    fileToolBar->addAction(extendImageAct);

    fileToolBar->addSeparator();

    _exportImageAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62830, 12, 16, 16)), tr("&Export"), this);
    connect(_exportImageAct, &QAction::triggered, this, &ImageViewer::on_exportImage_clicked);
    fileToolBar->addAction(_exportImageAct);
    _exportImageAct->setEnabled(false);

    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    fileToolBar->addWidget(spacer);

    //QAction* closeAct = new QAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62197, 12, 16, 16)), tr("&close"), this);
    //closeAct->setShortcuts(QKeySequence::New);
    //connect(closeAct, &QAction::triggered, this, &ImageViewer::close);
    //fileToolBar->addAction(closeAct);
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
    this->_currentPixmap = this->_originImage->pixmap;

    loadImage(ImageLoadType::normal);
}

void ImageViewer::loadImage(ImageLoadType loadType)
{
    if (this->_currentPixmap.isNull()) {
        return;
    }
    QPixmap pix = _currentPixmap;
    switch (loadType)
    {
    case normal:
        computeScaleWithView(pix);
        break;
    case flip:
        pix = flipImage(_currentPixmap);
        computeScaleWithView(pix);
        _currentPixmap = pix;
        break;
    case rotate:
        pix = rotateImage(_currentPixmap);
        computeScaleWithView(pix);
        _currentPixmap = pix;
        break;
    case zoomIn:
    case zoomOut:
    case extend:
        break;
    default:
        return;
    }
    displayImage(resizeImage(pix));
    imageSizeLabel->setText(QString::number(pix.width()) + "x" + QString::number(pix.height()));
    imageScaleLabel->setText(QString::number(((float)((int)((_scale + 0.005) * 100)))) + " %");
}

QPixmap ImageViewer::resizeImage(const QPixmap& pixmap)
{
    if (pixmap.isNull())
    {
        return QPixmap();
    }

    if (_scale == 1)
    {
        return pixmap;
    }
    int width = floor(pixmap.width() * _scale);
    int height = floor(pixmap.height() * _scale);
    return this->_imageCore->scaled(pixmap, QSize(width, height));
}

void ImageViewer::displayImage(const QPixmap pixmap) {
    imgArea->setPixmap(pixmap);
    imgArea->adjustSize();
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
    if (nullptr != _flip)
    {
        const QPixmap pix = this->_imageCore->flipImage(originPixmap, _flip->horizontal, _flip->horizontal ? _flip->dirH : _flip->dirV);
        if (!pix.isNull())
        {
            return pix;
        }
    }
    return originPixmap;
}

QPixmap ImageViewer::rotateImage(const QPixmap& originPixmap) {
    if (originPixmap.isNull())
    {
        return QPixmap();
    }

    if (nullptr != _rotate)
    {
        const QPixmap pix = this->_imageCore->rotateImage(originPixmap, _rotate->right, _rotate->right ? _rotate->dirR : _rotate->dirL);
        if (!pix.isNull())
        {
            return pix;
        }
    }
    return originPixmap;
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
    initImageParam();
    loadFile(_imageSwitcher->previous().absoluteFilePath());
}

void ImageViewer::on_readNextImage_clicked(){
    initImageParam();
    loadFile(_imageSwitcher->next().absoluteFilePath());
}

void ImageViewer::on_refreshImage_clicked()
{
    initImageParam();
    loadFile(_imageSwitcher->getImage().absoluteFilePath());
}

void ImageViewer::initImageParam()
{
    _scale = 1.0;
    if (nullptr != _flip)
    {
        delete _flip;
        _flip = nullptr;
    }
    if (nullptr != _rotate)
    {
        delete _rotate;
        _rotate = nullptr;
    }
}

void ImageViewer::on_rotateImage_r_clicked()
{
    if (nullptr == _rotate)
    {
        initRotate();
    }
    _rotate->right = true;
    loadImage(ImageLoadType::rotate);
}

void ImageViewer::on_rotateImage_l_clicked()
{
    if (nullptr == _rotate)
    {
        initRotate();
    }
    _rotate->right = false;
    loadImage(ImageLoadType::rotate);
}

void ImageViewer::on_vflipImage_clicked()
{
    if (nullptr == _flip)
    {
        initFlip();
    }
    _flip->horizontal = false;
    loadImage(ImageLoadType::flip);
}

void ImageViewer::on_hflipImage_clicked()
{
    if (nullptr == _flip)
    {
        initFlip();
    }
    _flip->horizontal = true;
    loadImage(ImageLoadType::flip);
}

void ImageViewer::initRotate()
{
    _rotate = new Rotate;
    _rotate->dirL = 1;
    _rotate->dirR = 1;
}

void ImageViewer::initFlip()
{
    _flip = new Flip;
    _flip->dirH = 1;
    _flip->dirV = 1;
}

void ImageViewer::on_zoomInImage_clicked()
{
    if (_scale < 4) {
        if (_scale <= 3.75) {
            _scale = _scale + 0.25;
        }
        else {
            _scale = 4;
        }
        loadImage(ImageLoadType::zoomIn);
    }
}

void ImageViewer::on_zoomOutImage_clicked()
{
    if (_scale > 0.1) {
        if (_scale >= 0.35) {
            _scale = _scale - 0.25;
        }
        else {
            _scale = 0.1;
        }
        loadImage(ImageLoadType::zoomOut);
    }
}

void ImageViewer::on_extendImage_clicked()
{
    const double epslion = 1e-8;
    if (abs(_scale - 1.0) > epslion) {
        _scale = 1.0;
        loadImage(ImageLoadType::extend);
    }
}

void ImageViewer::on_exportImage_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(nullptr,
        tr("open directory"),
        "",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks/* | QFileDialog::DontUseNativeDialog*/);
    if (directory != "") {
        this->_imageCore->exportWeChatImage(_imageSwitcher->getImage(), directory);
    }
}
