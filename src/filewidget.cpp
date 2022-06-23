#include "filewidget.h"
#include "config.h"
#include "filelistmodel/filefilterproxymodel.h"
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

inline wchar_t* appendToString(wchar_t* buffer, const wchar_t* what, size_t whatLengthInCharacters = 0)
{
    if (whatLengthInCharacters == 0)
        whatLengthInCharacters = wcslen(what);

    memcpy(buffer, what, whatLengthInCharacters * sizeof(wchar_t));
    return buffer + whatLengthInCharacters;
}

inline wchar_t* appendToString(wchar_t* buffer, const QString& what)
{
    const auto written = what.toWCharArray(buffer);
    return buffer + written;
}

FileWidget::FileWidget(QAbstractItemModel* model, ImageCore* imageCore, QWidget* parent) : QWidget(parent) {
 
    this->imageCore = imageCore;

    this->fileViewType = ::FileViewType::List;

    // init file model
    auto* fileModel = (QFileSystemModel*)model;

    proxyModel = new FileFilterProxyModel;
    proxyModel->setSourceModel(fileModel);

    // widget init
    setupToolBar();
    listView = nullptr;
    thumbnailView = nullptr;
    initListView();
    initWidgetLayout();
}

FileWidget::~FileWidget() {
    proxyModel->deleteLater();
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
    initTableView();
    initThumbnailView();
}

void FileWidget::initTableView()
{
    if (nullptr != listView)
    {
        return;
    }
    listView = new QTableView;
    listView->setContentsMargins(0, 0, 0, 0);
    listView->setModel(proxyModel);

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
    thumbnailView->setSpacing(1);
    thumbnailView->setViewMode(QListView::IconMode);
 /*   thumbnailView->setIconSize(QSize(210, 210));*/
    thumbnailView->setResizeMode(QListView::Adjust);
    thumbnailView->setMovement(QListView::Static);
    thumbnailView->setModel(proxyModel);
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
    QModelIndex index = proxyModel->proxyIndex(dir);

    if (FileViewType::List == fileViewType)
    {
        this->listView->setRootIndex(index);
        stackedWidget->setCurrentIndex(0);
    }
    else if (FileViewType::thumbnail == fileViewType)
    {
        //QFileSystemModel* data = (QFileSystemModel*)this->proxyModel->srcModel();
        //data->setRootPath(dir);
        thumbnailView->setUpdatesEnabled(false);
        thumbnailView->setRootIndex(index);
        //setThumbnailView(data);
        thumbnailView->setUpdatesEnabled(true);
        stackedWidget->setCurrentIndex(1);
    }
}

void FileWidget::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {

}

void FileWidget::thumbnail()
{
    if (this->fileViewType != FileViewType::thumbnail)
    {
        this->fileViewType = FileViewType::thumbnail;
        updateCurrentPath(proxyModel->srcModel()->rootPath());
    }
    //QAbstractItemModel *data = this->proxyModel->sourceModel();

    //this->thumbnailView->setViewMode(QListView::IconMode);
    //this->listView->setIconSize(QSize(120, 120));
    //this->listView->setAlternatingRowColors(false);
    //this->listView->viewport()->setAttribute(Qt::WA_StaticContents);
    //this->listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    //setThumbnailView(this->proxyModel->sourceModel());
    
}

void FileWidget::detail()
{
    if (this->fileViewType != FileViewType::List)
    {
        this->fileViewType = FileViewType::List;
        updateCurrentPath(proxyModel->srcModel()->rootPath());
    }
}

void FileWidget::onTreeViewClicked(const QModelIndex& index) {
    QString target;
    QFileInfo info = proxyModel->fileInfo(index);

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

