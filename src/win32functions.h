#ifndef WIN32FUNCTIONS_H
#define WIN32FUNCTIONS_H

#include "openwith.h"

#include <qfileiconprovider.h>

class QVWin32Functions
{
public:
    static QList<OpenWith::OpenWithItem> getOpenWithItems(const QString &filePath);

    static void openWithInvokeAssocHandler(const QString &filePath, void *winAssocHandler);

    static void showOpenWithDialog(const QString &filePath, const QWindow *parent);

};

#endif // WIN32FUNCTIONS_H
