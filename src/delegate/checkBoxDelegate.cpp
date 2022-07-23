#include "checkBoxDelegate.h"
#include "..\filelistmodel\filelistmodel.h"
#include "..\delegate\thumbnailData.h"

#include <QRadioButton>
#include <QApplication>
#include <QMouseEvent>
//#include <QPainter>

CheckBoxDelegate::CheckBoxDelegate(QObject* parent /*= 0*/)
{

}

CheckBoxDelegate::~CheckBoxDelegate()
{

}

void CheckBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    if (option.state.testFlag(QStyle::State_HasFocus))
        viewOption.state = viewOption.state ^ QStyle::State_HasFocus;

    if (index.column() == CheckBoxColumn)
    {
        Qt::CheckState checkStat = Qt::CheckState(qvariant_cast<int>(index.data(Qt::CheckStateRole)));

        QStyleOptionButton checkBoxStyle;
        checkBoxStyle.state = checkStat == Qt::CheckState::Checked ? QStyle::State_On : QStyle::State_Off;
        checkBoxStyle.state |= QStyle::State_Enabled;
        checkBoxStyle.iconSize = QSize(20, 20);
        checkBoxStyle.rect = option.rect;

        QRadioButton checkBox;
        checkBoxStyle.iconSize = QSize(20, 20);
        checkBoxStyle.rect = option.rect;
        QApplication::style()->drawPrimitive(QStyle::PE_IndicatorRadioButton, &checkBoxStyle, painter, &checkBox);
    }
    else {
        QStyledItemDelegate::paint(painter, viewOption, index);
    }
}

bool CheckBoxDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
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

    QRect decorationRect = option.rect;

    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    if (decorationRect.contains(mouseEvent->pos()) && index.column() == CheckBoxColumn)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            Qt::CheckState checkStat = Qt::CheckState(qvariant_cast<int>(index.data(Qt::CheckStateRole)));
            model->setData(index, checkStat == Qt::CheckState::Checked ? Qt::CheckState::Unchecked : Qt::CheckState::Checked, Qt::CheckStateRole);
        }
        else if (event->type() == QEvent::MouseButtonDblClick)
        {
            return false;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
