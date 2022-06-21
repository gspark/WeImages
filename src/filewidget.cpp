#include "filewidget.h"
#include "config.h"
#include "filelistmodel/filelistmodel.h"
#include "filelistmodel/filelistsortfilterproxymodel.h"
#include "panel/columns.h"
#include "filesystemhelperfunctions.h"
#include "util/qdatetime_helpers.hpp"
#include "delegate/itemdelegate.h"
#include "delegate/itemdef.h"

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
#include <QTableView>
#include <QStandardItemModel>
#include <QActionGroup>
#include <QStackedWidget>


FileWidget::FileWidget(const QString& tag, ImageCore* imageCore, QWidget* parent) : QWidget(parent) {
    name = tag;
    this->imageCore = imageCore;

    fileViewType = FileViewType::List;
    list_delegate = nullptr;

    // init file model
    model = new FileListModel;
    sortModel = new FileListSortFilterProxyModel(this);
    sortModel->setSourceModel(model);

    // widget init
    setupToolBar();
    listView = nullptr;
    thumbnailView = nullptr;
    initListView();
    initWidgetLayout();
}

FileWidget::~FileWidget() {
    sortModel->deleteLater();
    listView = nullptr;
    thumbnailView = nullptr;
}


void FileWidget::setupToolBar() {
    toolBar = new QToolBar;
    toolBar->setContentsMargins(0, 0, 0, 0);

    QAction* detailAction = toolBar->addAction(QIcon(QLatin1String(":/up.png")), tr("detail"));
    connect(detailAction, &QAction::triggered, this, &FileWidget::detail);

    QAction* thumbnailAction = toolBar->addAction(QIcon(QLatin1String(":/minus.png")), tr("thumbnail"));
    connect(thumbnailAction, &QAction::triggered, this, &FileWidget::thumbnail);

    auto listGroup = new QActionGroup(this);
    listGroup->addAction(detailAction);
    listGroup->addAction(thumbnailAction);
    detailAction->setCheckable(true);
    thumbnailAction->setCheckable(true);
}


void FileWidget::initListView() {
    initTabelView();
    initThumbnailView();
}

void FileWidget::initTabelView()
{
    if (nullptr != listView)
    {
        return;
    }
    listView = new QTableView;
    listView->setContentsMargins(0, 0, 0, 0);
    //listView->setModel(sortModel);

    // interactive settings
    //listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // view settings
    //listView->setStyle(QStyleFactory::create("Fusion"));

    //connect(listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileWidget::onSelectionChanged);

    connect(listView, &QListView::clicked, this, &FileWidget::onTreeViewClicked);
    //connect(listView, &QListView::doubleClicked, this, &FileWidget::onTreeViewDoubleClicked);
}

void FileWidget::initThumbnailView()
{
    if (nullptr != thumbnailView)
    {
        return;
    }
    m_delegate = new ItemDelegate(this->imageCore, this);

    //m_proxyModel = new QSortFilterProxyModel(ui->listView);
    //m_proxyModel->setSourceModel(m_model);
    //m_proxyModel->setFilterRole(Qt::UserRole);
    //m_proxyModel->setDynamicSortFilter(true);
    //ui->listView->setModel(m_proxyModel);                  //为委托设置模型
    //ui->listView->setViewMode(QListView::IconMode); //设置Item图标显示
    //ui->listView->setDragEnabled(false);            //控件不允许拖动

    thumbnailView = new QListView(this);
    thumbnailView->setContentsMargins(0, 0, 0, 0);
    thumbnailView->setStyleSheet("background-color: transparent;border:1px solid #EFEFEF;");
    //为视图设置委托
    thumbnailView->setItemDelegate(m_delegate);
    //为视图设置控件间距
    thumbnailView->setSpacing(15);
    thumbnailView->setViewMode(QListView::IconMode);
    thumbnailView->setIconSize(QSize(210, 210));
    thumbnailView->setResizeMode(QListView::Adjust);
    thumbnailView->setMovement(QListView::Static);
}

void FileWidget::initWidgetLayout() {
    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    this->setContentsMargins(0, 0, 0, 0);
    this->setLayout(vLayout);

    stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(listView);
    stackedWidget->addWidget(thumbnailView);

    vLayout->addWidget(toolBar);
    vLayout->addWidget(stackedWidget);
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

void FileWidget::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {

}

void FileWidget::fillFromList(const std::map<qulonglong, FileSystemObject> items)
{
    QStandardItemModel* data = (QStandardItemModel*)this->model;
    sortModel->setSourceModel(nullptr);
    data->clear();

    if (FileViewType::List == fileViewType)
    {
        if (nullptr != list_delegate)
        {
            this->listView->setItemDelegate(list_delegate);
        }
        setListView(items, data);
        stackedWidget->setCurrentIndex(0);
    }
    else if (FileViewType::thumbnail == fileViewType)
    {
        setThumbnailView(items, data);
        stackedWidget->setCurrentIndex(1);
    }
}

int FileWidget::setListView(const std::map<qulonglong, FileSystemObject> items, QStandardItemModel* data)
{
    data->setColumnCount(NumberOfColumns);
    data->setHorizontalHeaderLabels(QStringList{ tr("Name"), tr("Ext"), tr("Size"), tr("Date") });

    struct TreeViewItem {
        const int row;
        const FileListViewColumn column;
        QStandardItem* const item;
    };

    std::vector<TreeViewItem> qTreeViewItems;
    qTreeViewItems.reserve(items.size() * NumberOfColumns);

    int itemRow = 0;

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

    if (nullptr == list_delegate)
    {
        list_delegate = listView->itemDelegate();
    }
    return itemRow;
}

int FileWidget::setThumbnailView(const std::map<qulonglong, FileSystemObject> items, QStandardItemModel* data)
{
    for (const auto& item : items) {
        const FileSystemObject& object = item.second;

        QStandardItem* thumbnailItem = new QStandardItem;

        ItemData itemData;
 
        itemData.fullName = object.fullName();
        itemData.fullAbsolutePath = object.fullAbsolutePath();
        itemData.extension = object.extension();

        thumbnailItem->setData(QVariant::fromValue(itemData), Qt::UserRole + 3);//整体存取

        data->appendRow(thumbnailItem);      //追加Item
    }
    thumbnailView->setModel(data);
    return items.size();
}

void FileWidget::thumbnail()
{
    this->fileViewType = FileViewType::thumbnail;
    //QAbstractItemModel *data = this->proxyModel->sourceModel();

    this->thumbnailView->setViewMode(QListView::IconMode);
    //this->listView->setIconSize(QSize(120, 120));
    //this->listView->setAlternatingRowColors(false);
    //this->listView->viewport()->setAttribute(Qt::WA_StaticContents);
    //this->listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    fillFromList(model->getItems());
}

void FileWidget::detail()
{
    this->fileViewType = FileViewType::List;
    fillFromList(model->getItems());
}

void FileWidget::onTreeViewClicked(const QModelIndex& index) {
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


void FileWidget::onItemActivated(const QString& path) {
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

