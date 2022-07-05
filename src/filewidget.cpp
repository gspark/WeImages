#include "filewidget.h"
#include "config.h"
#include "filelistmodel/filefilterproxymodel.h"
#include "delegate/thumbnailDelegate.h"
#include "delegate/thumbnailData.h"
#include "logger/Logger.h"
#include "slideshow.h"

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

FileWidget::FileWidget(QAbstractItemModel* model, ImageCore* imageCore, QWidget* parent) : QWidget(parent) {
 
    this->imageCore = imageCore;
    this->thumbnailModel = nullptr;

    this->fileViewType = ::FileViewType::Table;

    // init file model
    fileSystemMode = (QFileSystemModel*)model;

    proxyModel = new FileFilterProxyModel;
    proxyModel->setSourceModel(fileSystemMode);

    // widget init
    setupToolBar();
    tableView = nullptr;
    thumbnailView = nullptr;
    initListView();
    initWidgetLayout();
}

FileWidget::~FileWidget() {
    proxyModel->deleteLater();
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
    tableView = new QTableView;
    tableView->verticalHeader()->setVisible(false);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // 单选
    tableView->setSelectionMode(QAbstractItemView::NoSelection);
    // 不可以编辑
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 形状
    //tableView->setFrameShape(QTableView::NoFrame);
    // 栅格
    tableView->setShowGrid(false);
    // 排序
    tableView->setSortingEnabled(true);
    tableView->setContentsMargins(0, 0, 0, 0);
    tableView->setModel(proxyModel);
    // view settings
    //tableView->setStyle(QStyleFactory::create("Fusion"));

    connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileWidget::onSelectionChanged);

    connect(tableView, &QTableView::doubleClicked, this, &FileWidget::onFileDoubleClicked);
}

void FileWidget::initThumbnailView()
{
    if (nullptr != thumbnailView)
    {
        return;
    }
    m_delegate = new ThumbnailDelegate(this->imageCore, this);

    thumbnailView = new QListView(this);
    thumbnailView->setContentsMargins(0, 0, 0, 0);
    thumbnailView->setStyleSheet("background-color: transparent;border:1px solid #EFEFEF;");
    // 为视图设置委托
    thumbnailView->setItemDelegate(m_delegate);
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

void FileWidget::updateCurrentPath(const QString& dir)
{
    if (FileViewType::Table == fileViewType)
    {
        QModelIndex index = proxyModel->proxyIndex(dir);
        this->tableView->setRootIndex(index);
        stackedWidget->setCurrentIndex(0);
    }
    else if (FileViewType::Thumbnail == fileViewType)
    {
        DWORD start = GetTickCount();
        setThumbnailView(dir);
        LOG_INFO << " thumbnailView append rows time: " << GetTickCount() - start;
        stackedWidget->setCurrentIndex(1);
    }
}

void FileWidget::setThumbnailView(const QString& dir, bool readPixmap)
{
    LOG_INFO << " setThumbnailView dir: " << dir << " initModel:" << readPixmap;
    currentDirPath = dir;

    if (nullptr == thumbnailModel)
    {
        thumbnailModel = new QStandardItemModel;
        thumbnailView->setModel(this->thumbnailModel);
    }
    else {
        thumbnailModel->clear();
    }
    QList<QFileInfo> fileInfos = getFileInfoList(dir);
    if (fileInfos.isEmpty())
    {
        return;
    }
    int itemRow = 0;
    //QThreadPool::globalInstance()->setMaxThreadCount(12);
    std::function<QStandardItem* (const QFileInfo&)> getThumbnailItem = [this, &readPixmap](const QFileInfo& fileInfo) -> QStandardItem*
    {
        //std::lock_guard<std::recursive_mutex> locker(_fileListAndCurrentDirMutex);
        auto thumbnailItem = new QStandardItem();
        //thumbnailItem->setEditable(false);
        auto itemData = new ThumbnailData;;
        itemData->fileName = fileInfo.fileName();
        itemData->absoluteFilePath = fileInfo.absoluteFilePath();
        itemData->extension = fileInfo.suffix();
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
                auto* iconProvider = (QFileIconProvider*)fileSystemMode->iconProvider();
                auto icon = iconProvider->icon(fileInfo);
                itemData->thumbnail = icon.pixmap(ICON_WIDE, ICON_HEIGHT);
            }
        }

        thumbnailItem->setData(QVariant::fromValue(*itemData), Qt::UserRole + 3);
        return thumbnailItem;
    };

    QFuture<QStandardItem*> future = QtConcurrent::mapped(fileInfos, getThumbnailItem);
    future.waitForFinished();
    QList<QStandardItem*> items = future.results();

    thumbnailView->setUpdatesEnabled(false);
    for (const auto item : items) {
        thumbnailModel->appendRow(item);
    }
    thumbnailView->setUpdatesEnabled(true);
}

void FileWidget::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    QModelIndexList selectList = selected.indexes();
    if (selectList.isEmpty())
    {
        return;
    }

    QFileInfo info = proxyModel->fileInfo(selectList.last());

    if (info.isFile()) {
        this->imageCore->loadFile(info.absoluteFilePath(), QSize(THUMBNAIL_WIDE_N,THUMBNAIL_HEIGHT_N));
    }
}

void FileWidget::thumbnail()
{
    if (this->fileViewType != FileViewType::Thumbnail)
    {
        this->fileViewType = FileViewType::Thumbnail;
        updateCurrentPath(proxyModel->srcModel()->rootPath());
    }
}

void FileWidget::detail()
{
    if (this->fileViewType != FileViewType::Table)
    {
        this->fileViewType = FileViewType::Table;
        updateCurrentPath(proxyModel->srcModel()->rootPath());
    }
}

void FileWidget::selectAll()
{
    if (this->fileViewType == FileViewType::Thumbnail)
    {
        if (nullptr == thumbnailModel)
        {
            return;
        }
        for (int r = 0; r < this->thumbnailModel->rowCount(); ++r)
        {
            auto item = thumbnailModel->item(r);
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
    QFileInfo info = proxyModel->fileInfo(index);
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

    if (this->thumbnailModel == nullptr || this->thumbnailModel->rowCount() <= 0)
    {
        setThumbnailView(info.path(), false);
    }
    else if (info.path() != this->currentDirPath)
    {
        setThumbnailView(info.path(), this->fileViewType == FileViewType::Thumbnail);
    }

    QStandardItem* item = nullptr;
    for( int r = 0; r < this->thumbnailModel->rowCount(); ++r)
    {
        item = thumbnailModel->item(r);
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
    auto* switcher = new ImageSwitcher(item, thumbnailModel);
    Slideshow* slideshow = new Slideshow(this->imageCore, switcher);
    slideshow->show();
}

void FileWidget::onTreeViewClicked(const QString& path)
{
    cdPath(path);
}

QList<QFileInfo> FileWidget::getFileInfoList(const QString& currentDirPath)
{
    QList<QFileInfo> list;
    //{
        //std::lock_guard<std::recursive_mutex> locker(_fileListAndCurrentDirMutex);

        list = QDir{ currentDirPath }.entryInfoList(
            QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDir::NoSort);
    //}
    return list;

}
