#include "filewidget.h"
#include "config.h"

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
    auto *fileModel = new QStandardItemModel;
    proxyModel = new FileFilterProxyModel;
    proxyModel->setSourceModel(fileModel);

    // widget init
    setupToolBar();
    initListView();
    initWidgetLayout();
}

FileWidget::~FileWidget() {
    proxyModel->deleteLater();
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
    listView->setModel(proxyModel);

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
    QString currPath = QDir::toNativeSeparators(dir);
    current_path = currPath;
    QList<QFileInfo> list = setItems(currPath);
    if (!list.isEmpty())
    {
        fillFromList(list);
    }
}

void FileWidget::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {

}

void FileWidget::fillFromList(const QList<QFileInfo>& items)
{
    QStandardItemModel* data = (QStandardItemModel*)this->proxyModel->sourceModel();
    data->clear();
    int itemRow = 0;
    if (this->listView->viewMode() == QListView::ListMode)
    {
        for (const auto& item : items) {
            auto fileNameItem = new QStandardItem();
            fileNameItem->setEditable(false);
            
            fileNameItem->setData(item.fileName(),Qt::DisplayRole);
            
            data->appendRow(fileNameItem);
        }
    }
}

QList<QFileInfo> FileWidget::setItems(const QString& currentDirPath)
{
    QList<QFileInfo> list;
    {
        std::lock_guard<std::recursive_mutex> locker(_fileListAndCurrentDirMutex);

        list = QDir{ currentDirPath }.entryInfoList(
            QDir::Dirs | QDir::Files | QDir::NoDot | QDir::Hidden | QDir::System, QDir::NoSort);
    }
    return list;

}

void FileWidget::thumbnail()
{
    //QAbstractItemModel *data = this->proxyModel->sourceModel();
    
    this->listView->setViewMode(QListView::IconMode);
    //this->listView->setIconSize(QSize(120, 120));
    //this->listView->setAlternatingRowColors(false);
    //this->listView->viewport()->setAttribute(Qt::WA_StaticContents);
    //this->listView->setAttribute(Qt::WA_MacShowFocusRect, false);
    QModelIndex index = proxyModel->proxyIndex(current_path);
    listView->setRootIndex(index);
}

void FileWidget::detail()
{
    this->listView->setViewMode(QListView::ListMode);
}

void FileWidget::onTreeViewClicked(const QModelIndex &index) {
    QString target;
    QFileInfo info = proxyModel->fileInfo(index);

    if (info.isShortcut()) {
        // handle shortcut
        if (!info.exists()) {
            QModelIndex srcIndex = proxyModel->mapToSource(index);
            ((FileSystemModel*)proxyModel->srcModel())->showShortcutInfo(srcIndex);
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


void FileWidget::onItemActivated(const QString &path) {
//    if (!QFileInfo::exists(path)) {   // shortcut maybe not exist
//        return;
//    }
    QModelIndex index = proxyModel->proxyIndex(path);
    onTreeViewClicked(index);
}

void FileWidget::onNavigateBarClicked(const QString& path)
{
    cdPath(path);
}

