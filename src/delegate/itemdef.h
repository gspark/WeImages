#ifndef ITEMDEF_H
#define ITEMDEF_H

#include <QMetaType>
#include <QPixmap>

struct ItemData{
    QString fullName;
    QString fullAbsolutePath;
    QString extension;
    QPixmap thumbnail;
};

// 自定义数据类型需注册才能放入QVariant
Q_DECLARE_METATYPE(ItemData);

#endif // ITEMDEF_H
