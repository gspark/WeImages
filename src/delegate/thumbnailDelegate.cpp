#include "thumbnailDelegate.h"

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
#include <QRadioButton>
#include <QGraphicsColorizeEffect>
//#include <QtConcurrent/QtConcurrentRun>

#include "thumbnailData.h"
#include "../filelistmodel/filefilterproxymodel.h"
#include "../config.h"
#include "../imagecore.h"
#include "../logger/Logger.h"


ThumbnailDelegate::ThumbnailDelegate(ImageCore* imageCore, QObject* parent) :
    QStyledItemDelegate(parent)
{
    this->imageCore = imageCore;
}

ThumbnailDelegate::~ThumbnailDelegate()
{

}

void ThumbnailDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return;
    }

    QVariant variant = index.data(Qt::UserRole + 3);
    if (variant.isNull())
    {
        return;
    }
    ThumbnailData data = variant.value<ThumbnailData>();

    painter->save();

    QRectF rect;
    rect.setX(option.rect.x());
    rect.setY(option.rect.y());
    rect.setWidth(option.rect.width() - 1);
    rect.setHeight(option.rect.height() - 1);

    //QPainterPath画圆角矩形
    const qreal radius = 2;
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

    if (option.state.testFlag(QStyle::State_MouseOver))
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

    //绘制按钮
    QStyleOptionButton cbOpt;
    QRect buttonRect(rect.left() + 10, rect.top() + 4, 20, 20);
    cbOpt.rect = buttonRect;
    Qt::CheckState checkStat = Qt::CheckState(qvariant_cast<int>(index.data(Qt::CheckStateRole)));
   
    cbOpt.state |= checkStat == Qt::CheckState::Checked ? QStyle::State_On : QStyle::State_Off;
    if (data.isWeChatImage)
    {
        cbOpt.state |= QStyle::State_Enabled;
    }
    QApplication::style()->drawControl(QStyle::CE_RadioButton, &cbOpt, painter, option.widget);

    // 绘制缩略图
    //LOG_INFO << "thumbnail.width " << thumbnail.width() << " thumbnail.height " << thumbnail.height();
    QRect pixmapRect = QRect(
        rect.left() + (rect.width() - data.thumbnail.width()) / 2,
        rect.top() + (rect.height() - 50 - data.thumbnail.height() ) / 2,
        data.thumbnail.width(),
        data.thumbnail.height());
    painter->drawPixmap(pixmapRect, data.thumbnail);
    //LOG_INFO << "pixmapRect.left " << pixmapRect.left() << " pixmapRect.top " << pixmapRect.top();

    //绘制名字
    QRect NameRect = QRect(rect.left() + 10, rect.bottom() - 25, rect.width() - 30, 20);
    painter->setPen(QPen(Qt::black));
    painter->setFont(QFont("Fixedsys", 12));
    painter->drawText(NameRect, Qt::AlignLeft, data.fileName);
    //LOG_INFO << "NameRect.left " << NameRect.left() << " NameRect.top " << NameRect.top();
    painter->restore();
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize(THUMBNAIL_WIDE + 4, THUMBNAIL_HEIGHT + 100);
}

QWidget* ThumbnailDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // 内容不可编辑
    return nullptr;
}

bool ThumbnailDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    QVariant variant = index.data(Qt::UserRole + 3);
    if (variant.isNull())
    {
        return false;
    }
    ThumbnailData data = variant.value<ThumbnailData>();
    if (!data.isWeChatImage)
    {
        return false;
    }

    QRect rect = option.rect;

    // 对应上面paint画的的button
    QRect buttonRect(rect.left() + 10, rect.top() + 4, 20, 20);

    // 按钮点击事件；
    QMouseEvent* mevent = static_cast<QMouseEvent*>(event);
    if (buttonRect.contains(mevent->pos()) && event->type() == QEvent::MouseButtonPress)
    {
        Qt::CheckState checked = Qt::CheckState(qvariant_cast<int>(index.data(Qt::CheckStateRole)));
        model->setData(index, checked == Qt::CheckState::Checked ? Qt::CheckState::Unchecked : Qt::CheckState::Checked, Qt::CheckStateRole);
        model->dataChanged(index, index);
        //此处可以添加自定义信号，即使checbox点击信号；
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
