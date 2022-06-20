#pragma once

#include "../naturalsorting/naturalsorting.h"

#include <QSortFilterProxyModel>

class FileListSortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit FileListSortFilterProxyModel(QObject* parent);

    void setSortingOptions(SortingOptions options);

    // Drag and drop
    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
        const QModelIndex& parent) const override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

signals:

    void sorted();

protected:
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    NaturalSorting _sorter;
};
