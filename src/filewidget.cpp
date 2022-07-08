#include "filewidget.h"
#include "config.h"
#include "filelistmodel/filefilterproxymodel.h"
#include "delegate/thumbnailDelegate.h"
#include "delegate/thumbnailData.h"
#include "logger/Logger.h"
#include "slideshow.h"
#include "delegate/checkBoxDelegate.h"
#include "filelistmodel/filelistmodel.h"
#include "filesystemhelperfunctions.h"

#include <QApplication>
#include <QStyleFactory>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QFileIconProvider>
#include <QIcon>
#include <QToolBar>
#include <QListView>
#include <QTableView>
#include <QStandardItemModel>
#include <QActionGroup>
#include <QStackedWidget>
#include <QHeaderView>
#include <QtConcurrent/QtConcurrent>
#include <functional>
#include <QMimeType>



FileWidget::FileWidget(/*QAbstractItemModel* model, */ImageCore* imageCore, QWidget* parent) : QWidget(parent) {
 
    this->imageCore = imageCore;
    this->m_iconProvider = nullptr;
    this->thumbnailModel = nullptr;
    this->fileListModel = nullptr;
    this->proxyModel = nullptr;

    this->fileViewType = ::FileViewType::Table;

    //// init file model
    //fileSystemMode = (QFileSystemModel*)model;

    // widget init
    setupToolBar();
    tableView = nullptr;
    thumbnailView = nullptr;
    thumbnailDelegate = nullptr;
    checkBoxDelegate = nullptr;
    initListView();
    initWidgetLayout();
}

FileWidget::~FileWidget() {
    if (nullptr != thumbnailDelegate)
    {
        delete thumbnailDelegate;
    }
    if (nullptr != checkBoxDelegate)
    {
        delete checkBoxDelegate;
    }
    delete m_iconProvider;

    //proxyModel->deleteLater();
    tableView->deleteLater();
    thumbnailView->deleteLater();

}

void FileWidget::setupToolBar() {
    toolBar = new QToolBar;
    toolBar->setContentsMargins(0, 0, 0, 0);

    QAction* detailAction = toolBar->addAction(QIcon(QLatin1String(":/up.png")), tr("detail"));
    connect(detailAction, &QAction::triggered, this, &FileWidget::detail);

    QAction* thumbnailAction = toolBar->addAction(QIcon(QLatin1String(":/minus.png")), tr("thumbnail"));
    connect(thumbnailAction, &QAction::triggered, this, &FileWidget::thumbnail);

    toolBar->addSeparator();

    QAction* selectAllAction = toolBar->addAction(QIcon(QLatin1String(":/minus.png")), tr("select all"));
    connect(selectAllAction, &QAction::triggered, this, &FileWidget::selectAll);

    auto listGroup = new QActionGroup(this);
    listGroup->addAction(detailAction);
    listGroup->addAction(thumbnailAction);
    detailAction->setCheckable(true);
    thumbnailAction->setCheckable(true);
}

QFileIconProvider* FileWidget::ensureIconProvider()
{
    if (!m_iconProvider)
        m_iconProvider = new QFileIconProvider;
    return m_iconProvider;

}

void FileWidget::initListView() {
    initTableView();
    initThumbnailView();
}

void FileWidget::initTableView()
{
    if (nullptr != tableView)
    {
        return;
    }
    checkBoxDelegate = new CheckBoxDelegate(this);
    tableView = new QTableView;
    tableView->verticalHeader()->setVisible(false);
    // 单行选中
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    //// 单选
    //tableView->setSelectionMode(QAbstractItemView::NoSelection);
    // 不可以编辑
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 形状
    //tableView->setFrameShape(QTableView::NoFrame);
    // 栅格
    tableView->setShowGrid(false);
    // 排序
    tableView->setSortingEnabled(true);
    tableView->setContentsMargins(0, 0, 0, 0);
 
    // view settings
    //tableView->setStyle(QStyleFactory::create("Fusion"));

    tableView->setItemDelegate(checkBoxDelegate);

    connect(tableView, &QTableView::doubleClicked, this, &FileWidget::onFileDoubleClicked);
}

void FileWidget::initThumbnailView()
{
    if (nullptr != thumbnailView)
    {
        return;
    }
    thumbnailDelegate = new ThumbnailDelegate(this->imageCore, this);

    thumbnailView = new QListView(this);
    thumbnailView->setContentsMargins(0, 0, 0, 0);
    thumbnailView->setStyleSheet("background-color: transparent;border:1px solid #EFEFEF;");
    // 为视图设置委托
    thumbnailView->setItemDelegate(thumbnailDelegate);
    // 为视图设置控件间距
    thumbnailView->setSpacing(1);
    // 设置Item图标显示
    thumbnailView->setViewMode(QListView::IconMode);
    thumbnailView->setResizeMode(QListView::Adjust);
    thumbnailView->setMovement(QListView::Static);
}

void FileWidget::initWidgetLayout() {
    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    this->setContentsMargins(0, 0, 0, 0);
    this->setLayout(vLayout);

    stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(tableView);
    stackedWidget->addWidget(thumbnailView);

    vLayout->addWidget(toolBar);
    vLayout->addWidget(stackedWidget);
}

void FileWidget::cdPath(const QString& path)
{
    if (path.isEmpty())
        return;
    updateCurrentPath(path);
    emit onCdDir(path);
}

void FileWidget::updateCurrentPath(const QString& path)
{
    initListModel(path, false);

    if (FileViewType::Table == fileViewType)
    {
        stackedWidget->setCurrentIndex(0);
    }
    else if (FileViewType::Thumbnail == fileViewType)
    {
        DWORD start = GetTickCount();
        thumbnailView->setUpdatesEnabled(false);
        setThumbnailView(path);
        thumbnailView->setUpdatesEnabled(true);
        LOG_INFO << " thumbnailView append rows time: " << GetTickCount() - start;
        stackedWidget->setCurrentIndex(1);
    }
    currentPath = path;
}

void FileWidget::initListModel(const QString& path, bool readPixmap) {
    if (nullptr == fileListModel)
    {
        fileListModel = new FileListModel(ensureIconProvider());
        fileListModel->setColumnCount(NumberOfColumns);

        proxyModel = new FileFilterProxyModel;
        proxyModel->setSourceModel(fileListModel);

        thumbnailView->setModel(proxyModel);
        tableView->setModel(proxyModel);
        // need after setModel 
        connect(thumbnailView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileWidget::onSelectionChanged);
        connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileWidget::onSelectionChanged);
    }
    else {
        fileListModel->clear();
    }

    if (FileViewType::Table == fileViewType)
    {
        fileListModel->setHorizontalHeaderLabels(QStringList{ tr(""),tr("Name")/*, tr("Ext")*/, tr("Size"), tr("Date") });
    }

    QList<QFileInfo> fileInfos = getRowItemList(path);
    if (fileInfos.isEmpty())
    {
        return;
    }
    int itemRow = 0;
    for (const auto& fileInfo : fileInfos)
    {
        auto checkBoxItem = new QStandardItem();
        checkBoxItem->setData(QVariant::fromValue(fileInfo), Qt::UserRole + 3);
        checkBoxItem->setData(Qt::CheckState::Unchecked, Qt::CheckStateRole);
        fileListModel->setItem(itemRow, CheckBoxColumn, checkBoxItem);

        auto fileNameItem = new QStandardItem();
        fileNameItem->setIcon(ensureIconProvider()->icon(fileInfo));
        fileNameItem->setData(fileInfo.fileName(), Qt::DisplayRole);
        fileListModel->setItem(itemRow, NameColumn, fileNameItem);

        //auto fileExtItem = new QStandardItem();
        //fileExtItem->setData(fileInfo.suffix(), Qt::DisplayRole);
        //fileListModel->setItem(itemRow, ExtColumn, fileExtItem);

        auto sizeItem = new QStandardItem();
        sizeItem->setData(fileSizeToString(fileInfo.size()), Qt::DisplayRole);
        fileListModel->setItem(itemRow, SizeColumn, sizeItem);

        auto dateItem = new QStandardItem();
        dateItem->setData(fileInfo.lastModified(), Qt::DisplayRole);
        fileListModel->setItem(itemRow, DateColumn, dateItem);
        itemRow++;
    }
}

void FileWidget::setThumbnailView(const QString& path, bool readPixmap)
{
    LOG_INFO << " setThumbnailView dir: " << path << " initModel:" << readPixmap;

    QList<QStandardItem*> itemInfos = getRowItemList();
    if (itemInfos.isEmpty())
    {
        return;
    }
    int itemRow = 0;
    //QThreadPool::globalInstance()->setMaxThreadCount(12);
    std::function<QStandardItem* (QStandardItem*)> getThumbnailItem = [this, &readPixmap](QStandardItem* rowItem) -> QStandardItem*
    {
        auto thumbnailItem = rowItem;
        QVariant variant = thumbnailItem->data(Qt::UserRole + 3);
        if (variant.isNull())
        {
            return rowItem;
        }
        auto fileInfo = variant.value<QFileInfo>();

        auto itemData = new ThumbnailData;;
        itemData->fileName = fileInfo.fileName();
        itemData->absoluteFilePath = fileInfo.absoluteFilePath();
        itemData->extension = fileInfo.suffix();
        //itemData->size = fileInfo.size();
        //itemData->lastModified = fileInfo.lastModified();

        itemData->isWeChatImage = this->imageCore->isWeChatImage(itemData->extension, itemData->fileName);

        if (readPixmap)
        {
            if (this->imageCore->isImageFile(fileInfo))
            {
                //std::lock_guard<std::recursive_mutex> locker(_fileListAndCurrentDirMutex);
                ImageCore::ReadData image = imageCore->readFile(fileInfo.absoluteFilePath(), true);
                itemData->thumbnail = image.pixmap;
            }
            else {
                std::lock_guard<std::recursive_mutex> locker(_fileListAndCurrentDirMutex);
                //auto* iconProvider = (QFileIconProvider*)fileSystemMode->iconProvider();
                auto icon = ensureIconProvider()->icon(fileInfo);
                itemData->thumbnail = icon.pixmap(ICON_WIDE, ICON_HEIGHT);
            }
        }

        thumbnailItem->setData(QVariant::fromValue(*itemData), Qt::UserRole + 3);
        return thumbnailItem;
    };

    QFuture<QStandardItem*> future = QtConcurrent::mapped(itemInfos, getThumbnailItem);
    future.waitForFinished();
    //QList<QStandardItem*> items = future.results();

    //for (auto &item : items)
    //{
    //    //thumbnailModel->appendRow(item);
    //    thumbnailModel->appendRow(item);
    //}
}

void FileWidget::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    if (selected == deselected)
    {
        return;
    }
    QModelIndexList selectList = selected.indexes();
    if (selectList.isEmpty())
    {
        return;
    }

    QFileInfo info = proxyModel->fileInfo(FileViewType::Table == fileViewType ? selectList.last().siblingAtColumn(0) : selectList.last());
    LOG_INFO << " onSelectionChanged fileInfo: " << info;
    if (FileViewType::Thumbnail == fileViewType)
    {
        info = this->fileListModel->fileInfo(selectList.last());
        LOG_INFO << " fileListModel fileInfo: " << info;
    }
 
    if (info.isFile()) {
        this->imageCore->loadFile(info.absoluteFilePath(), QSize(THUMBNAIL_WIDE_N,THUMBNAIL_HEIGHT_N));
    }
}

void FileWidget::thumbnail()
{
    if (this->fileViewType != FileViewType::Thumbnail)
    {
        this->fileViewType = FileViewType::Thumbnail;
        updateCurrentPath(this->currentPath);
    }
}

void FileWidget::detail()
{
    if (this->fileViewType != FileViewType::Table)
    {
        this->fileViewType = FileViewType::Table;
        updateCurrentPath(this->currentPath);
    }
}

void FileWidget::selectAll()
{
    if (this->fileViewType == FileViewType::Thumbnail)
    {
        if (nullptr == fileListModel)
        {
            return;
        }
        for (int r = 0; r < this->fileListModel->rowCount(); ++r)
        {
            auto item = fileListModel->item(r);
            QVariant variant = item->data(Qt::UserRole + 3);
            if (variant.isNull())
            {
                break;
            }
            ThumbnailData data = variant.value<ThumbnailData>();
            if (data.extension == "dat" && data.fileName.length() == 36)
            {
                // wechat
                item->setCheckState(Qt::CheckState::Checked);
            }
        }
    }
}

void FileWidget::onFileDoubleClicked(const QModelIndex& index)
{
    QString target;
    
    QFileInfo info = this->proxyModel->fileInfo(index.siblingAtColumn(0));
    LOG_INFO << " onFileDoubleClicked info: " << info;
    if (info.isShortcut()) {
        // handle shortcut
        if (!info.exists()) {
            return;
        }
        target = info.symLinkTarget();
    }
    else {
        target = info.absoluteFilePath();
    }

    // If the file is a symlink, this function returns true if the target is a directory (not the symlink)
    if (info.isDir()) {
        cdPath(target);
        return;
    }

    if (this->fileListModel == nullptr || this->fileListModel->rowCount() <= 0)
    {
        setThumbnailView(info.path(), false);
    }
    else if (info.path() != this->currentPath)
    {
        setThumbnailView(info.path(), this->fileViewType == FileViewType::Thumbnail);
    }

    QStandardItem* item = nullptr;
    for( int r = 0; r < this->fileListModel->rowCount(); ++r)
    {
        item = fileListModel->item(r);
        QVariant variant = item->data(Qt::UserRole + 3);
        if (variant.isNull())
        {
            break;
        }
        ThumbnailData data = variant.value<ThumbnailData>();
        if (data.absoluteFilePath == info.absoluteFilePath())
        {
            break;
        }
    }
    auto* switcher = new ImageSwitcher(item, fileListModel);
    Slideshow* slideshow = new Slideshow(this->imageCore, switcher);
    slideshow->show();
}

void FileWidget::onTreeViewClicked(const QString& path)
{
    cdPath(path);
}

QList<QFileInfo> FileWidget::getRowItemList(const QString& currentDirPath)
{
    QList<QFileInfo> list;
    //{
        //std::lock_guard<std::recursive_mutex> locker(_fileListAndCurrentDirMutex);
        list = QDir{ currentDirPath }.entryInfoList(
            QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDir::NoSort);
    //}
    return list;

}

QList<QStandardItem*> FileWidget::getRowItemList()
{
    QList<QStandardItem*> list;
    int row = this->fileListModel->rowCount();
    for (int r = 0; r < row; ++r) {
        QStandardItem* item = this->fileListModel->item(r);
        if (item)
        {
            list.append(item);
        }
    }
    return list;
}

