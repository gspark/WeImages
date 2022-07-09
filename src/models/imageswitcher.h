#ifndef IMAGESWITCHER_H
#define IMAGESWITCHER_H


#include <QStandardItem>
#include <QStandardItemModel>
#include <QFileInfo>

class ImageSwitcher
{
public:
    ImageSwitcher(QStandardItem *image, QStandardItemModel *model);
    QFileInfo getImage();

private:
    QStandardItem *m_image;
    QStandardItemModel *m_model;

public slots:
    QFileInfo imagePrecedente();
    QFileInfo imageSuivante();
};

#endif // IMAGESWITCHER_H
