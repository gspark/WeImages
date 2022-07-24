#ifndef THUMBNAILDELEGATE_H 
#define THUMBNAILDELEGATE_H

#include <QStyledItemDelegate>
#include <QModelIndex>

class ImageCore;

class ThumbnailDelegate : public QStyledItemDelegate
{
    Q_OBJECT
signals:

public:
    explicit ThumbnailDelegate(ImageCore* imageCore, QObject *parent = 0);
    ~ThumbnailDelegate();

    //重写重画函数
    void paint(QPainter * painter,const QStyleOptionViewItem & option,const QModelIndex & index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
protected:
    //处理鼠标事件
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    ImageCore* _imageCore;
};

#endif // THUMBNAILDELEGATE_H
