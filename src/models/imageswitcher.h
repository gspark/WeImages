#ifndef IMAGESWITCHER_H
#define IMAGESWITCHER_H

#include "..\filelistmodel\filefilterproxymodel.h"
#include <QFileInfo>

class ImageSwitcher
{
public:
    ImageSwitcher(const QModelIndex& current, const FileFilterProxyModel* model);
    QFileInfo getImage();

private:
    //QStandardItem *m_image;
    const FileFilterProxyModel* _model;

    QModelIndex _image;

public:
    int count();
    int currIndex();
public slots:
    QFileInfo previous();
    QFileInfo next();
};

#endif // IMAGESWITCHER_H
