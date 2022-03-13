﻿#include "mainwindow.h"
#include "openwith.h"
#include "cocoafunctions.h"
#include "win32functions.h"
#include "ui_openwithdialog.h"

#include <QCollator>
#include <QDir>
#include <QFileDialog>
#include <QProcess>
#include <QStandardPaths>
#include <QMimeDatabase>
#include <QSettings>

#include <QDebug>

const QList<OpenWith::OpenWithItem> OpenWith::getOpenWithItems(const QString &filePath)
{

    QList<OpenWithItem> listOfOpenWithItems;
    if (!QFileInfo::exists(filePath))
        return listOfOpenWithItems;


#if defined Q_OS_MACOS && defined COCOA_LOADED
    listOfOpenWithItems = QVCocoaFunctions::getOpenWithItems(filePath);
#elif defined Q_OS_WIN
#ifdef WIN32_LOADED
    listOfOpenWithItems = QVWin32Functions::getOpenWithItems(filePath);
#endif
#else
    listOfOpenWithItems = getOpenWithItemsFromDesktopFiles(filePath);
#endif

    // Natural/alphabetic sort
    QCollator collator;
    collator.setNumericMode(true);
    std::sort(listOfOpenWithItems.begin(),
              listOfOpenWithItems.end(),
              [&collator](const OpenWith::OpenWithItem &item0, const OpenWith::OpenWithItem &item1)
    {
            return collator.compare(item0.name, item1.name) < 0;
    });

    // Move default item to beginning
    for (int i = 0; i < listOfOpenWithItems.length(); i++)
    {
        const auto &item = listOfOpenWithItems.at(i);
        if (item.isDefault)
            listOfOpenWithItems.move(i, 0);
    }

    return listOfOpenWithItems;
}

QList<OpenWith::OpenWithItem> OpenWith::getOpenWithItemsFromDesktopFiles(const QString &filePath)
{
    QList<OpenWithItem> listOfOpenWithItems;

    QString mimeName;
    if (!filePath.isEmpty())
    {
        QMimeDatabase mimedb;
        QMimeType mime = mimedb.mimeTypeForFile(filePath, QMimeDatabase::MatchContent);
        mimeName = mime.name();
    }

    // This should probably be async dude
    QProcess process;
    process.start("xdg-mime", {"query", "default", mimeName});
    process.waitForFinished();
    QString defaultApplication = process.readAllStandardOutput().trimmed();

    QList<QMap<QString, QString>> programList;

    const QStringList &applicationLocations = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    for (const auto &location : applicationLocations)
    {
        auto dir = QDir(location);
        const auto &entryInfoList = dir.entryInfoList();
        for(const auto &fileInfo : entryInfoList)
        {
            // Don't add qView to the open with menu!
            if (fileInfo.fileName() == "qView.desktop")
                continue;

            if (!fileInfo.fileName().endsWith(".desktop"))
                continue;

            OpenWithItem openWithItem;
            // additional info to hold
            QString mimeTypes;
            bool noDisplay = false;

            QFile file(fileInfo.absoluteFilePath());
            file.open(QIODevice::ReadOnly);
            QTextStream in(&file);
            QString line;
            while (in.readLineInto(&line))
            {
                if (line.startsWith("Name=", Qt::CaseInsensitive) && openWithItem.name.isEmpty())
                {
                    line.remove("Name=", Qt::CaseInsensitive);
                    openWithItem.name = line;
                }
                else if (line.startsWith("Icon=", Qt::CaseInsensitive) && openWithItem.icon.isNull())
                {
                    line.remove("Icon=", Qt::CaseInsensitive);
                    openWithItem.icon = QIcon::fromTheme(line);
                }
                else if (line.startsWith("Categories=", Qt::CaseInsensitive) && openWithItem.categories.isEmpty())
                {
                    line.remove("Categories=", Qt::CaseInsensitive);
                    openWithItem.categories = line.split(";");
                }
                else if (line.startsWith("Exec=", Qt::CaseInsensitive) && openWithItem.exec.isEmpty())
                {
                    line.remove("Exec=", Qt::CaseInsensitive);
                    QRegularExpression regExp;
                    regExp.setPattern("%.*");
                    line.remove(regExp);
                    openWithItem.exec = line;
                }
                else if (line.startsWith("MimeType=", Qt::CaseInsensitive) && mimeTypes.isEmpty())
                {
                    line.remove("MimeType=", Qt::CaseInsensitive);
                    mimeTypes = line;
                }
                else if (line.startsWith("NoDisplay=", Qt::CaseInsensitive))
                {
                    line.remove("NoDisplay=");
                    if (!line.compare("true", Qt::CaseInsensitive))
                    {
                        noDisplay = true;
                    }
                }
                else if (line.startsWith("Hidden=", Qt::CaseInsensitive))
                {
                    line.remove("Hidden=");
                    if (!line.compare("true", Qt::CaseInsensitive))
                    {
                        noDisplay = true;
                    }
                }
            }
            if ((mimeTypes.contains(mimeName, Qt::CaseInsensitive) || mimeName.isEmpty()) && !noDisplay)
            {
                // If the program is the default program, save it to add to the beginning after sorting
                openWithItem.isDefault = fileInfo.fileName() == defaultApplication;

                listOfOpenWithItems.append(openWithItem);
            }
        }
    }

    return listOfOpenWithItems;
}

void OpenWith::showOpenWithDialog(QWidget *parent)
{
    auto mainWindow = reinterpret_cast<MainWindow*>(parent);
    QString filePath = mainWindow->getCurrentFileDetails().fileInfo.absoluteFilePath();
#ifdef Q_OS_MACOS
    auto openWithDialog = new QFileDialog(parent);
    openWithDialog->setNameFilters({QT_TR_NOOP("All Applications (*.app)")});
    openWithDialog->setDirectory("/Applications");
    openWithDialog->open();
    connect(openWithDialog, &QFileDialog::fileSelected, [filePath](const QString &executablePath){
        openWithExecutable("open", {"-a", executablePath}, filePath);
    });
#elif defined Q_OS_WIN
#ifdef WIN32_LOADED
    QVWin32Functions::showOpenWithDialog(filePath, mainWindow->windowHandle());
#else
    auto openWithDialog = new QFileDialog(parent);
    openWithDialog->setWindowTitle("Open with...");
    openWithDialog->setNameFilters({QT_TR_NOOP("Programs (*.exe *.pif *.com *.bat *.cmd)"), QT_TR_NOOP("All Files (*)")});
    openWithDialog->setDirectory(QProcessEnvironment::systemEnvironment().value("PROGRAMFILES", "C:\\"));
    openWithDialog->open();
    connect(openWithDialog, &QFileDialog::fileSelected, [filePath](const QString &executablePath){
        openWithExecutable(executablePath, filePath);
    });
#endif
#else
    auto openWithDialog = new OpenWithDialog(parent);
    openWithDialog->open();
    connect(openWithDialog, &OpenWithDialog::selected, [filePath](const QString &exec, const QStringList &args){
        openWithExecutable(exec, args, filePath);
    });
#endif
}

void OpenWith::openWithExecutable(const QString &executablePath, const QString &filePath)
{
    OpenWithItem item;
    item.exec = executablePath;
    openWith(filePath, item);
}

void OpenWith::openWithExecutable(const QString &executablePath, const QStringList &args, const QString &filePath)
{
    OpenWithItem item;
    item.exec = executablePath;
    item.args = args;
    openWith(filePath, item);
}

void OpenWith::openWith(const QString &filePath, const OpenWithItem &openWithItem)
{
    const QString &nativeFilePath = QDir::toNativeSeparators(filePath);
    const QString &exec = openWithItem.exec.trimmed();
    QStringList args = openWithItem.args;

    if (exec.isEmpty() || exec.isNull())
        return;

    // Windows-only native app launch method
    if (openWithItem.winAssocHandler)
    {
#if defined Q_OS_WIN && WIN32_LOADED
        QVWin32Functions::openWithInvokeAssocHandler(nativeFilePath, openWithItem.winAssocHandler);
#endif
    }
    else
    {
        args.append(nativeFilePath);
        QProcess::startDetached(exec, args);
    }
}

// OpenWithDialog (for linux)
OpenWithDialog::OpenWithDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenWithDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint | Qt::CustomizeWindowHint));

    connect(this, &QDialog::accepted, this, &OpenWithDialog::triggeredOpen);
    connect(ui->treeView, &QTreeView::doubleClicked, this, &OpenWithDialog::triggeredOpen);

    model = new QStandardItemModel();

    ui->treeView->setModel(model);
    populateTreeView();
}

void OpenWithDialog::populateTreeView()
{
    auto listOfAllApps = OpenWith::getOpenWithItems("");

    for (const auto &category : categories)
    {
        auto *categoryItem = new QStandardItem(QIcon::fromTheme(category.iconName, QIcon::fromTheme("applications-other")), category.readableName);

        QMutableListIterator<OpenWith::OpenWithItem> i(listOfAllApps);
        while (i.hasNext()) {
            auto app = i.next();
            if (app.categories.contains(category.name, Qt::CaseInsensitive) || category.name.isEmpty())
            {
                auto *item = new QStandardItem(app.icon, app.name);
                item->setData(app.exec, Qt::UserRole);
                categoryItem->setChild(categoryItem->rowCount(), item);
                i.remove();
            }
        }
        if (categoryItem->rowCount())
            model->appendRow(categoryItem);
    }
}

void OpenWithDialog::triggeredOpen()
{
    auto selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
        return;

    QString exec = selectedIndexes.first().data(Qt::UserRole).toString();
    QStringList args = exec.split(" ");
    exec = args.takeFirst();
    emit selected(exec, args);
}

OpenWithDialog::~OpenWithDialog()
{
    delete ui;
}

