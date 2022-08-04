#include "wxWindowPrivate.h"

#include <QHBoxLayout>
#include <QStatusBar>
#include <QMenuBar>

WxWindowPrivate::WxWindowPrivate():
    m_wxBar(nullptr),
    m_statusBar(nullptr),
    m_menuBar(nullptr),
    m_layout(nullptr),
    m_toolbar(nullptr)
{
}

WxWindowPrivate::~WxWindowPrivate()
{
    m_statusBar = nullptr;
}

void WxWindowPrivate::slotResizable(bool resizable)
{
    if (m_statusBar) {
        m_statusBar->setSizeGripEnabled(resizable);
    }
}
