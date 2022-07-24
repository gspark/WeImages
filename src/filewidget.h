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

#include "imagecore.h"

class QToolBar;
class QListView;
class QTableView;
class ThumbnailDelegate;
class QStandardItemModel;
class FileListModel;
class QVBoxLayout;
class QAbstractItemDelegate;
class QStackedWidget;
class FileFilterProxyModel;
class QAbstractItemModel;
class ThumbnailData;
class QStandardItem;
class CheckBoxDelegate;
class QFileIconProvider;

enum FileViewType {
    Table, Thumbnail
};

class FileWidget : public QWidget {
    Q_OBJECT
public:
  
    explicit FileWidget(/*QAbstractItemModel* model, */ImageCore* imageCore, QWidget* parent = nullptr);

    ~FileWidget() override;

    void setupToolBar();

public slots:
    void onTreeViewClicked(const QString& path);

private:
    ImageCore* _imageCore;

    QToolBar* toolBar;

    QStandardItemModel* thumbnailModel;

    QFileIconProvider* ensureIconProvider();

    QFileIconProvider* m_iconProvider;

    FileListModel* fileListModel;

    FileFilterProxyModel* proxyModel;

    QStackedWidget* stackedWidget;

    // pointer of current view and history, can not delete
    QTableView* tableView;

    QListView* thumbnailView;

    // 委托
    ThumbnailDelegate* thumbnailDelegate;

    CheckBoxDelegate* checkBoxDelegate;

    mutable std::recursive_mutex fileIconMutex;

    FileViewType fileViewType;

    QString currentPath;

    // widget init
    void initListView();

    void initTableView();

    void initThumbnailView();

    void initWidgetLayout();

    void cdPath(const QString& path);

    void setThumbnailView(const QString& dir, bool readPixmap = true);

    void initListModel(const QString& dir, bool readPixmap = true);

    void setTableColWidth();

    void onCurrentChanged(const QModelIndex& current, const QModelIndex& previous);

    QList<QFileInfo> getRowItemList(const QString& currentDirPath);

    QList<QStandardItem*> getRowItemList();


    int _sortColumn;
    int _sortOrder;
    int _column1w;
    void loadFileListInfo();
    void saveFileListInfo();
 
private slots:

    void onFileDoubleClicked(const QModelIndex &index);

    void thumbnail();

    void detail();

    void selectAll();

    void exportSelected();

    void onUpdateItems();

signals:
    void cdDir(const QString path);
};

#endif // FILEWIDGET_H
