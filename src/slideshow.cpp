#include "slideshow.h"
#include "ui_slideshow.h"

#include "imagecore.h"

#include <QPixmap>
#include <QApplication>
#include <QScreen>

Slideshow::Slideshow(ImageCore* imageCore, ImageSwitcher* imageSwitcher, QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::Slideshow)
{
    ui->setupUi(this);
    this->imageSwitcher = imageSwitcher;
    this->imageCore = imageCore;

    QRect rec = QGuiApplication::screens().first()->geometry();

    ui->label->setMaximumWidth(rec.width() - 100);
    ui->label->setMaximumHeight(rec.height() - 100);

    //ui->label->setPixmap(imageSwitcher->getImage().fullAbsolutePath);
    const ImageCore::ReadData& readData = this->imageCore->readFile(imageSwitcher->getImage().absoluteFilePath(), true, QSize());
    ui->label->setPixmap(readData.pixmap);

    ui->toolButtonNext->setDefaultAction(ui->actionImage_suivante);
    ui->toolButtonPrev->setDefaultAction(ui->actionImage_pr_c_dente);
    ui->toolButtonQuit->setDefaultAction(ui->actionQuitter);

    connect(ui->actionImage_suivante, SIGNAL(triggered()), this, SLOT(nextImage()));
    connect(ui->actionImage_pr_c_dente, SIGNAL(triggered()), this, SLOT(prevImage()));
    connect(ui->actionQuitter, SIGNAL(triggered()), this, SLOT(quit()));

    //setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    //setWindowState(Qt::WindowFullScreen);
    //showFullScreen();
}

Slideshow::~Slideshow()
{
    delete this->imageSwitcher;
    delete ui;
}

void Slideshow::nextImage() {

    const ImageCore::ReadData& readData = this->imageCore->readFile(imageSwitcher->next().absoluteFilePath(), true, QSize(this->width() - 2, this->height() - 2));
    ui->label->setPixmap(readData.pixmap);
}

void Slideshow::prevImage() {
    const ImageCore::ReadData& readData = this->imageCore->readFile(imageSwitcher->previous().absoluteFilePath(), true, QSize(this->width() - 2, this->height() - 2));
    ui->label->setPixmap(readData.pixmap);
}

void Slideshow::quit() {
    this->close();
}
