#include "imageswitcher.h"
#include "../delegate/thumbnailData.h"

ImageSwitcher::ImageSwitcher(QStandardItem* image, QStandardItemModel* model)
{
    m_image = image;
    m_model = model;
}

QFileInfo ImageSwitcher::getImage() {
    QVariant variant = m_image->data(Qt::UserRole + 3);
    if (variant.canConvert<ThumbnailData>())
    {
        auto thumbnailData = variant.value<ThumbnailData>();
        return thumbnailData.fileInfo;
    }
    return variant.value<QFileInfo>();
}


QFileInfo ImageSwitcher::imagePrecedente() {
    if (m_image->row() <= 0) m_image = m_model->item(m_model->rowCount() - 1);
    else m_image = m_model->item(m_image->row() - 1);
    return getImage();
}

QFileInfo ImageSwitcher::imageSuivante() {
    if (m_image->row() >= m_model->rowCount() - 1) m_image = m_model->item(0);
    else m_image = m_model->item(m_image->row() + 1);
    return getImage();
}
