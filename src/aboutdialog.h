#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class QVAboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QVAboutDialog(QWidget *parent = nullptr);
    ~QVAboutDialog() override;

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
