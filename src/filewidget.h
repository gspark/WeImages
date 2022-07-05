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
class QFileSystemModel;
class QVBoxLayout;
class QAbstractItemDelegate;
class QStackedWidget;
class FileFilterProxyModel;
class QAbstractItemModel;
class ThumbnailData;
class QStandardItem;


enum FileViewType {
    Table, Thumbnail
};

class FileWidget : public QWidget {
    Q_OBJECT
public:
  
    explicit FileWidget(QAbstractItemModel* model, ImageCore* imageCore, QWidget* parent = nullptr);

    ~FileWidget() override;

    void setupToolBar();

public slots:
    void onTreeViewClicked(const QString& path);

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
    ThumbnailDelegate* m_delegate;

    mutable std::recursive_mutex _fileListAndCurrentDirMutex;

    // widget init
    void initListView();

    void initTableView();

    void initThumbnailView();

    void initWidgetLayout();

    void cdPath(const QString& path);

    void updateCurrentPath(const QString& dir);

    void setThumbnailView(const QString& dir, bool readPixmap = true);

    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    FileViewType fileViewType;

    QString currentDirPath;

    QList<QFileInfo> getFileInfoList(const QString& currentDirPath);
 
private slots:

    void onFileDoubleClicked(const QModelIndex &index);

    void thumbnail();

    void detail();

    void selectAll();

signals:
    void onCdDir(const QString path);
};

#endif // FILEWIDGET_H
