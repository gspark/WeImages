#ifndef NAVDOCKWIDGET_H
#define NAVDOCKWIDGET_H

#include <QDockWidget>
#include <QHeaderView>
#include <QTreeView>

#include "treeview.h"

class QLabel;
class ImageCore;
class QScrollArea;
class QFileSystemModel;
class FileFilterProxyModel;

class NavDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    NavDockWidget(QAbstractItemModel* model, ImageCore* imageCore);
    ~NavDockWidget();

    virtual QSize sizeHint() const;

    void refreshTreeView();
    void loadDockInfo();
    void saveDockInfo();

private:
    QFileSystemModel* fileModel;     // can not delete here
    FileFilterProxyModel* proxyModel;
    TreeView* treeView;
    QLabel* thumbnail;

    void fileModelInit();
    void treeViewInit();

    ImageCore* imageCore;
    QScrollArea* scrollArea;

private slots:
    void onExpanded(const QModelIndex& index);
    void onTreeViewClicked(const QModelIndex& index);
    void fileDataChanged(const QPixmap& readData);

signals:
    void navDockClicked(const QString path);
};

#endif // NAVDOCKWIDGET_H
