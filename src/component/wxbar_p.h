#ifndef WXBAR_P_H
#define WXBAR_P_H

#include <QObject>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QIcon>

class QLabel;
//class QToolButton;
class QToolBar;


class WxBar;

class WxCursor
{
public:
    WxCursor();

    void reset();
    void recalculate(const QPoint &gMousePos, const QRect &frameRect);

public:
    bool m_bOnEdges              : true;
    bool m_bOnLeftEdge           : true;
    bool m_bOnRightEdge          : true;
    bool m_bOnTopEdge            : true;
    bool m_bOnBottomEdge         : true;
    bool m_bOnTopLeftEdge        : true;
    bool m_bOnBottomLeftEdge     : true;
    bool m_bOnTopRightEdge       : true;
    bool m_bOnBottomRightEdge    : true;

    int m_nBorderWidth          : 5;
};

class WxBarPrivate : public QObject
{
    Q_OBJECT
public:
    WxBarPrivate();

    void init();
    void installWidget(QWidget *w);

    void setIconDark(bool dark);

    void handleWidgetMouseEvent(QObject *obj, QEvent *event);

    // window
    bool windowTitleChange(QObject *obj);
    bool windowIconChange(QObject *obj);
    void windowSizeChange(QObject *obj);
    void windowStateChange(QObject *obj);

    void updateWindowButtons();

private:
    // mouse event
    void handleMousePressEvent(QMouseEvent *event);
    void handleMouseReleaseEvent(QMouseEvent *event);
    void handleMouseMoveEvent(QMouseEvent *event);
    void handleLeaveEvent(QEvent *event);
    void handleHoverMoveEvent(QHoverEvent *event);
    void handleMouseDblClickEvent(QMouseEvent *event);

    // widget action
    void restoreWidget(QWidget *pWidget);
    void maximizeWidget(QWidget *pWidget);
    void resizeWidget(const QPoint &gMousePos);
    void moveWidget(const QPoint &gMousePos);

    void doubleMoveWidget();

    // mouse shape
    void updateCursorShape(const QPoint &gMousePos);
    QPoint calcDragPoint(QWidget *pWindow, QMouseEvent *event) const;

    QRect validDragRect();

private slots:
    void slotSysButtonClicked();

public:
    WxBar *q;
    QWidget *m_titleBar;

    QLabel*m_logoButton;
    QLabel *m_titleLabel;
    QToolBar *m_toolBar;
    QAction *m_closeAction;
    QAction *m_maximizeAction;
    QAction *m_minimizeAction;
    QIcon m_closeIcon;
    QIcon m_minimizeIcon;
    QIcon m_maximizeIcon;
    QIcon m_normalIcon;

    QWidget *m_mainWidget;
    QPoint m_dragPos;
    WxCursor m_pressCursor;
    WxCursor m_moveCursor;
    bool m_bEdgePressed;
    bool m_bCursorShapeChanged;
    Qt::WindowFlags m_windowFlags;
    bool m_bWidgetResizable;
    bool m_bWidgetMovable;
    bool m_bWidgetMaximizable;

    bool m_isMaximized;
    bool m_isMinimized;

    // screen
    int m_currentScreen;

    QRect m_normalRect;
    QPoint m_movePoint;
    QPoint m_dragPoint;
    QPoint m_mousePoint;
    bool m_bLeftButtonPressed;
    bool m_bLeftButtonDbClicked;
    bool m_bLeftButtonTitlePressed;

    int sz;
};

#endif // WXBAR_P_H
