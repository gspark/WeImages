#include "imageswitcher.h"
#include "../delegate/thumbnailData.h"
#include "../logger/Logger.h"

ImageSwitcher::ImageSwitcher(const QModelIndex& current, const FileFilterProxyModel* model)
{
    m_image = current;
    m_model = model;
}

QFileInfo ImageSwitcher::getImage()
{
    return m_model->fileInfo(m_image);
}


int ImageSwitcher::count()
{
    return m_model->rowCount();
}

int ImageSwitcher::currIndex()
{
    return m_image.row();
}

QFileInfo ImageSwitcher::previous() {
    if (m_image.row() <= 0) {
        m_image = m_model->index(m_model->rowCount() - 1, 0);
    }
    else {
        m_image = m_model->index(m_image.row() - 1, 0);
    }
    return getImage();
}

QFileInfo ImageSwitcher::next() {
    if (m_image.row() >= m_model->rowCount() - 1) {
        m_image = m_model->index(0, 0);
    }
    else {
        m_image = m_model->index(m_image.row() + 1, 0);
    }
    return getImage();
}
