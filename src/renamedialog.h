#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include <QInputDialog>
#include <QFileInfo>

class RenameDialog : public QInputDialog
{
    Q_OBJECT
public:
    RenameDialog(QWidget *parent, QFileInfo fileInfo);

    void onFinished(int result);

signals:
    void newFileToOpen(const QString &filePath);
    void readyToRenameFile();

protected:
    void showEvent(QShowEvent *event) override;

private:
    QFileInfo fileInfo;
};

#endif // RENAMEDIALOG_H
