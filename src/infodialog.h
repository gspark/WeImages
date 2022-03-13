#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <QLocale>

namespace Ui {
class InfoDialog;
}

class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(QWidget *parent = nullptr);
    ~InfoDialog();

    void setInfo(const QFileInfo &value, const int &value2, const int &value3, const int &value4);

    void updateInfo();

private:
    Ui::InfoDialog *ui;

    QFileInfo selectedFileInfo;
    int width;
    int height;

    int frameCount;

public:
    // If Qt 5.10 is available, the built-in function will be used--for Qt 5.9, a custom solution will be used
    static QString formatBytes(qint64 bytes)
    {
        QString sizeString;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        QLocale locale;
        sizeString = locale.formattedDataSize(bytes);
#else
        double size = bytes;

        int reductionAmount = 0;
        double newSize = size/1024;
        while (newSize > 1024)
        {
            newSize /= 1024;
            reductionAmount++;
            if (reductionAmount > 2)
                break;
        }

        QString unit;
        switch (reductionAmount)
        {
        case 0: {
            unit = " KiB";
            break;
        }
        case 1: {
            unit = " MiB";
            break;
        }
        case 2: {
            unit = " GiB";
            break;
        }
        case 3: {
            unit = " TiB";
            break;
        }
        }

        sizeString = QString::number(newSize, 'f', 2) + unit;
#endif
        return sizeString;
    }
};

#endif // INFODIALOG_H
