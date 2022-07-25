#ifndef ABOUT_MESSAGEBOX_H
#define ABOUT_MESSAGEBOX_H

#include <QMessageBox>

class AboutDialog : public QMessageBox
{
    Q_OBJECT
public:
    static void show(QWidget* parent, const QString& title, const QString& component, const QString& version);
private:
    explicit AboutDialog(QWidget* parent = nullptr);
    AboutDialog(Icon icon, const QString& title, const QString& text, StandardButtons buttons = NoButton, QWidget* parent = nullptr,
        Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    virtual ~AboutDialog();

    Q_DISABLE_COPY(AboutDialog)
};


#endif // ABOUT_MESSAGEBOX_H
