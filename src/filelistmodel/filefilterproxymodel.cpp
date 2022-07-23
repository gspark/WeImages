#include "filefilterproxymodel.h"
#include "filelistmodel.h"
#include "../filesystemhelperfunctions.h"

#include <QDateTime>
#include <QFileSystemModel>


FileFilterProxyModel::FileFilterProxyModel(int sortColumn) : 
    _useFilter(false), _sortColumn(sortColumn)
{
}

// filter
void FileFilterProxyModel::enableFilter(bool enable)
{
    _useFilter = enable;
}

// only keep dir
bool FileFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (!_useFilter)
        return true;

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    QFileInfo info = this->fileInfoByModel(index);

    if (info.fileName() == "." || info.fileName() == "..") {
        return false;
    }

    return (info.isDir() && !info.isShortcut());
}

// sort
void FileFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    _sortColumn = column;
    QSortFilterProxyModel::sort(column, order);
}

bool FileFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    auto srcModel = dynamic_cast<QStandardItemModel*>(sourceModel());
    if (nullptr != srcModel)
    {
        QStandardItem* const l = srcModel->item(left.row(), left.column());
        QStandardItem* const r = srcModel->item(right.row(), right.column());

        if (!l && r)
            return true;
        else if (!r && l)
            return false;
        else if (!l && !r)
            return false;
    }
 
    const int sortColumn = left.column();

    QFileInfo leftInfo = this->fileInfoByModel(left);
    QFileInfo rightInfo = this->fileInfoByModel(right);

    switch (sortColumn) {
    case 0:
    case 1: {
        return nameCompare(leftInfo, rightInfo);
    }
    case 2: {
        qint64 sizeDifference = leftInfo.size() - rightInfo.size();
        if (sizeDifference == 0) {
            // use nameCompare if the left equal to the right
            return nameCompare(leftInfo, rightInfo);
        }
        return sizeDifference < 0;
    }
    //case 2: {
    //    //int compare = naturalCompare.compare(model->type(source_left), model->type(source_right));
    //    int compare = naturalCompare.compare(this->type(source_left), this->type(source_right));
    //    if (compare == 0)
    //        return nameCompare(source_left, source_right);

    //    return compare < 0;
    //}
    case 3: {
        if (leftInfo.lastModified() == rightInfo.lastModified()) 
        {
            return nameCompare(leftInfo, rightInfo);
        }
        return leftInfo.lastModified() < rightInfo.lastModified();
    }
    default:
        return false;
    }
}

bool FileFilterProxyModel::nameCompare(const QFileInfo& leftInfo, const QFileInfo& rightInfo) const
{
    //// place drives before directories
    //bool l = isDrive(leftInfo);
    //bool r = isDrive(rightInfo);
    //if (l ^ r)
    //    return l;
    //if (l && r)     // both Drive
    //    return naturalCompare.compare(leftInfo.filePath(), rightInfo.filePath()) < 0;

    // place directories before files
    bool l = leftInfo.isDir();
    bool r = rightInfo.isDir();
    if (l ^ r)
        return l;
    //DWORD start = GetTickCount();
    bool ret = naturalCompare.compare(leftInfo.fileName(), rightInfo.fileName()) < 0;
    //LOG_INFO << "naturalCompare.compare time: " << GetTickCount() - start;
    return ret;
}

// return proxy model's index
QModelIndex FileFilterProxyModel::proxyIndex(const QString& path, int column) const
{
    auto* model = dynamic_cast<QFileSystemModel*>(sourceModel());
    if (model)
    {
        // TODO
        // model->setRootPath(path);
        return mapFromSource(model->index(path, column));
    }

    auto* lmodel = dynamic_cast<FileListModel*>(sourceModel());
    if (lmodel)
    {
        mapFromSource(lmodel->index(path, column));
    }
    return QModelIndex();
}

// index is proxy model's index
QFileInfo FileFilterProxyModel::fileInfo(const QModelIndex& index) const
{
    auto* model = dynamic_cast<QFileSystemModel*>(sourceModel());
    if (model)
    {
        return model->fileInfo(mapToSource(index));
    }
    auto* lmodel = dynamic_cast<FileListModel*>(sourceModel());
    if (lmodel)
    {
        return lmodel->fileInfo(mapToSource(index));
    }
    return QFileInfo();
}


//************************************
// fileInfo from model
//************************************
QFileInfo FileFilterProxyModel::fileInfoByModel(const QModelIndex& index) const
{
    auto* model = dynamic_cast<QFileSystemModel*>(sourceModel());
    if (model)
    {
        return model->fileInfo(index);
    }
    auto* lmodel = dynamic_cast<FileListModel*>(sourceModel());
    if (lmodel)
    {
        return lmodel->fileInfo(index.siblingAtColumn(0));
    }
    return QFileInfo();
}

QStandardItem* FileFilterProxyModel::itemFromIndex(const QModelIndex& index) const
{
    auto* model = dynamic_cast<QFileSystemModel*>(sourceModel());
    if (model)
    {
        return nullptr;
    }
    auto* lmodel = dynamic_cast<FileListModel*>(sourceModel());
    if (lmodel)
    {
        return lmodel->itemFromIndex(mapToSource(index));
    }
    return nullptr;
}

int FileFilterProxyModel::getSortColumn() const
{
    return _sortColumn;
}
