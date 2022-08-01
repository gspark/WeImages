#pragma once

#include <QObject>

class WxBar;
class QStatusBar;
class QMenuBar;
class QHBoxLayout;

class WxWindowPrivate : public QObject
{
    Q_OBJECT
public:
    WxWindowPrivate();
    ~WxWindowPrivate();

    WxBar* m_wxBar;
    QStatusBar* m_statusBar;
    QMenuBar* m_menuBar;
    QHBoxLayout* m_layout;

public slots:
    void slotResizable(bool resizable);
};


