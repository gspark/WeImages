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
    loadDockInfo();
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
}


void NavDockWidget::onTreeViewClicked(const QModelIndex &index)
{
    QFileInfo info = proxyModel->fileInfo(index);
    if (info.isDir()) {
        emit treeViewClicked(info.absoluteFilePath());
    }
}

void NavDockWidget::loadDockInfo()
{
//    qDebug() << QString("loadDockInfo");

//    QStringList dirList = readArraySettings(CONFIG_GROUP_NAV_DOCK);
//    qDebug() << dirList;
//    foreach (QString dir, dirList) {
//        treeView->expand(proxyModel->proxyIndex(dir));
//    }

//    QVariant hidden = readSettings(CONFIG_GROUP_NAV_DOCK, CONFIG_DOCK_HIDE);
//    if (hidden.isValid()) {
//        setHidden(hidden.toBool());
//    }
}

void NavDockWidget::saveDockInfo()
{
    QStringList dirList;
    QModelIndex index = proxyModel->index(0, 0);
    while (index.isValid()) {
        if (treeView->isExpanded(index)) {
            QFileInfo info = proxyModel->fileInfo(index);
            if (info.fileName() != "." && info.fileName() != "..") {
                dirList.append(info.absoluteFilePath());
            }
        }
        index = treeView->indexBelow(index);
    }
}

void NavDockWidget::onCdDir(const QString path)
{
    this->treeView->setCurrentIndex(this->proxyModel->proxyIndex(path));
}

void NavDockWidget::imageLoaded(const QPixmap &readData)
{
    this->thumbnail->setPixmap(readData);
    this->thumbnail->setVisible(true);
}

