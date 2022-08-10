﻿#ifndef ICONHELPER_H
#define ICONHELPER_H

#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
#include <QtWidgets>
#endif

class IconHelper : public QObject
{
    Q_OBJECT

private:
    //默认构造函数,传入字体文件+字体名称
    explicit IconHelper(const QString& fontFile, const QString& fontName, QObject* parent = 0);

    ////FontAwesome6图形字体类
    //static IconHelper *iconFontAwesome6;

public:
    //样式颜色结构体
    struct StyleColor {
        QString position;           //位置 left right top bottom
        bool defaultBorder;         //默认有边框

        quint32 iconSize;           //图标字体尺寸
        quint32 iconWidth;          //图标图片宽度
        quint32 iconHeight;         //图标图片高度

        quint32 borderWidth;        //边框宽度
        QString borderColor;        //边框颜色

        QString normalBgColor;      //正常背景颜色
        QString normalTextColor;    //正常文字颜色
        QString hoverBgColor;       //悬停背景颜色
        QString hoverTextColor;     //悬停文字颜色
        QString pressedBgColor;     //按下背景颜色
        QString pressedTextColor;   //按下文字颜色
        QString checkedBgColor;     //选中背景颜色
        QString checkedTextColor;   //选中文字颜色

        StyleColor() {
            position = "left";
            defaultBorder = false;

            iconSize = 12;
            iconWidth = 15;
            iconHeight = 15;

            borderWidth = 3;
            borderColor = "#029FEA";

            normalBgColor = "#292F38";
            normalTextColor = "#54626F";
            hoverBgColor = "#40444D";
            hoverTextColor = "#FDFDFD";
            pressedBgColor = "#404244";
            pressedTextColor = "#FDFDFD";
            checkedBgColor = "#44494F";
            checkedTextColor = "#FDFDFD";
        }

        //设置常规颜色 普通状态+加深状态
        void setColor(const QString &normalBgColor,
                      const QString &normalTextColor,
                      const QString &darkBgColor,
                      const QString &darkTextColor) {
            this->normalBgColor = normalBgColor;
            this->normalTextColor = normalTextColor;
            this->hoverBgColor = darkBgColor;
            this->hoverTextColor = darkTextColor;
            this->pressedBgColor = darkBgColor;
            this->pressedTextColor = darkTextColor;
            this->checkedBgColor = darkBgColor;
            this->checkedTextColor = darkTextColor;
        }
    };

    static IconHelper& getInstance();

    //获取图形字体
    QFont getIconFont();

    //设置图形字体到标签
     void setIcon(QLabel *lab, int icon, quint32 size = 12);
    //设置图形字体到按钮
     void setIcon(QAbstractButton *btn, int icon, quint32 size = 12);

    //设置图形字体到图标
     void setPixmap(QAbstractButton *btn, const QColor &color,
                          int icon, quint32 size = 12,
                          quint32 width = 15, quint32 height = 15,
                          int flags = Qt::AlignCenter);
    //获取指定图形字体,可以指定文字大小,图片宽高,文字对齐
     QPixmap getPixmap(const QColor &color, int icon, quint32 size = 12,
                             quint32 width = 15, quint32 height = 15,
                             int flags = Qt::AlignCenter);

     //获取指定图形字体,可以指定文字大小,图片宽高,文字对齐
     QPixmap getPixmap(int icon, quint32 size = 12, quint32 width = 16, quint32 height = 16, int flags = Qt::AlignCenter, const QColor& color = StyleColor().normalBgColor);

    //指定导航面板样式,带图标和效果切换+悬停颜色+按下颜色+选中颜色
     void setStyle(QWidget *widget, QList<QPushButton *> btns, QList<int> icons, const StyleColor &styleColor);
     void setStyle(QWidget *widget, QList<QToolButton *> btns, QList<int> icons, const StyleColor &styleColor);
     void setStyle(QWidget *widget, QList<QAbstractButton *> btns, QList<int> icons, const StyleColor &styleColor);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QFont iconFont;                 //图形字体
    QList<QAbstractButton *> btns;  //按钮队列
    QList<QPixmap> pixNormal;       //正常图片队列
    QList<QPixmap> pixHover;        //悬停图片队列
    QList<QPixmap> pixPressed;      //按下图片队列
    QList<QPixmap> pixChecked;      //选中图片队列

private slots:
    //按钮选中状态切换处理
    void toggled(bool checked);

private:

    //设置图形字体到标签
    void setIcon1(QLabel *lab, int icon, quint32 size = 12);
    //设置图形字体到按钮
    void setIcon1(QAbstractButton *btn, int icon, quint32 size = 12);

    //设置图形字体到图标
    void setPixmap1(QAbstractButton *btn, const QColor &color,
                    int icon, quint32 size = 12,
                    quint32 width = 15, quint32 height = 15,
                    int flags = Qt::AlignCenter);
    //获取指定图形字体,可以指定文字大小,图片宽高,文字对齐
    QPixmap getPixmap1(const QColor &color, int icon, quint32 size = 12,
                       quint32 width = 15, quint32 height = 15,
                       int flags = Qt::AlignCenter);

    //指定导航面板样式,带图标和效果切换+悬停颜色+按下颜色+选中颜色
    void setStyle1(QWidget *widget, QList<QPushButton *> btns, QList<int> icons, const StyleColor &styleColor);
    void setStyle1(QWidget *widget, QList<QToolButton *> btns, QList<int> icons, const StyleColor &styleColor);
    void setStyle1(QWidget *widget, QList<QAbstractButton *> btns, QList<int> icons, const StyleColor &styleColor);
};

#endif // ICONHELPER_H
