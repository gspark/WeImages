#include "filelistmodel.h"
#include "../panel/columns.h"

#include <QMimeData>
#include <QUrl>
#include <QDir>
#include <set>

FileListModel::FileListModel(QObject* parent) :
    QStandardItemModel(0, NumberOfColumns, parent) {
}

FileListModel::~FileListModel() = default;

QVariant FileListModel::data(const QModelIndex & index, int role /*= Qt::DisplayRole*/) const {
    if (role == Qt::ToolTipRole) {
        if (!index.isValid())
            return QString();

        const FileSystemObject item = this->itemByHash(itemHash(index));
        return static_cast<QString>(item.fullName()/* % "\n\n" %
            QString::fromStdWString(OsShell::toolTip(item.fullAbsolutePath().toStdWString()))*/);
    }
    else if (role == Qt::EditRole) {
        return this->itemByHash(itemHash(index)).fullName();
    }
    else if (role == FullNameRole) {
        return this->itemByHash(itemHash(index)).fullName();
    }
    else
        return QStandardItemModel::data(index, role);
}

bool FileListModel::setData(const QModelIndex & index, const QVariant & value, int role) {
    if (role == Qt::EditRole) {
        const qulonglong hash = itemHash(index);
        emit itemEdited(hash, value.toString());
        return false;
    }
    else
        return QStandardItemModel::setData(index, value, role);
}

Qt::ItemFlags FileListModel::flags(const QModelIndex & idx) const {
    const Qt::ItemFlags flags = QStandardItemModel::flags(idx);
    if (!idx.isValid())
        return flags;

    const qulonglong hash = itemHash(idx);
    const FileSystemObject item = this->itemByHash(hash);

    if (!item.exists())
        return flags;
    else if (item.isCdUp())
        return flags & ~Qt::ItemIsSelectable;
    else
        return flags | Qt::ItemIsEditable;
}

bool FileListModel::canDropMimeData(const QMimeData * data, Qt::DropAction /*action*/, int /*row*/, int /*column*/,
    const QModelIndex& /*parent*/) const {
    return data->hasUrls();
}

QStringList FileListModel::mimeTypes() const {
    return QStringList("text/uri-list");
}

bool FileListModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int /*row*/, int /*column*/,
    const QModelIndex & parent) {
    if (action == Qt::IgnoreAction)
        return true;
    else if (!data->hasUrls())
        return false;

    FileSystemObject dest = parent.isValid() ? this->itemByHash(itemHash(parent)) : FileSystemObject(
        this->currentDirObject.fullAbsolutePath());
    if (dest.isFile())
        dest = FileSystemObject(dest.parentDirPath());
 

    const QList<QUrl> urls = data->urls();
    std::vector<FileSystemObject> objects;
    for (const QUrl& url : urls)
        objects.emplace_back(url.toLocalFile());

    if (objects.empty())
        return false;

    //if (action == Qt::CopyAction)
    //    return MainWindow::get()->copyFiles(std::move(objects), dest.fullAbsolutePath());
    //else if (action == Qt::MoveAction)
    //    return MainWindow::get()->moveFiles(std::move(objects), dest.fullAbsolutePath());
    //else
    //    return false;
}

QMimeData* FileListModel::mimeData(const QModelIndexList & indexes) const {
    auto mime = new QMimeData();
    QList<QUrl> urls;
    std::set<int> rows;
    for (const auto& idx : indexes) {
        if (idx.isValid() && rows.count(idx.row()) == 0) {
            const QString path = this->itemByHash(itemHash(index(idx.row(), 0))).fullAbsolutePath();
            if (!path.isEmpty()) {
                rows.insert(idx.row());
                urls.push_back(QUrl::fromLocalFile(path));
            }
        }
    }

    mime->setUrls(urls);
    return mime;
}

qulonglong FileListModel::itemHash(const QModelIndex & index) const {
    QStandardItem* itm = item(index.row(), 0);
    if (!itm)
        return 0;

    bool ok = false;
    const qulonglong hash = itm->data(Qt::UserRole).toULongLong(&ok);
    return hash;
}

void FileListModel::updateCurrentPath(const QString & dir)
{
    QString currPath = QDir::toNativeSeparators(dir);
    currentDirObject.setPath(currPath);
    setItems(currPath);
}

void FileListModel::setItems(const QString & currentDirPath)
{
    QFileInfoList list;
    {
        std::lock_guard<std::recursive_mutex> locker(fileListAndCurrentDirMutex);

        list = QDir{ currentDirPath }.entryInfoList(
            QDir::Dirs | QDir::Files | QDir::NoDot | QDir::Hidden | QDir::System, QDir::NoSort);
        items.clear();
    }
    //const bool showHiddenFiles = CSettings().value(KEY_INTERFACE_SHOW_HIDDEN_FILES, true).toBool();
    const bool showHiddenFiles = true;
    std::vector<FileSystemObject> objectsList;

    const size_t numItemsFound = (size_t)list.size();
    objectsList.reserve(numItemsFound);

    for (size_t i = 0; i < numItemsFound; ++i)
    {
#ifndef _WIN32
        // TODO: Qt bug?
        if (list[(int)i].absoluteFilePath() == QLatin1String("/.."))
            continue;
#endif
        objectsList.emplace_back(list[(int)i]);
        if (!objectsList.back().isFile() && !objectsList.back().isDir())
            objectsList.pop_back(); // Could be a socket
    }

    {
        std::lock_guard<std::recursive_mutex> locker(fileListAndCurrentDirMutex);

        for (const auto& object : objectsList)
        {
            if (object.exists() && (showHiddenFiles || !object.isHidden()))
                items[object.hash()] = object;
        }
    }
}

std::map<qulonglong, FileSystemObject> FileListModel::getItems() const
{
    return items;
}

FileSystemObject FileListModel::itemByHash(qulonglong hash) const
{
    std::lock_guard<std::recursive_mutex> locker(fileListAndCurrentDirMutex);

    const auto it = items.find(hash);
    return it != items.end() ? it->second : FileSystemObject();
}

FileSystemObject* FileListModel::itemByIndex(const QModelIndex& index) const
{
    FileSystemObject item = this->itemByHash(itemHash(index));
    return &item;
}

