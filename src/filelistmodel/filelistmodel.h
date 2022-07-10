#pragma once

#include <QStandardItemModel>
#include <QFileInfo>

enum Role {
    FullNameRole = Qt::UserRole + 1
};

enum FileListViewColumn {
    //CheckBoxColumn, NameColumn, ExtColumn, SizeColumn, DateColumn, NumberOfColumns
    CheckBoxColumn, NameColumn, SizeColumn, DateColumn, NumberOfColumns
};

enum Type { Dir, File, System };

class QFileIconProvider;

class FileListModel : public QStandardItemModel {
    Q_OBJECT
public:
    explicit FileListModel(QFileIconProvider* iconProvider, QObject* parent = nullptr);
    ~FileListModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QFileInfo fileInfo(const QModelIndex& index) const;

    QFileInfo fileInfo(const QStandardItem* item) const;

    QString type(const QModelIndex& index) const;

    QModelIndex index(const QString& path, int column = 0) const;
Q_SIGNALS:

private:
    QFileIconProvider* m_iconProvider;
};
