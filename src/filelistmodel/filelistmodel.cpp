#include "filelistmodel.h"
#include "../delegate/thumbnailData.h"

#include <QFileIconProvider>

FileListModel::FileListModel(QFileIconProvider* iconProvider, QObject* parent) : QStandardItemModel(0, NumberOfColumns, parent) {
    this->m_iconProvider = iconProvider;
}

FileListModel::~FileListModel() = default;

QVariant FileListModel::data(const QModelIndex & index, int role /*= Qt::DisplayRole*/) const {
    return QStandardItemModel::data(index, role);

    /*if (!index.isValid())
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

    return QVariant();*/
}

bool FileListModel::setData(const QModelIndex & index, const QVariant & value, int role) {

    return QStandardItemModel::setData(index, value, role);
}

Qt::ItemFlags FileListModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}

QFileInfo FileListModel::fileInfo(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QFileInfo();
    }
   
    QStandardItem* item = this->itemFromIndex(index);
    if (item)
    {
        QVariant variant = item->data(Qt::UserRole + 3);
        if (variant.canConvert<ThumbnailData>())
        {
            auto thumbnailData = variant.value<ThumbnailData>();
            return thumbnailData.fileInfo;
        }
        return variant.value<QFileInfo>();
    }
    return QFileInfo();
}

QString FileListModel::type(const QModelIndex& index) const
{
    return this->m_iconProvider->type(this->fileInfo(index));
}

QModelIndex FileListModel::index(const QString& path, int column /*= 0*/) const
{
    return QModelIndex();
}
