#include "filewidget.h"
#include "config.h"
#include "filelistmodel/filefilterproxymodel.h"
#include "delegate/thumbnailDelegate.h"
#include "delegate/thumbnailData.h"
#include "logger/Logger.h"
#include "imageViewer.h"
#include "models/imageswitcher.h"
#include "delegate/checkBoxDelegate.h"
#include "filelistmodel/filelistmodel.h"
#include "config.h"
#include "iconhelper.h"

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


FileWidget::FileWidget(ImageCore* imageCore, QWidget* parent) : 
    QWidget(parent), _imageCore(imageCore) {

    this->m_iconProvider = nullptr;
    this->thumbnailModel = nullptr;
    this->fileListModel = nullptr;
    this->proxyModel = nullptr;

    this->fileViewType = FileViewType::Table;

    // widget init
    setupToolBar();
    tableView = nullptr;
    thumbnailView = nullptr;
    thumbnailDelegate = nullptr;
    checkBoxDelegate = nullptr;
    initListView();
    initWidgetLayout();
    loadFileListInfo();
}

FileWidget::~FileWidget() {
    saveFileListInfo();
    if (nullptr != thumbnailDelegate)
    {
        delete thumbnailDelegate;
    }
    if (nullptr != checkBoxDelegate)
    {
        delete checkBoxDelegate;
    }
    delete m_iconProvider;
   
    tableView->deleteLater();
    thumbnailView->deleteLater();
    proxyModel->deleteLater();
}

void FileWidget::setupToolBar() {
    toolBar = new QToolBar;
    //toolBar->setStyleSheet("background-color:rgb(0, 200, 0);}");
    toolBar->setContentsMargins(0, 0, 0, 0);

    IconHelper::StyleColor styleColor;

    QAction* detailAction = toolBar->addAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 61498, 16, 16, 16)), tr("detail"));
    connect(detailAction, &QAction::triggered, this, &FileWidget::detail);

    QAction* thumbnailAction = toolBar->addAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 57750, 16, 16, 16)), tr("thumbnail"));
    connect(thumbnailAction, &QAction::triggered, this, &FileWidget::thumbnail);

    toolBar->addSeparator();

    QAction* selectAllAction = toolBar->addAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 61528, 16, 16, 16)), tr("select all"));
    connect(selectAllAction, &QAction::triggered, this, &FileWidget::selectAll);

    QAction* exportAction = toolBar->addAction(QIcon(IconHelper::getInstance().getPixmap(styleColor.normalBgColor, 62830, 16, 16, 16)), tr("export"));
    connect(exportAction, &QAction::triggered, this, &FileWidget::exportSelected);

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
    tableView->horizontalHeader()->setContentsMargins(0, 0, 0, 1);
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
    thumbnailDelegate = new ThumbnailDelegate(this->_imageCore, this);

    thumbnailView = new QListView(this);
    thumbnailView->setContentsMargins(0, 0, 0, 0);
    //thumbnailView->setStyleSheet("background-color: transparent;border:1px solid #EFEFEF;");
    // 为视图设置委托
    thumbnailView->setItemDelegate(thumbnailDelegate);
    // 为视图设置控件间距
    thumbnailView->setSpacing(1);
    // 设置Item图标显示
    thumbnailView->setViewMode(QListView::IconMode);
    thumbnailView->setResizeMode(QListView::Adjust);
    thumbnailView->setMovement(QListView::Static);
    connect(thumbnailView, &QListView::doubleClicked, this, &FileWidget::onFileDoubleClicked);
}

void FileWidget::initWidgetLayout() {
    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    this->setContentsMargins(0, 0, 0, 0);
    this->setLayout(vLayout);

    stackedWidget = new QStackedWidget(this);
    stackedWidget->setContentsMargins(0, 0, 0, 0);
    stackedWidget->addWidget(tableView);
    stackedWidget->addWidget(thumbnailView);

    vLayout->addWidget(toolBar);
    vLayout->addWidget(stackedWidget);
}

void FileWidget::cdPath(const QString& path)
{
    currentPath = path;
    //QFuture<void> fut = QtConcurrent::run(&FileWidget::initListModel, this, path, false);
    //fut.waitForFinished();
    initListModel(path, false);
    onUpdateItems();
}

void FileWidget::initListModel(const QString& path, bool readPixmap) {
    if (nullptr == fileListModel)
    {
        fileListModel = new FileListModel(this->_imageCore, ensureIconProvider());
        fileListModel->setColumnCount(NumberOfColumns);

        proxyModel = new FileFilterProxyModel;
   
        thumbnailView->setModel(proxyModel);
        tableView->setModel(proxyModel);

        tableView->sortByColumn(this->_sortColumn, Qt::SortOrder(this->_sortOrder));
        tableView->setAlternatingRowColors(true);
        //tableView->setFont(QFont("Fixedsys", 8));
        //connect(thumbnailView->selectionModel(), &QItemSelectionModel::currentChanged, this, &FileWidget::onCurrentChanged);
        //connect(tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &FileWidget::onCurrentChanged);
        //connect(this->fileListModel, &FileListModel::onUpdateItems, this, &FileWidget::onUpdateItems);
    }
    QList<QFileInfo> fileInfos = getRowItemList(path);

    DWORD start = GetTickCount();
    disconnect(thumbnailView->selectionModel(), &QItemSelectionModel::currentChanged, this, &FileWidget::onCurrentChanged);
    disconnect(tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &FileWidget::onCurrentChanged);
    proxyModel->setSourceModel(nullptr);
    this->fileListModel->updateItems(fileInfos);
    proxyModel->setSourceModel(fileListModel);
    // must after setModel 
    connect(thumbnailView->selectionModel(), &QItemSelectionModel::currentChanged, this, &FileWidget::onCurrentChanged);
    connect(tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &FileWidget::onCurrentChanged);
    tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    LOG_INFO << "updateItems time: " << GetTickCount() - start;
}

void FileWidget::onUpdateItems()
{
    this->tableView->sortByColumn(this->proxyModel->getSortColumn(), this->proxyModel->sortOrder());
    //this->tableView->viewport()->update();

    if (FileViewType::Table == fileViewType)
    {
        stackedWidget->setCurrentIndex(0);
    }
    else if (FileViewType::Thumbnail == fileViewType)
    {
        DWORD start = GetTickCount();
        thumbnailView->setUpdatesEnabled(false);
        setThumbnailView(currentPath);
        thumbnailView->setUpdatesEnabled(true);
        LOG_INFO << "thumbnailView append rows time: " << GetTickCount() - start;
        stackedWidget->setCurrentIndex(1);
    }
    setTableColWidth();
}

void FileWidget::setTableColWidth()
{
    tableView->setColumnWidth(0, 18);
    tableView->setColumnWidth(1, this->_column1w);
    tableView->setColumnWidth(2, 90);
    tableView->setColumnWidth(3, 137);
}

void FileWidget::setThumbnailView(const QString& path, bool readPixmap)
{
    LOG_INFO << "setThumbnailView path: " << path << " readPixmap:" << readPixmap;

    QList<QStandardItem*> itemInfos = getRowItemList();
    if (itemInfos.isEmpty())
    {
        return;
    }
    QFuture<QStandardItem*> future = QtConcurrent::mapped(itemInfos, [this, &readPixmap](QStandardItem* rowItem) -> QStandardItem*
        {
            QVariant variant = rowItem->data(Qt::UserRole + 3);
            if (variant.isNull())
            {
                return rowItem;
            }

            auto itemData = variant.value<ThumbnailData>();
            itemData.isWeChatImage = this->_imageCore->isWeChatImage(itemData.fileInfo);

            if (readPixmap)
            {
                if (this->_imageCore->isImageFile(itemData.fileInfo))
                {
                    ImageReadData image = _imageCore->readFile(itemData.fileInfo.absoluteFilePath(), true);
                    itemData.thumbnail = image.pixmap;
                }
                else {
                    std::lock_guard<std::recursive_mutex> locker(fileIconMutex);
                    auto icon = ensureIconProvider()->icon(itemData.fileInfo);
                    itemData.thumbnail = icon.pixmap(ICON_WIDE, ICON_HEIGHT);
                }
            }
            rowItem->setData(QVariant::fromValue(itemData), Qt::UserRole + 3);
            return rowItem;
        });
    future.waitForFinished();
}

void FileWidget::thumbnail()
{
    if (this->fileViewType != FileViewType::Thumbnail)
    {
        this->fileViewType = FileViewType::Thumbnail;
        onUpdateItems();
    }
}

void FileWidget::detail()
{
    if (this->fileViewType != FileViewType::Table)
    {
        this->fileViewType = FileViewType::Table;
        onUpdateItems();
    }
}

void FileWidget::selectAll()
{
    if (nullptr == fileListModel)
    {
        return;
    }

    for (int r = 0; r < this->fileListModel->rowCount(); ++r)
    {
        auto item = fileListModel->item(r);
        auto fileInfo = fileListModel->fileInfo(item);
        
        if (this->_imageCore->isWeChatImage(fileInfo))
        {
            // wechat
            item->setCheckState(Qt::CheckState::Checked);
        }
    }
}

void FileWidget::exportSelected()
{
    QList<QFileInfo> selects;
    for (int r = 0; r < this->fileListModel->rowCount(); ++r)
    {
        auto item = fileListModel->item(r);
       
        if (item->checkState() == Qt::CheckState::Checked)
        {
            selects.append(fileListModel->fileInfo(item));
        }
    }
    if (!selects.isEmpty())
    {
        QString directory = QFileDialog::getExistingDirectory(this, tr("open directory"), QDir::currentPath());
        if (directory != "")
        {
            QFuture<bool> future = QtConcurrent::mapped(selects, [this, &directory](const QFileInfo& fileInfo) -> bool {
                const ImageReadData& readData = this->_imageCore->readFile(fileInfo.absoluteFilePath(), true, QSize());
                QString file = directory + QDir::separator() + fileInfo.baseName() + "." + readData.suffix;
                LOG_INFO << "export file: " << file;
                return readData.pixmap.save(file);
                });
            future.waitForFinished();
        }
    }
}

void FileWidget::onCurrentChanged(const QModelIndex& current, const QModelIndex& previous) {
    QFileInfo info = proxyModel->fileInfo(current.siblingAtColumn(0));
    LOG_INFO << "onCurrentChanged fileInfo: " << info;
    if (info.isFile() && this->_imageCore->isImageFile(info)) {
        this->_imageCore->loadFile(info.absoluteFilePath(), QSize(THUMBNAIL_WIDE_N, THUMBNAIL_HEIGHT_N));
    }
    else {
        emit this->_imageCore->imageLoaded(nullptr);
    }
}

void FileWidget::onFileDoubleClicked(const QModelIndex& index)
{
    QString target;
    QModelIndex clicked = index.siblingAtColumn(0);
    QFileInfo info = this->proxyModel->fileInfo(clicked);
    LOG_INFO << "onFileDoubleClicked info: " << info;
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
        emit cdDir(target);
        return;
    }
    if (this->_imageCore->isImageFile(info))
    {
        ImageViewer* slideshow = new ImageViewer(this->_imageCore, new ImageSwitcher(clicked, this->proxyModel));
        slideshow->show();
    }
}

void FileWidget::onTreeViewClicked(const QString& path)
{
    if (path.isEmpty())
        return;
    cdPath(path);
}

QList<QFileInfo> FileWidget::getRowItemList(const QString& currentDirPath)
{
    QList<QFileInfo> list;
    list = QDir{ currentDirPath }.entryInfoList(
        QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDir::NoSort);
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

void FileWidget::loadFileListInfo()
{
    _sortColumn = ConfigIni::getInstance().iniRead(QStringLiteral("FileList/sortColumn"), "-1").toInt();
    _sortOrder = ConfigIni::getInstance().iniRead(QStringLiteral("FileList/sortOrder"), "0").toInt();
    _column1w = ConfigIni::getInstance().iniRead(QStringLiteral("FileList/column1w"), "256").toInt();
    if (_column1w <= 0)
    {
        _column1w = 256;
    }
}

void FileWidget::saveFileListInfo()
{
    ConfigIni::getInstance().iniWrite(QStringLiteral("FileList/sortColumn"), this->proxyModel->sortColumn());
    ConfigIni::getInstance().iniWrite(QStringLiteral("FileList/sortOrder"), this->proxyModel->sortOrder());
    ConfigIni::getInstance().iniWrite(QStringLiteral("FileList/column1w"), this->tableView->columnWidth(1));
}

