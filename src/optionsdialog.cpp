#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "application.h"
#include <QColorDialog>
#include <QPalette>
#include <QScreen>
#include <QMessageBox>
#include <QSettings>

#include <QDebug>

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    languageRestartMessageShown = false;

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint | Qt::CustomizeWindowHint));

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &OptionsDialog::buttonBoxClicked);
    connect(ui->shortcutsTable, &QTableWidget::cellDoubleClicked, this, &OptionsDialog::shortcutCellDoubleClicked);
    connect(ui->bgColorCheckbox, &QCheckBox::stateChanged, this, &OptionsDialog::bgColorCheckboxStateChanged);
    connect(ui->scalingCheckbox, &QCheckBox::stateChanged, this, &OptionsDialog::scalingCheckboxStateChanged);

    populateLanguages();

    QSettings settings;
    ui->tabWidget->setCurrentIndex(settings.value("optionstab", 1).toInt());

    // On macOS, the dialog should not be dependent on any window
#ifndef Q_OS_MACOS
    setWindowModality(Qt::WindowModal);
#else
    // Load window geometry
    restoreGeometry(settings.value("optionsgeometry").toByteArray());
#endif


#ifdef Q_OS_UNIX
    setWindowTitle("Preferences");
#endif

// Platform specific settings
#ifdef Q_OS_MACOS
    ui->menubarCheckbox->hide();
#else
    ui->darkTitlebarCheckbox->hide();
    ui->quitOnLastWindowCheckbox->hide();
#endif

// Hide language selection below 5.12, as 5.12 does not support embedding the translations :(
#if (QT_VERSION < QT_VERSION_CHECK(5, 12, 0))
    ui->langComboBox->hide();
    ui->langComboLabel->hide();
#endif

    syncSettings(false, true);
    connect(ui->langComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OptionsDialog::languageComboBoxCurrentIndexChanged);
    syncShortcuts();
    updateButtonBox();
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::done(int r)
{
    // Save window geometry
    QSettings settings;
    settings.setValue("optionsgeometry", saveGeometry());
    settings.setValue("optionstab", ui->tabWidget->currentIndex());

    QDialog::done(r);
}

void OptionsDialog::modifySetting(QString key, QVariant value)
{
    transientSettings.insert(key, value);
    updateButtonBox();
}

void OptionsDialog::saveSettings()
{
    QSettings settings;
    settings.beginGroup("options");

    const auto keys = transientSettings.keys();
    for (const auto &key : keys)
    {
        const auto &value = transientSettings[key];
        settings.setValue(key, value);
    }

    settings.endGroup();
    settings.beginGroup("shortcuts");

    const auto &shortcutsList = qvApp->getShortcutManager().getShortcutsList();
    for (int i = 0; i < transientShortcuts.length(); i++)
    {
        settings.setValue(shortcutsList.value(i).name, transientShortcuts.value(i));
    }

    qvApp->getShortcutManager().updateShortcuts();
    qvApp->getSettingsManager().loadSettings();
}

void OptionsDialog::syncSettings(bool defaults, bool makeConnections)
{
    auto &settingsManager = qvApp->getSettingsManager();
    settingsManager.loadSettings();

    // bgcolorenabled
    syncCheckbox(ui->bgColorCheckbox, "bgcolorenabled", defaults, makeConnections);
    if (ui->bgColorCheckbox->isChecked())
        ui->bgColorButton->setEnabled(true);
    else
        ui->bgColorButton->setEnabled(false);
    // bgcolor
    ui->bgColorButton->setText(settingsManager.getString("bgcolor", defaults));
    transientSettings.insert("bgcolor", ui->bgColorButton->text());
    updateBgColorButton();
    connect(ui->bgColorButton, &QPushButton::clicked, this, &OptionsDialog::bgColorButtonClicked);
    // titlebarmode
    syncRadioButtons({ui->titlebarRadioButton0, ui->titlebarRadioButton1,
                     ui->titlebarRadioButton2, ui->titlebarRadioButton3}, "titlebarmode", defaults, makeConnections);
    // windowresizemode
    syncComboBox(ui->windowResizeComboBox, "windowresizemode", defaults, makeConnections);
    if (ui->windowResizeComboBox->currentIndex() == 0) {
        ui->minWindowResizeLabel->setEnabled(false);
        ui->minWindowResizeSpinBox->setEnabled(false);
        ui->maxWindowResizeLabel->setEnabled(false);
        ui->maxWindowResizeSpinBox->setEnabled(false);
    } else {
        ui->minWindowResizeLabel->setEnabled(true);
        ui->minWindowResizeSpinBox->setEnabled(true);
        ui->maxWindowResizeLabel->setEnabled(true);
        ui->maxWindowResizeSpinBox->setEnabled(true);
    }
    // minwindowresizedpercentage
    syncSpinBox(ui->minWindowResizeSpinBox, "minwindowresizedpercentage", defaults, makeConnections);
    // maxwindowresizedperecentage
    syncSpinBox(ui->maxWindowResizeSpinBox, "maxwindowresizedpercentage", defaults, makeConnections);
    // titlebaralwaysdark
    syncCheckbox(ui->darkTitlebarCheckbox, "titlebaralwaysdark", defaults, makeConnections);
    // quitonlastwindow
    syncCheckbox(ui->quitOnLastWindowCheckbox, "quitonlastwindow", defaults, makeConnections);
    // menubarenabled
    syncCheckbox(ui->menubarCheckbox, "menubarenabled", defaults, makeConnections);
    // fullscreendetails
    syncCheckbox(ui->detailsInFullscreen, "fullscreendetails", defaults, makeConnections);
    // filteringenabled
    syncCheckbox(ui->filteringCheckbox, "filteringenabled", defaults, makeConnections);
    // scalingenabled
    syncCheckbox(ui->scalingCheckbox, "scalingenabled", defaults, makeConnections);
    if (ui->scalingCheckbox->isChecked())
        ui->scalingTwoCheckbox->setEnabled(true);
    else
        ui->scalingTwoCheckbox->setEnabled(false);
    // scalingtwoenabled
    syncCheckbox(ui->scalingTwoCheckbox, "scalingtwoenabled", defaults, makeConnections);
    // scalefactor
    syncSpinBox(ui->scaleFactorSpinBox, "scalefactor", defaults, makeConnections);
    // scrollzoomsenabled
    syncCheckbox(ui->scrollZoomsCheckbox, "scrollzoomsenabled", defaults, makeConnections);
    // cursorzoom
    syncCheckbox(ui->cursorZoomCheckbox, "cursorzoom", defaults, makeConnections);
    // cropmode
    syncComboBox(ui->cropModeComboBox, "cropmode", defaults, makeConnections);
    // pastactualsizeenabled
    syncCheckbox(ui->pastActualSizeCheckbox, "pastactualsizeenabled", defaults, makeConnections);
    // language
    syncComboBoxData(ui->langComboBox, "language", defaults, makeConnections);
    // sortmode
    syncComboBox(ui->sortComboBox, "sortmode", defaults, makeConnections);
    // sortdescending
    syncRadioButtons({ui->descendingRadioButton0, ui->descendingRadioButton1}, "sortdescending", defaults, makeConnections);
    // preloadingmode
    syncComboBox(ui->preloadingComboBox, "preloadingmode", defaults, makeConnections);
    // loopfolders
    syncCheckbox(ui->loopFoldersCheckbox, "loopfoldersenabled", defaults, makeConnections);
    // slideshowreversed
    syncComboBox(ui->slideshowDirectionComboBox, "slideshowreversed", defaults, makeConnections);
    // slideshowtimer
    syncDoubleSpinBox(ui->slideshowTimerSpinBox, "slideshowtimer", defaults, makeConnections);
    // afterdelete
    syncComboBox(ui->afterDeletionComboBox, "afterdelete", defaults, makeConnections);
    // askdelete
    syncCheckbox(ui->askDeleteCheckbox, "askdelete", defaults, makeConnections);
    // saverecents
    syncCheckbox(ui->saveRecentsCheckbox, "saverecents", defaults, makeConnections);
    // updatenotifications
    syncCheckbox(ui->updateCheckbox, "updatenotifications", defaults, makeConnections);
}

void OptionsDialog::syncCheckbox(QCheckBox *checkbox, const QString &key, bool defaults, bool makeConnection)
{
    auto val = qvApp->getSettingsManager().getBoolean(key, defaults);
    checkbox->setChecked(val);
    transientSettings.insert(key, val);

    if (makeConnection)
    {
        connect(checkbox, &QCheckBox::stateChanged, this, [this, key](int arg1) {
            modifySetting(key, static_cast<bool>(arg1));
        });
    }
}

void OptionsDialog::syncRadioButtons(QList<QRadioButton *> buttons, const QString &key, bool defaults, bool makeConnection)
{
    auto val = qvApp->getSettingsManager().getInteger(key, defaults);
    buttons.value(val)->setChecked(true);
    transientSettings.insert(key, val);

    if (makeConnection)
    {
        for (int i = 0; i < buttons.length(); i++)
        {
            connect(buttons.value(i), &QRadioButton::clicked, this, [this, key, i] {
                modifySetting(key, i);
            });
        }
    }
}

void OptionsDialog::syncComboBox(QComboBox *comboBox, const QString &key, bool defaults, bool makeConnection)
{
    auto val = qvApp->getSettingsManager().getInteger(key, defaults);
    comboBox->setCurrentIndex(val);
    transientSettings.insert(key, val);

    if (makeConnection)
    {
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, key](int index) {
            modifySetting(key, index);
        });
    }
}

void OptionsDialog::syncComboBoxData(QComboBox *comboBox, const QString &key, bool defaults, bool makeConnection)
{
    auto val = qvApp->getSettingsManager().getString(key, defaults);
    comboBox->setCurrentIndex(comboBox->findData(val));
    transientSettings.insert(key, val);

    if (makeConnection)
    {
        connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, key, comboBox](int index) {
            Q_UNUSED(index)
            modifySetting(key, comboBox->currentData());
        });
    }
}

void OptionsDialog::syncSpinBox(QSpinBox *spinBox, const QString &key, bool defaults, bool makeConnection)
{
    auto val = qvApp->getSettingsManager().getInteger(key, defaults);
    spinBox->setValue(val);
    transientSettings.insert(key, val);

    if (makeConnection)
    {
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, key](int arg1) {
            modifySetting(key, arg1);
        });
    }
}

void OptionsDialog::syncDoubleSpinBox(QDoubleSpinBox *doubleSpinBox, const QString &key, bool defaults, bool makeConnection)
{
    auto val = qvApp->getSettingsManager().getDouble(key, defaults);
    doubleSpinBox->setValue(val);
    transientSettings.insert(key, val);

    if (makeConnection)
    {
        connect(doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, key](double arg1) {
            modifySetting(key, arg1);
        });
    }
}

void OptionsDialog::syncShortcuts(bool defaults)
{
    qvApp->getShortcutManager().updateShortcuts();

    transientShortcuts.clear();
    const auto &shortcutsList = qvApp->getShortcutManager().getShortcutsList();
    ui->shortcutsTable->setRowCount(shortcutsList.length());

    for (int i = 0; i < shortcutsList.length(); i++)
    {
        const ShortcutManager::SShortcut &shortcut = shortcutsList.value(i);

        // Add shortcut to transient shortcut list
        if (defaults)
            transientShortcuts.append(shortcut.defaultShortcuts);
        else
            transientShortcuts.append(shortcut.shortcuts);

        // Add shortcut to table widget
        auto *nameItem = new QTableWidgetItem();
        nameItem->setText(shortcut.readableName);
        ui->shortcutsTable->setItem(i, 0, nameItem);

        auto *shortcutsItem = new QTableWidgetItem();
        shortcutsItem->setText(ShortcutManager::stringListToReadableString(
                                   transientShortcuts.value(i)));
        ui->shortcutsTable->setItem(i, 1, shortcutsItem);
    }
    updateShortcutsTable();
}

void OptionsDialog::updateShortcutsTable()
{
    for (int i = 0; i < transientShortcuts.length(); i++)
    {
        const QStringList &shortcuts = transientShortcuts.value(i);
        ui->shortcutsTable->item(i, 1)->setText(ShortcutManager::stringListToReadableString(shortcuts));
    }
    updateButtonBox();
}

void OptionsDialog::shortcutCellDoubleClicked(int row, int column)
{
    Q_UNUSED(column)
    auto *shortcutDialog = new ShortcutDialog(row, this);
    connect(shortcutDialog, &ShortcutDialog::shortcutsListChanged, this, [this](int index, const QStringList &stringListShortcuts) {
        transientShortcuts.replace(index, stringListShortcuts);
        updateShortcutsTable();
    });
    shortcutDialog->open();
}

void OptionsDialog::buttonBoxClicked(QAbstractButton *button)
{
    auto role = ui->buttonBox->buttonRole(button);
    if (role == QDialogButtonBox::AcceptRole || role == QDialogButtonBox::ApplyRole)
    {
        saveSettings();
        if (role == QDialogButtonBox::ApplyRole)
            button->setEnabled(false);
    }
    else if (role == QDialogButtonBox::ResetRole)
    {
        syncSettings(true);
        syncShortcuts(true);
    }
}

void OptionsDialog::updateButtonBox()
{
    QPushButton *defaultsButton = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    QPushButton *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
    defaultsButton->setEnabled(false);
    applyButton->setEnabled(false);

    // settings
    const QList<QString> settingKeys = transientSettings.keys();
    for (const auto &key : settingKeys)
    {
        const auto &transientValue = transientSettings.value(key);
        const auto &savedValue = qvApp->getSettingsManager().getSetting(key);
        const auto &defaultValue = qvApp->getSettingsManager().getSetting(key, true);

        if (transientValue != savedValue)
            applyButton->setEnabled(true);
        if (transientValue != defaultValue)
            defaultsButton->setEnabled(true);
    }

    // shortcuts
    const QList<ShortcutManager::SShortcut> &shortcutsList = qvApp->getShortcutManager().getShortcutsList();
    for (int i = 0; i < transientShortcuts.length(); i++)
    {
        const auto &transientValue = transientShortcuts.value(i);
        QStringList savedValue = shortcutsList.value(i).shortcuts;
        QStringList defaultValue = shortcutsList.value(i).defaultShortcuts;

        if (transientValue != savedValue)
            applyButton->setEnabled(true);
        if (transientValue != defaultValue)
            defaultsButton->setEnabled(true);
    }
}

void OptionsDialog::bgColorButtonClicked()
{

    auto *colorDialog = new QColorDialog(ui->bgColorButton->text(), this);
    colorDialog->setWindowModality(Qt::WindowModal);
    connect(colorDialog, &QDialog::accepted, colorDialog, [this, colorDialog] {
        auto selectedColor = colorDialog->currentColor();

        if (!selectedColor.isValid())
            return;

        modifySetting("bgcolor", selectedColor.name());
        ui->bgColorButton->setText(selectedColor.name());
        updateBgColorButton();
        colorDialog->deleteLater();
    });
    colorDialog->open();
}

void OptionsDialog::updateBgColorButton()
{
    QPixmap newPixmap = QPixmap(32, 32);
    newPixmap.fill(ui->bgColorButton->text());
    ui->bgColorButton->setIcon(QIcon(newPixmap));
}

void OptionsDialog::bgColorCheckboxStateChanged(int arg1)
{
    if (arg1 > 0)
        ui->bgColorButton->setEnabled(true);
    else
        ui->bgColorButton->setEnabled(false);

    updateBgColorButton();
}

void OptionsDialog::scalingCheckboxStateChanged(int arg1)
{
    if (arg1 > 0)
        ui->scalingTwoCheckbox->setEnabled(true);
    else
        ui->scalingTwoCheckbox->setEnabled(false);
}

void OptionsDialog::windowResizeComboBoxCurrentIndexChanged(int index)
{
    if (index == 0)
    {
        ui->minWindowResizeLabel->setEnabled(false);
        ui->minWindowResizeSpinBox->setEnabled(false);
        ui->maxWindowResizeLabel->setEnabled(false);
        ui->maxWindowResizeSpinBox->setEnabled(false);
    }
    else
    {
        ui->minWindowResizeLabel->setEnabled(true);
        ui->minWindowResizeSpinBox->setEnabled(true);
        ui->maxWindowResizeLabel->setEnabled(true);
        ui->maxWindowResizeSpinBox->setEnabled(true);
    }
}

void OptionsDialog::populateLanguages()
{
    ui->langComboBox->clear();

    ui->langComboBox->addItem(tr("System Language"), "system");

    // Put english at the top seperately because it has no file
    QLocale eng("en");
    ui->langComboBox->addItem("English (en)", "en");

    const auto entries = QDir(":/i18n/").entryList();
    for (auto entry : entries)
    {
        entry.remove(0, 6);
        entry.remove(entry.length()-3, 3);
        QLocale locale(entry);

        const QString langString = locale.nativeLanguageName() + " (" + entry + ")";

        ui->langComboBox->addItem(langString, entry);
    }
}

void OptionsDialog::languageComboBoxCurrentIndexChanged(int index)
{
    Q_UNUSED(index)
    if (!languageRestartMessageShown)
    {
        QMessageBox::information(this, tr("Restart Required"), tr("You must restart qView to change the language."));
        languageRestartMessageShown = true;
    }
}
