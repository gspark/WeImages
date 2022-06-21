#ifndef ITEMDEF_H
#define ITEMDEF_H

#include <QMetaType>


struct ItemData{
    QString fullName;
    QString fullAbsolutePath;
    QString extension;
};

Q_DECLARE_METATYPE(ItemData);

#endif // ITEMDEF_H
