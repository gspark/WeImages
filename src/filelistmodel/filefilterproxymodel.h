#ifndef FILEFILTERPROXYMODEL_H
#define FILEFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QCollator>
#include <QFileInfo>
#include <QStandardItem>

class QFileIconProvider;
class QFileSystemModel;

class FileFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    FileFilterProxyModel(int sortColumn = -1);

    void enableFilter(bool enable);
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QModelIndex proxyIndex(const QString& path, int column = 0) const;
    QFileInfo fileInfo(const QModelIndex& index) const;
    QFileInfo fileInfoByModel(const QModelIndex& index) const;
    QStandardItem* itemFromIndex(const QModelIndex& index) const;

    int getSortColumn() const;
protected:
    // filter
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    // sort
    bool nameCompare(const QFileInfo& leftInfo, const QFileInfo& rightInfo) const;
private:
    bool _useFilter;
    int _sortColumn;
    QCollator naturalCompare;
};


#endif // FILEFILTERPROXYMODEL_H
