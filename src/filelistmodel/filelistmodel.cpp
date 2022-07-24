#include "filelistmodel.h"
#include "../delegate/thumbnailData.h"
#include "../filesystemhelperfunctions.h"
#include "../imagecore.h"

#include <QFileIconProvider>

FileListModel::FileListModel(ImageCore* imageCore, QFileIconProvider* iconProvider, QObject* parent) : QStandardItemModel(0, NumberOfColumns, parent) {
    this->_iconProvider = iconProvider;
    this->_imageCore = imageCore;
    this->setHorizontalHeaderLabels(QStringList{ tr(""),tr("Name")/*, tr("Ext")*/, tr("Size"), tr("Date") });
}

FileListModel::~FileListModel() = default;

QVariant FileListModel::data(const QModelIndex & index, int role /*= Qt::DisplayRole*/) const {
    return QStandardItemModel::data(index, role);
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
        //if (variant.canConvert<ThumbnailData>())
        //{
            auto thumbnailData = variant.value<ThumbnailData>();
            return thumbnailData.fileInfo;
        //}
        //return variant.value<QFileInfo>();
    }
    return QFileInfo();
}

QFileInfo FileListModel::fileInfo(const QStandardItem* item) const
{
    if (item == nullptr)
    {
        return QFileInfo();
    }
    QVariant variant = item->data(Qt::UserRole + 3);
    if (variant.isNull())
    {
        return QFileInfo();
    }
    //if (variant.canConvert<ThumbnailData>())
    //{
    //    auto data = variant.value<ThumbnailData>();
    //    return data.fileInfo;
    //}
    auto data = variant.value<ThumbnailData>();
    return data.fileInfo;
}

QString FileListModel::type(const QModelIndex& index) const
{
    return this->_iconProvider->type(this->fileInfo(index));
}

QString FileListModel::type(const QFileInfo& fileInfo) const
{
    return this->_iconProvider->type(fileInfo);
}

QModelIndex FileListModel::index(const QString& path, int column /*= 0*/) const
{
    return QModelIndex();
}

void FileListModel::updateItems(const QList<QFileInfo> fileInfos)
{
    this->removeRows(0, this->rowCount());
    if (fileInfos.isEmpty())
    {
        return;
    }
    //this->beginResetModel();
    this->setRowCount(fileInfos.size());
    int itemRow = 0;
    for (const auto& fileInfo : fileInfos)
    {
        ThumbnailData data;
        data.isWeChatImage = this->_imageCore->isWeChatImage(fileInfo);
        data.fileInfo = fileInfo;
    
        auto checkBoxItem = new QStandardItem();
        checkBoxItem->setData(QVariant::fromValue(data), Qt::UserRole + 3);
        //checkBoxItem->setData(Qt::CheckState::Unchecked, Qt::CheckStateRole);
        this->setItem(itemRow, CheckBoxColumn, checkBoxItem);

        auto fileNameItem = new QStandardItem();
        fileNameItem->setIcon(_iconProvider->icon(fileInfo));
        fileNameItem->setData(fileInfo.fileName(), Qt::DisplayRole);
        this->setItem(itemRow, NameColumn, fileNameItem);

        //auto fileExtItem = new QStandardItem();
        //fileExtItem->setData(fileInfo.suffix(), Qt::DisplayRole);
        //fileListModel->setItem(itemRow, ExtColumn, fileExtItem);

        auto sizeItem = new QStandardItem();
        sizeItem->setData(fileSizeToString(fileInfo.size()), Qt::DisplayRole);
        this->setItem(itemRow, SizeColumn, sizeItem);

        auto dateItem = new QStandardItem();
        dateItem->setData(fileInfo.lastModified().toString("yyyy-MM-dd"), Qt::DisplayRole);
        this->setItem(itemRow, DateColumn, dateItem);
        itemRow++;
    }
    //this->endResetModel();
    //emit onUpdateItems();
}

