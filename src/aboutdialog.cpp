#include "aboutdialog.h"

#include <QDateTime>

AboutDialog::AboutDialog(QWidget* parent)
    : QMessageBox(parent)
{
}

AboutDialog::AboutDialog(Icon icon, const QString& title, const QString& text, 
                       StandardButtons buttons, QWidget* parent, Qt::WindowFlags f)
    : QMessageBox(icon, title, text, buttons, parent, f)
{
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::show(QWidget* parent, const QString& title, const QString& component, const QString& version)
{
    QString translatedTextAboutQtnCaption = QMessageBox::tr(
        "<h3>%1</h3>"
        "<p></p>"
        "<p></p>"
        "<p>Version:&nbsp;%2</p>"
        "<p>Author:&nbsp;&nbsp;shrill</p>"
        "<p>Date:&nbsp;&nbsp;&nbsp;&nbsp;%3</p>"
        "<p></p>"
        "<p>github:&nbsp;&nbsp;<a href=\"https://github.com/gspark/WeImages\">https://github.com/gspark/WeImages</a></p>"
    ).arg(component).arg(version).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    AboutDialog* msgBox = new AboutDialog(parent);

    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setWindowTitle(title.isEmpty() ? tr("About") + " " + component : title);
    msgBox->setText(translatedTextAboutQtnCaption);

    QPixmap pm(QLatin1String(":/res/qtitan.png"));
    if (!pm.isNull())
        msgBox->setIconPixmap(pm);
    msgBox->setTextInteractionFlags(Qt::TextSelectableByMouse);
    msgBox->exec();
}
