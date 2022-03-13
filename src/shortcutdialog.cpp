#include "shortcutdialog.h"
#include "ui_shortcutdialog.h"
#include "application.h"

#include <QMessageBox>

#include <QDebug>

ShortcutDialog::ShortcutDialog(int index, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShortcutDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint | Qt::CustomizeWindowHint));

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &ShortcutDialog::buttonBoxClicked);

    shortcutObject = qvApp->getShortcutManager().getShortcutsList().value(index);
    this->index = index;
    ui->keySequenceEdit->setKeySequence(shortcutObject.shortcuts.join(", "));
}

ShortcutDialog::~ShortcutDialog()
{
    delete ui;
}

void ShortcutDialog::done(int r)
{
    if (r == QDialog::Accepted)
    {
        return;
    }

    QDialog::done(r);
}

void ShortcutDialog::buttonBoxClicked(QAbstractButton *button)
{
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
    {
        QStringList shortcutsStringList = ui->keySequenceEdit->keySequence().toString().split(", ");
        const auto sequenceList = ShortcutManager::stringListToKeySequenceList(shortcutsStringList);

        for (const auto &sequence : sequenceList)
        {
            auto conflictingShortcut = shortcutAlreadyBound(sequence, shortcutObject.name);
            if (!conflictingShortcut.isEmpty())
            {
                QString nativeShortcutString = sequence.toString(QKeySequence::NativeText);
                QMessageBox::warning(this, tr("Shortcut Already Used"), tr("\"%1\" is already bound to \"%2\"").arg(nativeShortcutString, conflictingShortcut));
                return;
            }
        }

        acceptValidated();

        emit shortcutsListChanged(index, shortcutsStringList);
    }
    else if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole)
    {
        ui->keySequenceEdit->setKeySequence(shortcutObject.defaultShortcuts.join(", "));
    }
}

QString ShortcutDialog::shortcutAlreadyBound(const QKeySequence &chosenSequence, const QString &exemptShortcut)
{
    if (chosenSequence.isEmpty())
        return "";

    const auto &shortcutsList = qvApp->getShortcutManager().getShortcutsList();
    for (const auto &shortcut : shortcutsList)
    {
        auto sequenceList = ShortcutManager::stringListToKeySequenceList(shortcut.shortcuts);

        if (sequenceList.contains(chosenSequence) && shortcut.name != exemptShortcut)
            return shortcut.readableName;
    }
    return "";
}

void ShortcutDialog::acceptValidated()
{
    QDialog::done(1);
}
