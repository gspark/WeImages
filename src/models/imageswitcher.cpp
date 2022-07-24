#include "imageswitcher.h"
#include "../delegate/thumbnailData.h"

ImageSwitcher::ImageSwitcher(const QModelIndex& current, const FileFilterProxyModel* model)
    :_image(current), _model(model)
{
}

QFileInfo ImageSwitcher::getImage()
{
    return _model->fileInfo(_image);
}

int ImageSwitcher::count()
{
    return _model->rowCount();
}

int ImageSwitcher::currIndex()
{
    return _image.row();
}

QFileInfo ImageSwitcher::previous() {
    if (_image.row() <= 0) {
        _image = _model->index(_model->rowCount() - 1, 0);
    }
    else {
        _image = _model->index(_image.row() - 1, 0);
    }
    return getImage();
}

QFileInfo ImageSwitcher::next() {
    if (_image.row() >= _model->rowCount() - 1) {
        _image = _model->index(0, 0);
    }
    else {
        _image = _model->index(_image.row() + 1, 0);
    }
    return getImage();
}
