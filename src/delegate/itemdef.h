#ifndef ITEMDEF_H
#define ITEMDEF_H

#include <QMetaType>
#include <QPixmap>
#include <QFileInfo>

struct ThumbnailData{
    QString fullName;
    QString fullAbsolutePath;
    QString extension;
    QPixmap thumbnail;
    //QFileInfo fileInfo;
};

// 自定义数据类型需注册才能放入QVariant
Q_DECLARE_METATYPE(ThumbnailData);

#endif // ITEMDEF_H
