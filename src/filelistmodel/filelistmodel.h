#pragma once

#include <QStandardItemModel>

enum Role {
    FullNameRole = Qt::UserRole + 1
};

enum FileListViewColumn {
    //CheckBoxColumn, NameColumn, ExtColumn, SizeColumn, DateColumn, NumberOfColumns
    CheckBoxColumn, NameColumn, SizeColumn, DateColumn, NumberOfColumns
};

class FileListModel : public QStandardItemModel {
    Q_OBJECT
public:
    explicit FileListModel(QObject* parent = nullptr);
    ~FileListModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

Q_SIGNALS:

private:

};
