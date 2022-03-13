#ifndef SHORTCUTDIALOG_H
#define SHORTCUTDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include "shortcutmanager.h"

namespace Ui {
class ShortcutDialog;
}

class ShortcutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShortcutDialog(int index, QWidget *parent = nullptr);
    ~ShortcutDialog() override;

    QString shortcutAlreadyBound(const QKeySequence &chosenSequence, const QString &exemptShortcut);
    void acceptValidated();

signals:
    void shortcutsListChanged(int index, QStringList shortcutsStringList);

private slots:
    void buttonBoxClicked(QAbstractButton *button);

private:
    void done(int r) override;

    Ui::ShortcutDialog *ui;

    ShortcutManager::SShortcut shortcutObject;
    int index;
};

#endif // SHORTCUTDIALOG_H
