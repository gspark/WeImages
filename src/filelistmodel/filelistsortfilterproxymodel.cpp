#include "filelistsortfilterproxymodel.h"
#include "..\panel\columns.h"

#include <QStandardItemModel>
#include "filelistmodel.h"

FileListSortFilterProxyModel::FileListSortFilterProxyModel(QObject* parent) :
    QSortFilterProxyModel(parent),
    _sorter(NaturalSorting(nsaQCollator, {})) {
}

void FileListSortFilterProxyModel::setSortingOptions(SortingOptions options) {
    _sorter.setSortingOptions(options);
}

bool FileListSortFilterProxyModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
    const QModelIndex& parent) const {
    QModelIndex srcIndex = mapToSource(index(row, column));
    return sourceModel()->canDropMimeData(data, action, srcIndex.row(), srcIndex.column(), parent);
}

void FileListSortFilterProxyModel::sort(int column, Qt::SortOrder order) {
    QSortFilterProxyModel::sort(column, order);
    emit sorted();
}

inline bool isFileOrBundle(const FileSystemObject& item) {
    return item.isFile() || item.isBundle();
}

bool FileListSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
    const int sortColumn = left.column();

    auto srcModel = dynamic_cast<FileListModel*>(sourceModel());
    QStandardItem* const l = srcModel->item(left.row(), left.column());
    QStandardItem* const r = srcModel->item(right.row(), right.column());

    if (!l && r)
        return true;
    else if (!r && l)
        return false;
    else if (!l && !r)
        return false;

    const qulonglong leftHash = l->data(Qt::UserRole).toULongLong();
    const qulonglong rightHash = r->data(Qt::UserRole).toULongLong();

    const FileSystemObject leftItem = srcModel->itemByHash(leftHash),
                   rightItem = srcModel->itemByHash(rightHash);

    const bool descendingOrder = sortOrder() == Qt::DescendingOrder;
    // Folders always before files, no matter the sorting column and direction
    if (!isFileOrBundle(leftItem) && isFileOrBundle(rightItem))
        return !descendingOrder;  // always keep directory on top
    else if (isFileOrBundle(leftItem) && !isFileOrBundle(rightItem))
        return descendingOrder;   // always keep directory on top

    // [..] is always on top
    if (leftItem.isCdUp())
        return !descendingOrder;
    else if (rightItem.isCdUp())
        return descendingOrder;

    switch (sortColumn) {
    case NameColumn:
        return _sorter.lessThan(leftItem.name(), rightItem.name());
    case ExtColumn:
        if (!isFileOrBundle(leftItem) &&
            !isFileOrBundle(rightItem)) // Sorting directories by name, files - by extension
            return _sorter.lessThan(leftItem.name(), rightItem.name());
        else if (isFileOrBundle(leftItem) && isFileOrBundle(rightItem) && leftItem.extension().isEmpty() &&
            rightItem.extension().isEmpty())
            return _sorter.lessThan(leftItem.name(), rightItem.name());
        else {
            QString leftExt = leftItem.extension(), rightExt = rightItem.extension();
            QString leftName = leftItem.name(), rightName = rightItem.name();

            // Special handling for files with no extension.
            // They will be displayed with a leading '.', and I want them to appear first in the list, before files with no extension.
            if (rightName.isEmpty()) {
                rightName = '.' + rightExt;
                rightExt.clear();
            }

            if (leftName.isEmpty()) {
                leftName = '.' + leftExt;
                leftExt.clear();
            }

            if (_sorter.lessThan(leftExt, rightExt))
                return true;
            else if (_sorter.lessThan(rightExt, leftExt)) // check if extensions are the same
                return false;
            else // if they are - compare by names
                return _sorter.lessThan(leftName, rightName);
        }
    case SizeColumn:
        return leftItem.size() < rightItem.size();
    case DateColumn:
        return leftItem.properties().modificationDate < rightItem.properties().modificationDate;
    default:
        break;
    }

    return false;
}
