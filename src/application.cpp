#include "application.h"
#include "optionsdialog.h"
#include "cocoafunctions.h"

#include <QFileOpenEvent>
#include <QSettings>
#include <QTimer>
#include <QFileDialog>

Application::Application(int &argc, char **argv) : QApplication(argc, argv)
{
    setDesktopFileName("com.interversehq.qView.desktop");

    // Connections
    connect(&actionManager, &ActionManager::recentsMenuUpdated, this, &Application::recentsMenuUpdated);

    // Add fallback fromTheme icon search on linux with qt >5.11
#if defined Q_OS_UNIX && !defined Q_OS_MACOS && QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    QIcon::setFallbackSearchPaths(QIcon::fallbackSearchPaths() << "/usr/share/pixmaps");
#endif

    defineFilterLists();

    // Setup macOS dock menu
    dockMenu = new QMenu();
    connect(dockMenu, &QMenu::triggered, this, [](QAction *triggeredAction){
       ActionManager::actionTriggered(triggeredAction);
    });

    actionManager.loadRecentsList();

#ifdef Q_OS_MACOS
    dockMenu->addAction(actionManager.cloneAction("newwindow"));
    dockMenu->addAction(actionManager.cloneAction("open"));
    dockMenu->setAsDockMenu();
#endif

    // Build menu bar
    menuBar = actionManager.buildMenuBar();
    connect(menuBar, &QMenuBar::triggered, this, [](QAction *triggeredAction){
        ActionManager::actionTriggered(triggeredAction);
    });

    // Set mac-specific application settings
#ifdef COCOA_LOADED
    QVCocoaFunctions::setUserDefaults();
#endif
#ifdef Q_OS_MACOS
    setQuitOnLastWindowClosed(getSettingsManager().getBoolean("quitonlastwindow"));
#endif

    // Block any erroneous icons from showing up on mac and windows
    // (this is overridden in some cases)
#if defined Q_OS_MACOS || defined Q_OS_WIN
    setAttribute(Qt::AA_DontShowIconsInMenus);
#endif
    // Adwaita Qt styles should hide icons for a more consistent look
    if (style()->objectName() == "adwaita-dark" || style()->objectName() == "adwaita")
        setAttribute(Qt::AA_DontShowIconsInMenus);

    hideIncompatibleActions();
}

Application::~Application()
{
    dockMenu->deleteLater();
    menuBar->deleteLater();
}

bool Application::event(QEvent *event)
{
    if (event->type() == QEvent::FileOpen)
    {
        auto *openEvent = static_cast<QFileOpenEvent *>(event);
        openFile(getMainWindow(true), openEvent->file());
    }
    else if (event->type() == QEvent::ApplicationStateChange)
    {
        auto *stateEvent = static_cast<QApplicationStateChangeEvent*>(event);
        if (stateEvent->applicationState() == Qt::ApplicationActive)
            settingsManager.loadSettings();
    }
    return QApplication::event(event);
}

void Application::openFile(MainWindow *window, const QString &file, bool resize)
{
    window->setJustLaunchedWithImage(resize);
    window->openFile(file);
}

void Application::openFile(const QString &file, bool resize)
{
    auto *window = qvApp->getMainWindow(true);

    Application::openFile(window, file, resize);
}

void Application::pickFile(MainWindow *parent)
{
    QSettings settings;
    settings.beginGroup("recents");

    auto *fileDialog = new QFileDialog(parent, tr("Open..."));
    fileDialog->setDirectory(settings.value("lastFileDialogDir", QDir::homePath()).toString());
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    fileDialog->setNameFilters(qvApp->getNameFilterList());
    if (parent)
        fileDialog->setWindowModality(Qt::WindowModal);

    connect(fileDialog, &QFileDialog::filesSelected, fileDialog, [parent](const QStringList &selected){
        bool isFirstLoop = true;
        for (const auto &file : selected)
        {
            if (isFirstLoop && parent)
                parent->openFile(file);
            else
                Application::openFile(file);

            isFirstLoop = false;
        }

        // Set lastFileDialogDir
        QSettings settings;
        settings.beginGroup("recents");
        settings.setValue("lastFileDialogDir", QFileInfo(selected.constFirst()).path());

    });
    fileDialog->show();
}

MainWindow *Application::newWindow()
{
    auto *w = new MainWindow();
    w->show();
    w->raise();

    return w;
}

MainWindow *Application::getMainWindow(bool shouldBeEmpty)
{
    // Attempt to use from list of last active windows
    for (const auto &window : qAsConst(lastActiveWindows))
    {
        if (!window)
            continue;

        if (shouldBeEmpty)
        {
            // File info is set if an image load is requested, but not loaded
            if (!window->getCurrentFileDetails().isLoadRequested)
            {
                return window;
            }
        }
        else
        {
            return window;
        }
    }

    // If none of those are valid, scan the list for any existing MainWindow
    const auto topLevelWidgets = QApplication::topLevelWidgets();
    for (const auto &widget : topLevelWidgets)
    {
        if (auto *window = qobject_cast<MainWindow*>(widget))
        {
            if (shouldBeEmpty)
            {
                if (!window->getCurrentFileDetails().isLoadRequested)
                {
                    return window;
                }
            }
            else
            {
                return window;
            }
        }
    }

    // If there are no valid ones, make a new one.
    auto *window = newWindow();
    return window;
}

void Application::recentsMenuUpdated()
{
#ifdef COCOA_LOADED
    QStringList recentsPathList;
    for(const auto &recent : actionManager.getRecentsList())
    {
        recentsPathList << recent.filePath;
    }
    QVCocoaFunctions::setDockRecents(recentsPathList);
#endif
}

qint64 Application::getPreviouslyRecordedFileSize(const QString &fileName)
{
    auto previouslyRecordedFileSizePtr = previouslyRecordedFileSizes.object(fileName);
    qint64 previouslyRecordedFileSize = 0;

    if (previouslyRecordedFileSizePtr)
        previouslyRecordedFileSize = *previouslyRecordedFileSizePtr;

    return previouslyRecordedFileSize;
}

void Application::setPreviouslyRecordedFileSize(const QString &fileName, long long *fileSize)
{
    previouslyRecordedFileSizes.insert(fileName, fileSize);
}

QSize Application::getPreviouslyRecordedImageSize(const QString &fileName)
{
    auto previouslyRecordedImageSizePtr = previouslyRecordedImageSizes.object(fileName);
    QSize previouslyRecordedImageSize = QSize();

    if (previouslyRecordedImageSizePtr)
        previouslyRecordedImageSize = *previouslyRecordedImageSizePtr;

    return previouslyRecordedImageSize;
}

void Application::setPreviouslyRecordedImageSize(const QString &fileName, QSize *imageSize)
{
    previouslyRecordedImageSizes.insert(fileName, imageSize);
}

void Application::addToLastActiveWindows(MainWindow *window)
{
    if (!window)
        return;

    if (!lastActiveWindows.isEmpty() && window == lastActiveWindows.first())
        return;

    lastActiveWindows.prepend(window);

    if (lastActiveWindows.length() > 5)
        lastActiveWindows.removeLast();
}

void Application::deleteFromLastActiveWindows(MainWindow *window)
{
    if (!window)
        return;

    lastActiveWindows.removeAll(window);
}

void Application::openOptionsDialog(QWidget *parent)
{
#ifdef Q_OS_MACOS
    // On macOS, the dialog should not be dependent on any window
    parent = nullptr;
#endif


    if (optionsDialog)
    {
        optionsDialog->raise();
        optionsDialog->activateWindow();
        return;
    }

    optionsDialog = new OptionsDialog(parent);
    optionsDialog->show();
}

void Application::openAboutDialog(QWidget *parent)
{
#ifdef Q_OS_MACOS
    // On macOS, the dialog should not be dependent on any window
    parent = nullptr;
#endif

    if (aboutDialog)
    {
        aboutDialog->raise();
        aboutDialog->activateWindow();
        return;
    }

    aboutDialog = new QVAboutDialog(parent);
    aboutDialog->show();
}

void Application::hideIncompatibleActions()
{    
    // Deletion actions
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
    auto hideDeleteActions = [this]{
        getActionManager().hideAllInstancesOfAction("delete");
        getActionManager().hideAllInstancesOfAction("undo");

        getShortcutManager().setShortcutsHidden({"delete", "undo"});
    };
#if defined Q_OS_UNIX && !defined Q_OS_MACOS
    QProcess *testGio = new QProcess(this);
    testGio->start("gio", QStringList());
    connect(testGio, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [hideDeleteActions, testGio, this](){
        if (testGio->error() == QProcess::FailedToStart)
        {
            qInfo() << "No backup gio trash backend found";
            hideDeleteActions();
        }
        else
        {
            qInfo() << "Using backup gio trash backend";
        }
    });
#elif defined Q_OS_WIN || (defined Q_OS_MACOS && !COCOA_LOADED)
    qInfo() << "Qt version too old for trash feature";
    hideDeleteActions();
#endif
#endif
}

void Application::defineFilterLists()
{
    const auto &byteArrayFormats = QImageReader::supportedImageFormats();

    auto filterString = tr("Supported Images") + " (";
    filterList.reserve(byteArrayFormats.size()-1);
    filterRegExpList.reserve(byteArrayFormats.size()-1);

    // Build the filterlist, filterstring, and filterregexplist in one loop
    for (const auto &byteArray : byteArrayFormats)
    {
        const auto fileExtString = "*." + QString::fromUtf8(byteArray);
        // Qt 5.15 seems to have added pdf support for QImageReader but it is super broken in qView
        if (fileExtString == "*.pdf")
            continue;

        filterList << fileExtString;
        filterString += fileExtString + " ";

        QString re = QRegularExpression::wildcardToRegularExpression(fileExtString);
        filterRegExpList << QRegularExpression(re, QRegularExpression::CaseInsensitiveOption);

        // If we support jpg, we actually support the jfif, jfi, and jpe file extensions too almost certainly.
        if (fileExtString == "*.jpg")
        {
            filterList << "*.jpe" << "*.jfi" << "*.jfif";
            filterString += "*.jpe *.jfi *.jfif";
            filterRegExpList << QRegularExpression(QRegularExpression::wildcardToRegularExpression("*.jpe"), QRegularExpression::CaseInsensitiveOption)
                             << QRegularExpression(QRegularExpression::wildcardToRegularExpression("*.jfi"), QRegularExpression::CaseInsensitiveOption)
                             << QRegularExpression(QRegularExpression::wildcardToRegularExpression("*.jfif"), QRegularExpression::CaseInsensitiveOption);
        }
    }
    filterString.chop(1);
    filterString += ")";


    // Build mime type list
    const auto &byteArrayMimeTypes = QImageReader::supportedMimeTypes();
    mimeTypeNameList.reserve(byteArrayMimeTypes.size()-1);
    for (const auto &byteArray : byteArrayMimeTypes)
    {
        // Qt 5.15 seems to have added pdf support for QImageReader but it is super broken in qView
        const QString mime = QString::fromUtf8(byteArray);
        if (mime == "application/pdf")
            continue;

        mimeTypeNameList << mime;
    }

    // Build name filter list for file dialogs
    nameFilterList << filterString;
    nameFilterList << tr("All Files") + " (*)";
}
