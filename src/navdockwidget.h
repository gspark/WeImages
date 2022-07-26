#ifndef NAVDOCKWIDGET_H
#define NAVDOCKWIDGET_H

#include <QDockWidget>

class QLabel;
class ImageCore;
class QFileSystemModel;
class FileFilterProxyModel;
class QTreeView;
class QAbstractItemModel;
class ImageReadData;

class NavDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    NavDockWidget(QAbstractItemModel* model, ImageCore* imageCore);
    ~NavDockWidget();

    virtual QSize sizeHint() const;

public slots:
    void onSetPath(const QString& path);
    void onCdDir(const QString path);
private:
    ImageCore* imageCore;
    // can not delete
    QFileSystemModel* fileModel;
    FileFilterProxyModel* proxyModel;
    QTreeView* treeView;
    QLabel* thumbnail;

    void fileModelInit();
    void treeViewInit();
    void currentRowChanged();
private slots:
    void onTreeViewClicked(const QModelIndex& index);
    void imageLoaded(ImageReadData* readData);
signals:
    void treeViewClicked(const QString path);
};

#endif // NAVDOCKWIDGET_H
