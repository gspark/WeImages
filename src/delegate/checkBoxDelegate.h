#pragma once

#include <QStyledItemDelegate>

class CheckBoxDelegate: public QStyledItemDelegate
{
    Q_OBJECT
signals:

public:
    explicit CheckBoxDelegate( QObject* parent = 0);
    ~CheckBoxDelegate();

    //重写重画函数
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
 
protected:
    //处理鼠标事件
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
  
};

