#ifndef IMAGESWITCHER_H
#define IMAGESWITCHER_H


#include <QStandardItem>
#include <QStandardItemModel>
#include <QtDebug>
#include "../delegate/thumbnailData.h"

class ImageSwitcher
{
public:
    ImageSwitcher(QStandardItem *image, QStandardItemModel *model);
    ThumbnailData getImage();

private:
    QStandardItem *m_image;
    QStandardItemModel *m_model;

public slots:
    ThumbnailData imagePrecedente();
    ThumbnailData imageSuivante();
};

#endif // IMAGESWITCHER_H
