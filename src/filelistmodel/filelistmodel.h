#pragma once

#include "../filesystemobject.h"

#include <mutex>
#include <QStandardItemModel>
#include <QList>

enum Role {
    FullNameRole = Qt::UserRole + 1
};

class FileListModel : public QStandardItemModel {
    Q_OBJECT
public:
    explicit FileListModel(QObject* parent = nullptr);
    ~FileListModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Drag and drop
    bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
        const QModelIndex& parent) const override;

    QStringList mimeTypes() const override;

    bool
        dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

    QMimeData* mimeData(const QModelIndexList& indexes) const override;

    qulonglong itemHash(const QModelIndex& index) const;

    void updateCurrentPath(const QString& dir);

    void setItems(const QString& currentDirPath);

    std::map<qulonglong, FileSystemObject> getItems() const;

    FileSystemObject itemByHash(qulonglong hash) const;

    FileSystemObject* itemByIndex(const QModelIndex& index) const;

Q_SIGNALS:

    void itemEdited(qulonglong itemHash, QString newName);

private:
    mutable std::recursive_mutex fileListAndCurrentDirMutex;

    FileSystemObject currentDirObject;

    std::map<qulonglong, FileSystemObject> items;
};
