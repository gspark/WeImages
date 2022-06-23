#ifndef FILEWIDGET_H
#define FILEWIDGET_H

#include <QWidget>

#include <QSortFilterProxyModel>
#include <QFileSystemWatcher>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QTabWidget>
#include <QTreeView>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QMenu>
#include <QDir>
#include <QMimeData>

#include "treeview.h"
#include "imagecore.h"

class QToolBar;
class QListView;
class QTableView;
class ItemDelegate;
class QStandardItemModel;
class QVBoxLayout;
class QAbstractItemDelegate;
class QStackedWidget;
class FileFilterProxyModel;
class QAbstractItemModel;

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

    FileFilterProxyModel* proxyModel;

    QStackedWidget* stackedWidget;

    // pointer of current view and history, can not delete
    QTableView* listView;

    QListView* thumbnailView;

    // 委托
    ItemDelegate* m_delegate;

    // widget init
    void initListView();

    void initTableView();

    void initThumbnailView();

    void initWidgetLayout();

    void cdPath(const QString& path);

    void updateCurrentPath(const QString& dir);

    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    int setThumbnailView(QFileSystemModel* fileModel);

    FileViewType fileViewType;
 
private slots:
    // tree view
    void onTreeViewClicked(const QModelIndex& index);

    //void onTreeViewDoubleClicked(const QModelIndex &index);

    void thumbnail();

    void detail();
};

#endif // FILEWIDGET_H
