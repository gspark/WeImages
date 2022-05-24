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

#include "filesystemmodel.h"
#include "filefilterproxymodel.h"
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


class QVGraphicsView;

class FileWidget : public QWidget {
Q_OBJECT
public:
    QString name;       // used to save and load settings

    explicit FileWidget(const QString &tag, QAbstractItemModel *model, ImageCore *imageCore, QWidget *parent = nullptr);

    ~FileWidget() override;

    virtual QSize sizeHint() const;

    QTreeView *getView() const { return treeView; }

    void fileWidgetAddTab();


public slots:    // for shortcut
    void cutSelectedItem();

    void copySelectedItem();

    void pasteSelectedItem();

    void deleteSelectedItem();

    void refreshTreeView();

    void expandCollapseOne();

    void expandCollapseAll();

    void onVisibilityChanged(bool visible);

    void onNavigateBarClicked(const QString &path);

    void onItemActivated(const QString &path);


private:
    typedef struct {
        QString name;
        QIcon icon;
    } deviceInfo;
    deviceInfo myComputer;

    // control button and menu
    typedef struct {
        int pos;
        QMenu *menu;
        QString path;
        QList<QAction *> act;   // add to menu, can not delete
    } historyPath;

    // navigate box
    QComboBox *pathBox;

    // proxy model and current dir
    FileFilterProxyModel *proxyModel;
    QDir *currDir;

    // status bar
    QLabel *statLab;
    QLabel *selectionLab;


    // current view path, equal tabList.at(currentIndex())->path
    QString treeViewPath;

    // pointer of current view and history, can not delete
    TreeView *treeView;
    historyPath *history;


    // widget init
    void initLabel();

    void initPathBox();

    void initTreeView();

    void initWidgetLayout();

    // widget control
    void cdPath(const QString &path);

    void updateRecord();

    void showMyComputer();

    void updateStatusBar();

    void updateSelectionInfo();

    void updateCurrentPath(const QString &dir);

    void updateCurrentTab(int index);

    void updateTreeView(const QString &dir, bool sort = true, bool defaultOrder = false);

    void refreshExpandedFolder(const QString &dir);

    void refreshModelData(const QString &dir);

    void refreshTreeViewNotSort();

    // history menu
    void hisPathSwitch(const QString &path);

    void removeInvalidHisAction(QAction *action);

    // for context menu
    QString dirPathFromIndex(const QModelIndex &index);

    QStringList dirEntryList(const QString &path);

    void openIndexes(const QModelIndexList &indexes);

    void createFolder(const QString &name, const QModelIndex &index = QModelIndex());

    void createTextFile(const QString &name, const QModelIndex &index = QModelIndex());

    bool isDirVisible(const QString &dir);

    void expandCollapseIndex(const QModelIndex &index, bool expand);

    void setMimeDataAction(const QModelIndexList &indexes, Qt::DropAction action);

    void mimeDataAction(const QModelIndex &index = QModelIndex(), Qt::DropAction action = Qt::IgnoreAction);

    void pasteMimeDataAction(const QModelIndex &index);

    void contextMenu(const QPoint &pos);

    ImageCore *imageCore;

    QVGraphicsView *graphicsView;

private slots:

    // file model
    void onFileRenamed(const QString &path, const QString &oldName, const QString &newName);

    void onDirectoryLoaded(const QString &path);

    void onDropCompleted(const QString &dir, const QString &target);

    void onPasteCompleted(const QString &dir, const QString &target);

    // tree view
    void onExpanded(const QModelIndex &index);

    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void onTreeViewClicked(const QModelIndex &index);

    void onTreeViewDoubleClicked(const QModelIndex &index);

    void onComboBoxReturnPressed();

    void onComboBoxActivated(int index);

    void onComboBoxIndexChanged(int index);

    void onRecordTriggered(QAction *action);

    void onPrevClicked();

    void onNextClicked();
};

#endif // FILEWIDGET_H
