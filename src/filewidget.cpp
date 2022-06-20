#include "filewidget.h"
#include "config.h"
#include "filelistmodel/filelistmodel.h"
#include "filelistmodel/filelistsortfilterproxymodel.h"
#include "panel/columns.h"
#include "filesystemhelperfunctions.h"
#include "util/qdatetime_helpers.hpp"

#include <QApplication>
#include <QStyleFactory>
#include <QGridLayout>
#include <QMessageBox>
#include <QSizePolicy>
#include <QSplitter>
#include <QLineEdit>
#include <QFocusEvent>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QClipboard>
#include <QIcon>
#include <QUrl>
#include <QToolBar>
#include <QListView>
#include <QStandardItemModel>


FileWidget::FileWidget(const QString &tag, ImageCore* imageCore, QWidget *parent) : QWidget(parent) {
    name = tag;
    this->imageCore = imageCore;

    // init file model
    model = new FileListModel;
    sortModel = new FileListSortFilterProxyModel(this);
    sortModel->setSourceModel(model);

    // widget init
    setupToolBar();
    initListView();
    initWidgetLayout();
}

FileWidget::~FileWidget() {
    sortModel->deleteLater();
    listView = nullptr;
}


void FileWidget::setupToolBar() {
    toolBar = new QToolBar;
    toolBar->setContentsMargins(0, 0, 0, 0);

    QAction* detailAction = toolBar->addAction(QIcon(QLatin1String(":/up.png")), tr("detail"));
    connect(detailAction, &QAction::triggered, this, &FileWidget::detail);

    QAction* thumbnailAction = toolBar->addAction(QIcon(QLatin1String(":/minus.png")), tr("thumbnail"));
    connect(thumbnailAction, &QAction::triggered, this, &FileWidget::thumbnail);
}


void FileWidget::initListView() {
    listView = new QListView;
    listView->setContentsMargins(0, 0, 0, 0);
//    treeView->setModel(fileModel);
    listView->setModel(sortModel);

    // interactive settings
    listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // view settings
    //listView->setStyle(QStyleFactory::create("Fusion"));

    connect(listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileWidget::onSelectionChanged);

    connect(listView, &QListView::clicked, this, &FileWidget::onTreeViewClicked);
    //connect(listView, &QListView::doubleClicked, this, &FileWidget::onTreeViewDoubleClicked);
}

void FileWidget::initWidgetLayout() {
    auto* vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    this->setContentsMargins(0, 0, 0, 0);
    this->setLayout(vLayout);
    vLayout->addWidget(toolBar);
    vLayout->addWidget(listView);
}

void FileWidget::cdPath(const QString& path)
{
    if (path.isEmpty())
        return;
    updateCurrentPath(path);
}

void FileWidget::updateCurrentPath(const QString& dir)
{
    model->updateCurrentPath(dir);
    fillFromList(model->getItems());
   
}

void FileWidget::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {

}

void FileWidget::fillFromList(const std::map<qulonglong, FileSystemObject> items)
{
    QStandardItemModel* data = (QStandardItemModel*)this->sortModel->sourceModel();
    sortModel->setSourceModel(nullptr);
    data->clear();

    data->setColumnCount(NumberOfColumns);
    //data->setHorizontalHeaderLabels(QStringList{ tr("Name"), tr("Ext"), tr("Size"), tr("Date") });
    model->setHeaderData(0, Qt::Horizontal, tr("Name"));
    model->setHeaderData(1, Qt::Horizontal, tr("Ext"));
    model->setHeaderData(2, Qt::Horizontal, tr("Size"));
    model->setHeaderData(3, Qt::Horizontal, tr("Date"));


    int itemRow = 0;
    struct TreeViewItem {
        const int row;
        const FileListViewColumn column;
        QStandardItem* const item;
    };

    std::vector<TreeViewItem> qTreeViewItems;
    qTreeViewItems.reserve(items.size() * NumberOfColumns);
    if (this->listView->viewMode() == QListView::ListMode)
    {
        for (const auto& item : items) {
            //auto fileNameItem = new QStandardItem();
            //fileNameItem->setEditable(false);
            //
            //fileNameItem->setData(item.second.name(), Qt::DisplayRole);
            //
            //data->appendRow(fileNameItem);

            const FileSystemObject& object = item.second;
            const auto& props = object.properties();

            auto fileNameItem = new QStandardItem();
            fileNameItem->setEditable(false);
            if (props.type == Directory && props.type != Bundle)
                fileNameItem->setData(QString("[" % (object.isCdUp() ? QLatin1String("..") : props.fullName) % "]"), Qt::DisplayRole);
            else if (props.completeBaseName.isEmpty() && props.type == File) // File without a name, displaying extension in the name field and adding point to extension
                fileNameItem->setData(QString('.') + props.extension, Qt::DisplayRole);
            else
                fileNameItem->setData(props.completeBaseName, Qt::DisplayRole);
            //fileNameItem->setIcon(CIconProvider::iconForFilesystemObject(object, useLessPreciseIcons));
            fileNameItem->setData(static_cast<qulonglong>(props.hash), Qt::UserRole); // Unique identifier for this object
            qTreeViewItems.emplace_back(TreeViewItem{ itemRow, NameColumn, fileNameItem });

            auto fileExtItem = new QStandardItem();
            fileExtItem->setEditable(false);
            if (!object.isCdUp() && !props.completeBaseName.isEmpty() && !props.extension.isEmpty())
                fileExtItem->setData(props.extension, Qt::DisplayRole);
            fileExtItem->setData(static_cast<qulonglong>(props.hash), Qt::UserRole); // Unique identifier for this object
            qTreeViewItems.emplace_back(TreeViewItem{ itemRow, ExtColumn, fileExtItem });

            auto sizeItem = new QStandardItem();
            sizeItem->setEditable(false);
            if (props.size > 0 || props.type == File)
                sizeItem->setData(fileSizeToString(props.size), Qt::DisplayRole);

            sizeItem->setData(static_cast<qulonglong>(props.hash), Qt::UserRole); // Unique identifier for this object
            qTreeViewItems.emplace_back(TreeViewItem{ itemRow, SizeColumn, sizeItem });

            auto dateItem = new QStandardItem();
            dateItem->setEditable(false);
            if (!object.isCdUp())
            {
                QDateTime modificationDate = fromTime_t(props.modificationDate).toLocalTime();
                dateItem->setData(modificationDate.toString("dd.MM.yyyy hh:mm:ss"), Qt::DisplayRole);
            }
            dateItem->setData(static_cast<qulonglong>(props.hash), Qt::UserRole); // Unique identifier for this object
            qTreeViewItems.emplace_back(TreeViewItem{ itemRow, DateColumn, dateItem });

            ++itemRow;
        }
        for (const auto& qTreeViewItem : qTreeViewItems) {
            data->setItem(qTreeViewItem.row, qTreeViewItem.column, qTreeViewItem.item);
        }
        sortModel->setSourceModel(data);
        listView->setModel(data);
    }
}

void FileWidget::thumbnail()
{
    //QAbstractItemModel *data = this->proxyModel->sourceModel();
    
    this->listView->setViewMode(QListView::IconMode);
    //this->listView->setIconSize(QSize(120, 120));
    //this->listView->setAlternatingRowColors(false);
    //this->listView->viewport()->setAttribute(Qt::WA_StaticContents);
    //this->listView->setAttribute(Qt::WA_MacShowFocusRect, false);
}

void FileWidget::detail()
{
    this->listView->setViewMode(QListView::ListMode);
}

void FileWidget::onTreeViewClicked(const QModelIndex &index) {
    QString target;
    FileSystemObject* info = model->itemByIndex(index);

    if (info->isSymLink()) {
        // handle shortcut
        if (!info->exists()) {
            return;
        }
        target = info->fullName();
    }
    else {
        target = info->fullName();
    }

    // If the file is a symlink, this function returns true if the target is a directory (not the symlink)
    if (info->isDir()) {
        //cdPath(target);
    }
    else {    //    else if (info.isFile())
        this->imageCore->loadFile(target);
    }
}

//void FileWidget::onTreeViewDoubleClicked(const QModelIndex &index) {
//
//}


void FileWidget::onItemActivated(const QString &path) {
//    if (!QFileInfo::exists(path)) {   // shortcut maybe not exist
//        return;
//    }
    //QModelIndex index = proxyModel->proxyIndex(path);
    //onTreeViewClicked(index);
}

void FileWidget::onNavigateBarClicked(const QString& path)
{
    cdPath(path);
}

