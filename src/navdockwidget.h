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
    void onSetPath(const QString path);
    void onCdDir(const QString path);
private:
    QFileSystemModel* fileModel;     // can not delete here
    FileFilterProxyModel* proxyModel;
    QTreeView* treeView;
    QLabel* thumbnail;

    void fileModelInit();
    void treeViewInit();

    ImageCore* imageCore;

private slots:
    void onTreeViewClicked(const QModelIndex& index);
    void imageLoaded(ImageReadData* readData);

signals:
    void treeViewClicked(const QString path);
};

#endif // NAVDOCKWIDGET_H
