#include "itemdelegate.h"

#include <QPainter>
#include <QStyledItemDelegate>
#include <QStyle>
#include <QEvent>
#include <QDebug>
#include <QPainterPath>
#include "itemdef.h"
#include "../imagecore.h"

ItemDelegate::ItemDelegate(ImageCore* imageCore, QObject* parent) :
    QStyledItemDelegate(parent)
{
    this->imageCore = imageCore;
}

ItemDelegate::~ItemDelegate()
{

}

void ItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.isValid())
    {
        painter->save();
        QVariant variant = index.data(Qt::UserRole + 3);
        if (variant.isNull())
        {
            return;
        }
        ItemData data = variant.value<ItemData>();

        QStyleOptionViewItem viewOption(option);//用来在视图中画一个item

        QRectF rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width() - 1);
        rect.setHeight(option.rect.height() - 1);

        //QPainterPath画圆角矩形
        const qreal radius = 7;
        QPainterPath path;
        path.moveTo(rect.topRight() - QPointF(radius, 0));
        path.lineTo(rect.topLeft() + QPointF(radius, 0));
        path.quadTo(rect.topLeft(), rect.topLeft() + QPointF(0, radius));
        path.lineTo(rect.bottomLeft() + QPointF(0, -radius));
        path.quadTo(rect.bottomLeft(), rect.bottomLeft() + QPointF(radius, 0));
        path.lineTo(rect.bottomRight() - QPointF(radius, 0));
        path.quadTo(rect.bottomRight(), rect.bottomRight() + QPointF(0, -radius));
        path.lineTo(rect.topRight() + QPointF(0, radius));
        path.quadTo(rect.topRight(), rect.topRight() + QPointF(-radius, -0));

        if (option.state.testFlag(QStyle::State_Selected))
        {
            painter->setPen(QPen(Qt::blue));
            painter->setBrush(QColor(229, 241, 255));
            painter->drawPath(path);
        }
        else if (option.state.testFlag(QStyle::State_MouseOver))
        {
            painter->setPen(QPen(Qt::green));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(path);
        }
        else {
            painter->setPen(QPen(Qt::gray));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(path);
        }

        //绘制数据位置
        QRect NameRect = QRect(rect.left() + 10, rect.bottom() - 25, rect.width() - 30, 20);
        //QRect circle = QRect(NameRect.right(), rect.top() + 10, 10, 10);
        QRect pixmapRect = QRect(rect.left() + 2, rect.top() + 2, rect.width() - 2, rect.height() - 2);

        //painter->setBrush(Qt::blue);
        //painter->setPen(QPen(Qt::blue));

        //painter->drawEllipse(circle);     //画圆圈

        painter->setPen(QPen(Qt::black));
        painter->setFont(QFont("Times", 12, QFont::Bold));
        painter->drawText(NameRect, Qt::AlignLeft, data.fullName); //绘制名字

        //painter->setPen(QPen(Qt::gray));
        //painter->setFont(QFont("Times", 10));
        //painter->drawText(telRect, Qt::AlignLeft, data.tel); //绘制电话

        if (data.extension == "png" || data.extension == "dat")
        {
            ImageCore::ReadData image = imageCore->readFile(data.fullAbsolutePath, false);
            QPixmap pixmap = image.pixmap;
            if (pixmap.width() > 227 || pixmap.height() > 227)
            {
                pixmap = pixmap.scaled(234, 234, Qt::KeepAspectRatio);
            }
            painter->drawPixmap(pixmapRect, pixmap);

        }

        painter->restore();
    }
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize(210, 210);
}
