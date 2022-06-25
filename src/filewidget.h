#ifndef FILEWIDGET_H
#define FILEWIDGET_H

#include <QWidget>

#include <QSortFilterProxyModel>
#include <QTabWidget>
#include <QTreeView>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include <QDir>
#include <QMimeData>
#include <QFileInfo>

#include "treeview.h"
#include "imagecore.h"

class QToolBar;
class QListView;
class QTableView;
class ItemDelegate;
class QStandardItemModel;
class QFileSystemModel;
class QVBoxLayout;
class QAbstractItemDelegate;
class QStackedWidget;
class FileFilterProxyModel;
class QAbstractItemModel;
class ThumbnailData;
class QStandardItem;


enum FileViewType {
    List, thumbnail
};

class FileWidget : public QWidget {
    Q_OBJECT
public:
  
    explicit FileWidget(QAbstractItemModel* model, ImageCore* imageCore, QWidget* parent = nullptr);

    ~FileWidget() override;

    void setupToolBar();

public slots:    // for shortcut
    void onItemActivated(const QString& path);
    void onNavigateBarClicked(const QString& path);

private:
    ImageCore* imageCore;

    QToolBar* toolBar;

    QStandardItemModel* thumbnailModel;

    QFileSystemModel* fileSystemMode;

    FileFilterProxyModel* proxyModel;

    QStackedWidget* stackedWidget;

    // pointer of current view and history, can not delete
    QTableView* tableView;

    QListView* thumbnailView;

    // 委托
    ItemDelegate* m_delegate;

    mutable std::recursive_mutex _fileListAndCurrentDirMutex;

    // widget init
    void initListView();

    void initTableView();

    void initThumbnailView();

    void initWidgetLayout();

    void cdPath(const QString& path);

    void updateCurrentPath(const QString& dir);

    //QStandardItem* getThumbnailItem(QFileInfo& fileInfo);

    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    FileViewType fileViewType;

    QList<QFileInfo> getFileInfoList(const QString& currentDirPath);
 
private slots:
    // tree view
    void onTreeViewClicked(const QModelIndex& index);

    //void onTreeViewDoubleClicked(const QModelIndex &index);

    void thumbnail();

    void detail();
};

#endif // FILEWIDGET_H
