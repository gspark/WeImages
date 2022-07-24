#include "navdockwidget.h"
#include "config.h"
#include "imagecore.h"
#include "filelistmodel/filefilterproxymodel.h"

#include <QStyleFactory>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QTreeView>
#include <QHeaderView>
#include <QFileSystemModel>


NavDockWidget::NavDockWidget(QAbstractItemModel *model, ImageCore* imageCore)
{
    this->imageCore = imageCore;
    fileModel = (QFileSystemModel*)model;
    
    proxyModel = new FileFilterProxyModel;
    treeView = new QTreeView;

    thumbnail = new QLabel;
    thumbnail->setAlignment(Qt::AlignCenter);
    thumbnail->setVisible(false);
    //thumbnail->setScaledContents(true);
    thumbnail->setFrameStyle(QFrame::StyledPanel);

    fileModelInit();
    treeViewInit();

    auto* widget = new QWidget(this);
    auto* vLayout = new QVBoxLayout(widget);
    vLayout->setContentsMargins(1, 0, 0, 0);
    vLayout->addWidget(treeView);
    vLayout->addWidget(thumbnail);

    this->setWidget(widget);
  
    connect(imageCore, &ImageCore::imageLoaded, this, &NavDockWidget::imageLoaded);
}

NavDockWidget::~NavDockWidget()
{
    treeView->deleteLater();
    proxyModel->deleteLater();
    thumbnail->deleteLater();
    fileModel = nullptr;
}

QSize NavDockWidget::sizeHint() const
{
    return QSize(220, -1);
}

void NavDockWidget::fileModelInit()
{
    proxyModel->setSourceModel(fileModel);
    proxyModel->enableFilter(true);
}

void NavDockWidget::treeViewInit()
{
    treeView->setModel(proxyModel);

    treeView->setSortingEnabled(true);
    treeView->sortByColumn(0, Qt::AscendingOrder);

    connect(treeView, &QTreeView::clicked, this, &NavDockWidget::onTreeViewClicked);

    // view settings
    //treeView->setStyle(QStyleFactory::create("Fusion"));
    treeView->hideColumn(1); 
    treeView->hideColumn(2);
    treeView->hideColumn(3);
 
    // header settings
    treeView->header()->hide();
    treeView->header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    treeView->header()->setStretchLastSection(false);
    treeView->setFont(QFont("Tahoma", 9));
}

void NavDockWidget::onTreeViewClicked(const QModelIndex &index)
{
    QFileInfo info = proxyModel->fileInfo(index);
    if (info.isDir()) {
        emit treeViewClicked(info.absoluteFilePath());
    }
}

void NavDockWidget::onSetPath(const QString path)
{
    QModelIndex index = this->proxyModel->proxyIndex(path);
    this->treeView->setCurrentIndex(index);
    emit treeViewClicked(path);
    this->treeView->scrollTo(index);
}

void NavDockWidget::onCdDir(const QString path)
{
    QModelIndex index = this->proxyModel->proxyIndex(path);
    this->treeView->setCurrentIndex(index);
}

void NavDockWidget::imageLoaded(ImageReadData* readData)
{
    this->thumbnail->setPixmap(readData->pixmap);
    this->thumbnail->setVisible(true);
}

