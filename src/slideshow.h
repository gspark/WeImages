#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include <QMainWindow>
#include "models/imageswitcher.h"


class ImageCore;

namespace Ui {
    class Slideshow;
}

class Slideshow : public QMainWindow
{
    Q_OBJECT

public:
    explicit Slideshow(ImageCore* imageCore, ImageSwitcher* imageSwitcher, QWidget* parent = nullptr);
    ~Slideshow();

private slots:
    void nextImage();
    void prevImage();
    void quit();

private:
    Ui::Slideshow* ui;
    ImageSwitcher* imageSwitcher;
    ImageCore* imageCore;
};

#endif // SLIDESHOW_H
