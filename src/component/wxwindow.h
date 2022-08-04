#ifndef WX_WINDOW_H
#define WX_WINDOW_H

#include <QMainWindow>

class WxBar;
class WxWindowPrivate;
class QPushButton;

class WxWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit WxWindow(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~WxWindow();

    WxBar *wxBar() const;

    QMenuBar *menuBar() const;
    void setMenuBar(QMenuBar *menuBar);

    QStatusBar *statusBar() const;
    void setStatusBar(QStatusBar *statusbar);

    void setFixedSize(const QSize &);
    void setFixedSize(int w, int h);
    void setFixedWidth(int w);
    void setFixedHeight(int h);

    void setWindowFlags(Qt::WindowFlags type);

    QToolBar* toolBar() const;
    void setToolBar(QToolBar* toolBar);
private:
    WxWindowPrivate *d;
};

#endif // WX_WINDOW_H
