#include "filelistmodel.h"
#include "../delegate/thumbnailData.h"

FileListModel::FileListModel(QObject* parent) : QStandardItemModel(0, NumberOfColumns, parent) {
}

FileListModel::~FileListModel() = default;

QVariant FileListModel::data(const QModelIndex & index, int role /*= Qt::DisplayRole*/) const {
    return QStandardItemModel::data(index, role);

    if (!index.isValid())
        return QVariant();

    QVariant variant = index.data(Qt::DisplayRole);
    if (variant.isNull())
    {
        return QVariant();
    }
    ThumbnailData data = variant.value<ThumbnailData>();

    switch (role)
    {
    case Qt::TextAlignmentRole:
        return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    case Qt::DisplayRole:
    {
        switch (index.column())
        {
        NameColumn:
            return data.fileName;
        ExtColumn:
            return data.extension;
        default:
            return QVariant();
        }
    }
    case Qt::CheckStateRole:
    {
        if (index.column() == CheckBoxColumn)
            return true;
    }
    default:
        return QVariant();
    }

    return QVariant();
}

bool FileListModel::setData(const QModelIndex & index, const QVariant & value, int role) {

    return QStandardItemModel::setData(index, value, role);
}


Qt::ItemFlags FileListModel::flags(const QModelIndex & idx) const {
    const Qt::ItemFlags flags = QStandardItemModel::flags(idx);
    return flags;
}
