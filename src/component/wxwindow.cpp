#include "wxwindow.h"

#include <QLabel>
#include <QToolButton>
#include <QToolBar>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QMenuBar>

#include "wxbar.h"
#include "shscreen.h"
#include "wxWindowPrivate.h"
#include "..\iconhelper.h"


WxWindow::WxWindow(QWidget *parent, Qt::WindowFlags f)
    : QMainWindow(parent), d(new WxWindowPrivate())
{
    QMainWindow::setWindowFlags(Qt::FramelessWindowHint |
                   Qt::WindowSystemMenuHint |
                   Qt::WindowMinimizeButtonHint |
                   Qt::WindowMaximizeButtonHint |
                   Qt::Window | f
                  );
    d->m_wxBar = new WxBar(this);
    QObject::connect(d->m_wxBar, SIGNAL(sigWidgetResizable(bool)),
                     d, SLOT(slotResizable(bool)));

    d->m_layout = new QHBoxLayout();
    d->m_layout->setSpacing(0);
    d->m_layout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(2, 0, 2, 0);
    mainLayout->setSpacing(1);
    mainLayout->addWidget(d->m_wxBar->logoButton(), 0, Qt::AlignCenter);
    mainLayout->addLayout(d->m_layout);
    mainLayout->addStretch();
    mainLayout->addWidget(d->m_wxBar->titleLabel(), 0, Qt::AlignCenter);
    mainLayout->addStretch();
    mainLayout->addWidget(d->m_wxBar->sysToolBar(), 0, Qt::AlignTop);

    QWidget* menuWidget = new QWidget(this);
    menuWidget->setLayout(mainLayout);
    menuWidget->setObjectName(QLatin1String("shrill_TitleBar"));
    d->m_wxBar->setTitleBar(menuWidget);

    setMenuWidget(menuWidget);

    setMouseTracking(true);
    QRect geom = ShScreen::normalRect();
    resize(geom.width(), geom.height());
    raise();
    activateWindow();

    setObjectName(QLatin1String("shrill_window"));


   this->setWindowIcon(QIcon(IconHelper::getInstance().getPixmap(61550)));
    
}

WxWindow::~WxWindow()
{
    delete d;
}

WxBar*WxWindow::wxBar() const
{
    return d->m_wxBar;
}

QMenuBar *WxWindow::menuBar() const
{
    if (d->m_menuBar == nullptr) {
        d->m_menuBar = new QMenuBar();
#if !defined(Q_OS_WIN)
        d->m_menuBar->setNativeMenuBar(false);
        d->m_menuBar->setVisible(true);
#endif
        d->m_layout->addWidget(d->m_menuBar, 0, Qt::AlignCenter);
        //d->m_layout->addStretch();
    }
    d->m_menuBar->setStyleSheet(menuBarStyle);
    return d->m_menuBar;
}

void WxWindow::setMenuBar(QMenuBar *menuBar)
{
    if (d->m_menuBar == menuBar || menuBar == nullptr) {
        return;
    }
    if (d->m_menuBar) {
        d->m_layout->removeWidget(d->m_menuBar);
        delete d->m_menuBar;
        d->m_menuBar = menuBar;
        d->m_layout->insertWidget(0, d->m_menuBar, 0, Qt::AlignCenter);
    } else {
        d->m_menuBar = menuBar;
        d->m_layout->addWidget(d->m_menuBar, 0, Qt::AlignCenter);
        d->m_layout->addStretch();
    }
#if !defined(Q_OS_WIN)
    d->m_menuBar->setNativeMenuBar(false);
    d->m_menuBar->setVisible(true);
#endif
    d->m_menuBar->setStyleSheet(menuBarStyle);
}

QStatusBar *WxWindow::statusBar() const
{
    d->m_statusBar = QMainWindow::statusBar();
    return d->m_statusBar;
}

void WxWindow::setStatusBar(QStatusBar *statusbar)
{
    d->m_statusBar = statusbar;
    QMainWindow::setStatusBar(statusbar);
}

void WxWindow::setFixedSize(const QSize &s)
{
    setFixedSize(s.width(), s.height());
}

void WxWindow::setFixedSize(int w, int h)
{
    d->m_wxBar->setWidgetResizable(false);
    QWidget::setFixedSize(w, h);
}

void WxWindow::setFixedWidth(int w)
{
    setFixedSize(w, this->height());
}

void WxWindow::setFixedHeight(int h)
{
    setFixedSize(this->width(), h);
}

void WxWindow::setWindowFlags(Qt::WindowFlags type)
{
    QMainWindow::setWindowFlags(type);
    d->m_wxBar->updateWidgetFlags();
}

QToolBar* WxWindow::toolBar() const
{
    if (d->m_toolbar == nullptr) {
        d->m_toolbar = new QToolBar();
#if !defined(Q_OS_WIN)
        d->m_toolbar->setVisible(true);
#endif
        d->m_layout->addSpacing(6);
        d->m_layout->addWidget(d->m_toolbar, 0, Qt::AlignCenter);
    }
    return d->m_toolbar;
}

void WxWindow::setToolBar(QToolBar* toolBar)
{
    if (d->m_toolbar == toolBar || toolBar == nullptr) {
        return;
    }
    if (d->m_toolbar) {
        d->m_layout->removeWidget(d->m_toolbar);
        delete d->m_toolbar;
    }
    d->m_toolbar = toolBar;
    d->m_layout->addSpacing(6);
    //d->m_layout->insertWidget(0, d->m_toolbar, 0, Qt::AlignCenter);
    d->m_layout->addWidget(d->m_toolbar, 0, Qt::AlignCenter);

#if !defined(Q_OS_WIN)
    d->m_toolbar->setVisible(true);
#endif
}

