#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include "../imagecore.h"

#include <QStyledItemDelegate>
#include <QModelIndex>

class ImageCore;

class ItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
signals:

public:
    explicit ItemDelegate(ImageCore* imageCore, QObject *parent = 0);
    ~ItemDelegate();

    //重写重画函数
    void paint(QPainter * painter,const QStyleOptionViewItem & option,const QModelIndex & index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
    //处理鼠标事件
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
private:
    ImageCore* imageCore;
};

#endif // ITEMDELEGATE_H
