#include "itemdelegate.h"

#include <QPainter>
#include <QStyledItemDelegate>
#include <QStyle>
#include <QEvent>
#include <QMouseEvent>
#include <QPainterPath>
#include <QApplication>
#include <QFileSystemModel>
#include <QFileInfo>
#include <QFileIconProvider>
//#include <QtConcurrent/QtConcurrentRun>

#include "../filelistmodel/filefilterproxymodel.h"
#include "../config.h"
#include "../logger/Logger.h"


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
    if (!index.isValid())
    {
        return;
    }
    FileFilterProxyModel* model = (FileFilterProxyModel*)index.model();
    QFileInfo fileInfo = model->fileInfo(index);

    QPixmap thumbnail;

    if (fileInfo.suffix() == "png" || fileInfo.suffix() == "dat")
    {
        ImageCore::ReadData image = imageCore->readFileSize(fileInfo.filePath(), true, QSize(THUMBNAIL_WIDE, THUMBNAIL_HEIGHT));
        thumbnail = image.pixmap;
    }
    else {
        auto* iconProvider = (QFileIconProvider*)model->iconProvider();
        auto icon = iconProvider->icon(fileInfo);
        thumbnail = icon.pixmap(48, 48);
    }

    painter->save();
    //QVariant variant = index.data(Qt::UserRole + 3);
    //if (variant.isNull())
    //{
    //    return;
    //}
    //ItemData data = variant.value<ItemData>();

    // 用来在视图中画一个item
    QStyleOptionViewItem viewOption(option);

    QRectF rect;
    rect.setX(option.rect.x());
    rect.setY(option.rect.y());
    rect.setWidth(option.rect.width() - 1);
    rect.setHeight(option.rect.height() - 1);

    //QPainterPath画圆角矩形
    const qreal radius = 3;
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

    //画圆圈
    // QRect circle = QRect(NameRect.right(), rect.top() + 10, 10, 10);
    //painter->setBrush(Qt::blue);
    //painter->setPen(QPen(Qt::blue));
    //painter->drawEllipse(circle);

    // 绘制电话
    //QRect telRect = QRect(rect.left() + 10, rect.bottom() - 25, rect.width() - 10, 20);
    //painter->setPen(QPen(Qt::gray));
    //painter->setFont(QFont("Times", 10));
    //painter->drawText(telRect, Qt::AlignLeft, data.tel); 

    //画按钮
    {
        //左边距10
        QStyleOptionButton cbOpt;
        QRect checboxRec(rect.left() + 10, rect.top() + 4, 20, 20);
        cbOpt.rect = checboxRec;
        Qt::CheckState checkStat = Qt::CheckState(qvariant_cast<int>(index.data(Qt::CheckStateRole)));
        if (checkStat == Qt::CheckState::Checked)
        {
            cbOpt.state |= QStyle::State_On;
        }
        else
        {
            cbOpt.state |= QStyle::State_Off;
        }
        QApplication::style()->drawControl(QStyle::CE_RadioButton, &cbOpt, painter);
    }

    // 绘制缩略图
    //LOG_INFO << "thumbnail.width " << thumbnail.width() << " thumbnail.height " << thumbnail.height();
    QRect pixmapRect = QRect(
        rect.left() + (rect.width() - thumbnail.width()) / 2,
        rect.top() + (rect.height() - 50 - thumbnail.height() ) / 2, 
        thumbnail.width(),
        thumbnail.height());
    painter->drawPixmap(pixmapRect, thumbnail);
    //LOG_INFO << "pixmapRect.left " << pixmapRect.left() << " pixmapRect.top " << pixmapRect.top();

    //绘制名字
    QRect NameRect = QRect(rect.left() + 10, rect.bottom() - 25, rect.width() - 30, 20);
    painter->setPen(QPen(Qt::black));
    painter->setFont(QFont("Fixedsys", 12));
    painter->drawText(NameRect, Qt::AlignLeft, fileInfo.fileName());
    //LOG_INFO << "NameRect.left " << NameRect.left() << " NameRect.top " << NameRect.top();
    painter->restore();
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize(THUMBNAIL_WIDE + 4, THUMBNAIL_HEIGHT + 100);
}

bool ItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    QRect rect = option.rect;

    // 对应上面画的checkbox的retc
    QRect checboxRec(rect.left() + 10, rect.top() + 4, 20, 20);

    // 按钮点击事件；
    QMouseEvent* mevent = static_cast<QMouseEvent*>(event);
    if (checboxRec.contains(mevent->pos()) && event->type() == QEvent::MouseButtonPress)
    {
        Qt::CheckState checked = Qt::CheckState(qvariant_cast<int>(index.data(Qt::CheckStateRole)));
        model->setData(index, checked == Qt::CheckState::Checked ? Qt::CheckState::Unchecked : Qt::CheckState::Checked, Qt::CheckStateRole);
        model->dataChanged(index, index);
        //此处可以添加自定义信号，即使checbox点击信号；
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
