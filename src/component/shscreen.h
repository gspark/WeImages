#ifndef SH_SCREEN_H
#define SH_SCREEN_H

#include <QRect>
#include <QList>

class ShScreen
{
public:
    ShScreen();

    int currentScreen(const int x);
    QRect screenRect(const int current);

    static QRect normalRect();

private:
    QList<QRect> m_screenRects;
};

#endif // SH_SCREEN_H
