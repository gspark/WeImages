#include "filefilterproxymodel.h"
#include "filelistmodel.h"

#include <QDateTime>
#include <QFileSystemModel>


FileFilterProxyModel::FileFilterProxyModel(int sortColumn)
{
    useFilter = false;
    this->sortColumn = sortColumn;
}

// filter
void FileFilterProxyModel::enableFilter(bool enable)
{
    useFilter = enable;
}

// only keep dir
bool FileFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (!useFilter)
        return true;

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    //QFileSystemModel *model = (QFileSystemModel *)sourceModel();
    //QFileInfo info = model->fileInfo(index);

    QFileInfo info = this->fileInfoBySource(index);

    if (info.fileName() == "." || info.fileName() == "..") {
        return false;
    }

    return (info.isDir() && !info.isShortcut());
}

// sort
void FileFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    sortColumn = column;
    //QSortFilterProxyModel::sort(column, order);
}

bool FileFilterProxyModel::nameCompare(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    //QFileSystemModel* model = (QFileSystemModel*)sourceModel();
    //QFileInfo leftInfo = model->fileInfo(source_left);
    //QFileInfo rightInfo = model->fileInfo(source_right);

    QFileInfo leftInfo = this->fileInfoBySource(source_left);
    QFileInfo rightInfo = this->fileInfoBySource(source_right);

    // place drives before directories
    bool l = this->type(source_left) == "Drive";
    bool r = this->type(source_right) == "Drive";
    if (l ^ r)
        return l;
    if (l && r)     // both Drive
        return naturalCompare.compare(leftInfo.filePath(), rightInfo.filePath()) < 0;

    // place directories before files
    bool left = leftInfo.isDir();
    bool right = rightInfo.isDir();
    if (left ^ right)
        return left;

    return naturalCompare.compare(leftInfo.fileName(), rightInfo.fileName()) < 0;
}

bool FileFilterProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    //QFileSystemModel* model = (QFileSystemModel*)sourceModel();
    //QFileInfo leftInfo = model->fileInfo(source_left);
    //QFileInfo rightInfo = model->fileInfo(source_right);

    QFileInfo leftInfo = this->fileInfoBySource(source_left);
    QFileInfo rightInfo = this->fileInfoBySource(source_right);

    switch (sortColumn) {
    case 0:
    case 1: {
        return nameCompare(source_left, source_right);
    }
    case 2: {
        qint64 sizeDifference = leftInfo.size() - rightInfo.size();
        if (sizeDifference == 0)    // use nameCompare if the left equal to the right
            return nameCompare(source_left, source_right);

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
            return nameCompare(source_left, source_right);

        return leftInfo.lastModified() < rightInfo.lastModified();
    }
    default:
        return false;
    }
}

QString FileFilterProxyModel::type(const QModelIndex& index) const
{
    auto* model = dynamic_cast<QFileSystemModel*>(sourceModel());
    if (model)
    {
        return model->type(index);
    }

    auto* lmodel = dynamic_cast<FileListModel*>(sourceModel());
    if (lmodel)
    {
        return lmodel->type(index);
    }
    return "";
}

//// QFileSystemModel
//QFileSystemModel *FileFilterProxyModel::srcModel()
//{
//    return (QFileSystemModel *)sourceModel();
//}

//QFileIconProvider *FileFilterProxyModel::iconProvider() const
//{
//    QFileSystemModel *model = (QFileSystemModel *)sourceModel();
//
//    return (QFileIconProvider*)model->iconProvider();
//}

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

// pIndex is proxy model's index
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

QFileInfo FileFilterProxyModel::fileInfoBySource(const QModelIndex& index) const
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
