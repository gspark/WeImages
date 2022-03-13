#ifndef APPLICATION_H
#define APPLICATION_H

#include "mainwindow.h"
#include "settingsmanager.h"
#include "shortcutmanager.h"
#include "actionmanager.h"
#include "optionsdialog.h"
#include "aboutdialog.h"


#include <QApplication>
#include <QRegularExpression>

#if defined(qvApp)
#undef qvApp
#endif

#define qvApp (qobject_cast<Application *>(QCoreApplication::instance()))	// global qvapplication object

class Application : public QApplication
{
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);
    ~Application() override;

    bool event(QEvent *event) override;

    static void openFile(MainWindow *window, const QString &file, bool resize = true);

    static void openFile(const QString &file, bool resize = true);

    static void pickFile(MainWindow *parent = nullptr);

    static void pickUrl(MainWindow *parnet = nullptr);

    static MainWindow *newWindow();

    MainWindow *getMainWindow(bool shouldBeEmpty);

    void recentsMenuUpdated();

    qint64 getPreviouslyRecordedFileSize(const QString &fileName);

    void setPreviouslyRecordedFileSize(const QString &fileName, long long *fileSize);

    QSize getPreviouslyRecordedImageSize(const QString &fileName);

    void setPreviouslyRecordedImageSize(const QString &fileName, QSize *imageSize);

    void addToLastActiveWindows(MainWindow *window);

    void deleteFromLastActiveWindows(MainWindow *window);

    void openOptionsDialog(QWidget *parent = nullptr);

    void openAboutDialog(QWidget *parent = nullptr);

    void hideIncompatibleActions();

    void defineFilterLists();

    QMenuBar *getMenuBar() const {  return menuBar; }

    const QStringList &getFilterList() const { return filterList; }

    const QStringList &getNameFilterList() const { return nameFilterList; }

    const QList<QRegularExpression> &getFilterRegExpList() const { return filterRegExpList; }

    const QStringList &getMimeTypeNameList() const { return mimeTypeNameList; }

    SettingsManager &getSettingsManager() { return settingsManager; }

    ShortcutManager &getShortcutManager() { return shortcutManager; }

    ActionManager &getActionManager() { return actionManager; }

private:

    QList<MainWindow*> lastActiveWindows;

    QMenu *dockMenu;

    QMenuBar *menuBar;

    QCache<QString, qint64> previouslyRecordedFileSizes;
    QCache<QString, QSize> previouslyRecordedImageSizes;

    QStringList filterList;
    QStringList nameFilterList;
    QList<QRegularExpression> filterRegExpList;
    QStringList mimeTypeNameList;

    // This order is very important
    SettingsManager settingsManager; 
    ActionManager actionManager;
    ShortcutManager shortcutManager;

    QPointer<OptionsDialog> optionsDialog;
    QPointer<QVAboutDialog> aboutDialog;

};

#endif // APPLICATION_H
