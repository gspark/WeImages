#ifndef WXBAR_H
#define WXBAR_H

#include <QObject>

class QMouseEvent;
class QLabel;
//class QToolButton;
class QToolBar;
class QAction;


class WxBarPrivate;

class WxBar : public QObject
{
    Q_OBJECT
public:
    explicit WxBar(QWidget *parent);
    ~WxBar();

    QWidget *mainWidget() const;
    QLabel *titleLabel() const;
    QLabel*logoButton() const;
    QWidget *sysToolBar() const;

    void addAction(QAction *action);

    void setCloseIcon(QIcon &icon);
    void setMaximizeIcon(QIcon &icon);
    void setMinimizeIcon(QIcon &icon);
    void setNormalIcon(QIcon &icon);
    void setIconDark(bool dark);

    void setWidgetResizable(bool resizable);
    void setWidgetMovable(bool movable);
    void setWidgetMaximizable(bool maximizable);

    void updateWidgetFlags();

    void setTitleBar(QWidget *widget);

signals:
    void sigWidgetResizable(bool resizable);
    //void sigWidgetMovable(bool movable);

protected:
    virtual bool eventFilter(QObject* object, QEvent* event);

private:
    WxBarPrivate *d;
};

#endif // WXBAR_H
