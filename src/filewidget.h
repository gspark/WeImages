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


#define USE_INSERT_HISTORY_MENU     1
#define MAX_HISTORY_COUNT           20
#define MAX_TAB_COUNT               10

#define HEADER_SIZE_DEFAULT         125
#define HEADER_SIZE_NAME            300
#define HEADER_SIZE_MODIFIED        150

#define STATUS_LAB_WIDTH_ITEM       100

#define HISTORY_WIDTH_MENUBTN       25
#define HISTORY_WIDTH_BUTTON        30

class QToolBar;
class QListView;
class QTableView;
class FileListModel;
class FileListSortFilterProxyModel;
class FileSystemObject;
class ItemDelegate;
class QStandardItemModel;
class QVBoxLayout;
class QAbstractItemDelegate;
class QStackedWidget;

enum FileViewType {
    List, thumbnail
};

class FileWidget : public QWidget {
    Q_OBJECT
public:
    QString name;       // used to save and load settings

    explicit FileWidget(const QString& tag, ImageCore* imageCore, QWidget* parent = nullptr);

    ~FileWidget() override;

    void setupToolBar();

public slots:    // for shortcut
    void onItemActivated(const QString& path);
    void onNavigateBarClicked(const QString& path);

private:
    ImageCore* imageCore;

    QToolBar* toolBar;

    FileListModel* model;
    // proxy model and current dir
    FileListSortFilterProxyModel* sortModel;

    QStackedWidget* stackedWidget;

    // pointer of current view and history, can not delete
    QTableView* listView;

    QListView* thumbnailView;

    ItemDelegate* m_delegate;                 //委托

    QAbstractItemDelegate* list_delegate;

    // widget init
    void initListView();

    void initTabelView();

    void initThumbnailView();

    void initWidgetLayout();

    void cdPath(const QString& path);

    void updateCurrentPath(const QString& dir);

    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    void fillFromList(const std::map<qulonglong, FileSystemObject> items);

    int setListView(const std::map<qulonglong, FileSystemObject> items, QStandardItemModel* data);

    int setThumbnailView(const std::map<qulonglong, FileSystemObject> items, QStandardItemModel* data);

    FileViewType fileViewType;
 
private slots:
    // tree view
    void onTreeViewClicked(const QModelIndex& index);

    //void onTreeViewDoubleClicked(const QModelIndex &index);

    void thumbnail();

    void detail();
};

#endif // FILEWIDGET_H
