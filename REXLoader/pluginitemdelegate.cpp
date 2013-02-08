/*
Project: REXLoader (Downloader), Source file: pluginitemdelegate.cpp
Copyright (C) 2012  Sarvaritdinov R.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pluginitemdelegate.h"

PluginItemDelegate::PluginItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    resetOptions();
}

PluginItemDelegate::~PluginItemDelegate()
{
}

void PluginItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt = QStyleOptionViewItemV4(option);
    initStyleOption(&opt,index);
    opt.text = "";
    painter->setBackground(QBrush(QColor(Qt::green)));
    opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    if(index.data(PluginListModel::PlugType).toString() == "Loader")
        paintBody(painter,opt,index);
    else
        paintBodyNotif(painter,opt,index);
    paintGrid(painter,opt);
}

QSize PluginItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    if(index.data(PluginListModel::PlugType).toString() == "File")
        return QSize(200,35);
    return QSize(200,60);
}

void PluginItemDelegate::paintBody(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->translate(option.rect.topLeft());
    QStringList data = index.data(Qt::DisplayRole).toStringList();

    //текст названия протокола
    QRectF staffTextBox(3,0,option.rect.width(),option.rect.height()/3.0f);
    painter->setFont(bigFont);
    painter->drawText(staffTextBox, Qt::AlignLeft | Qt::AlignBottom, protoText);

    staffTextBox.setLeft(staffTextBox.left() + dx1);
    painter->setFont(bigBoldFont);
    painter->drawText(staffTextBox,Qt::AlignLeft | Qt::AlignBottom, data.value(0));

    //текст названия плагина и версия
    staffTextBox.setRect(3,option.rect.height()/3.0f,option.rect.width(),option.rect.height()/3.0f);
    painter->setFont(normFont);
    painter->drawText(staffTextBox, Qt::AlignLeft | Qt::AlignBottom, pluginText);
    painter->setFont(boldFont);
    staffTextBox.setLeft(staffTextBox.left() + dx2);
    painter->drawText(staffTextBox, Qt::AlignLeft | Qt::AlignBottom, data.value(1));

    int dx = 0;
    if(!data.value(2).isEmpty())
    {
        QFontMetrics fm(boldFont);
        dx = fm.size(Qt::TextSingleLine,data.value(1)).width();
        staffTextBox.setLeft(staffTextBox.left() + dx);
        painter->setFont(normFont);
        painter->drawText(staffTextBox,Qt::AlignLeft | Qt::AlignBottom,tr(" (версия %1)").arg(data.value(2)));
    }

    //автор плагина, лицензия
    staffTextBox.setRect(3,option.rect.height()/3.0f*2,option.rect.width(),option.rect.height()/3.0f);
    painter->setFont(smallFont);
    painter->drawText(staffTextBox,Qt::AlignLeft | Qt::AlignTop, authorText);
    QFontMetrics fmsb(smallBoldFont);
    dx = fmsb.size(Qt::TextSingleLine,data.value(3)).width() + 3;
    staffTextBox.setLeft(staffTextBox.left() + dx3_1 + dx);
    painter->drawText(staffTextBox,Qt::AlignLeft | Qt::AlignTop, licText);
    painter->setFont(smallBoldFont);
    staffTextBox.setLeft(staffTextBox.left() + dx3_2);
    painter->drawText(staffTextBox,Qt::AlignLeft | Qt::AlignTop, data.value(4));
    staffTextBox.setLeft(staffTextBox.left() - dx3_2 - dx);
    painter->drawText(staffTextBox,Qt::AlignLeft | Qt::AlignTop, data.value(3));

    painter->restore();
}

void PluginItemDelegate::paintBodyNotif(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->translate(option.rect.topLeft());
    QStringList data = index.data(Qt::DisplayRole).toStringList();

    //текст названия плагина и версия
    QRectF staffTextBox(3,0,option.rect.width(),option.rect.height()/2.0f);
    painter->setFont(boldFont);
    staffTextBox.setLeft(staffTextBox.left());
    painter->drawText(staffTextBox, Qt::AlignLeft | Qt::AlignBottom, data.value(0));

    int dx = 0;
    QFontMetrics fm(boldFont);
    dx = fm.size(Qt::TextSingleLine,data.value(0)).width();
    staffTextBox.setLeft(staffTextBox.left() + dx);
    painter->setFont(normFont);
    painter->drawText(staffTextBox,Qt::AlignLeft | Qt::AlignBottom,tr(" (версия %1)").arg(data.value(1)));
    QFontMetrics fmn(normFont);
    dx = fmn.size(Qt::TextSingleLine, tr(" (версия %1)").arg(data.value(1))).width() + 3;
    staffTextBox.setLeft(staffTextBox.left() + dx);

    if(index.data(PluginListModel::PlugState).toBool())
        painter->drawImage(staffTextBox.x(),staffTextBox.y() + 1,QImage(":/appimages/yes_16x16.png"));
    else
        painter->drawImage(staffTextBox.x(),staffTextBox.y() + 1,QImage(":/appimages/no_16x16.png"));

    //автор плагина, лицензия
    staffTextBox.setRect(3,option.rect.height()/2.0f,option.rect.width(),option.rect.height()/2.0f);
    painter->setFont(smallFont);
    painter->drawText(staffTextBox,Qt::AlignLeft | Qt::AlignTop, authorText);
    QFontMetrics fmsb(smallBoldFont);
    dx = fmsb.size(Qt::TextSingleLine,data.value(2)).width() + 3;
    staffTextBox.setLeft(staffTextBox.left() + dx3_1 + dx);
    painter->drawText(staffTextBox,Qt::AlignLeft | Qt::AlignTop, licText);
    painter->setFont(smallBoldFont);
    staffTextBox.setLeft(staffTextBox.left() + dx3_2);
    painter->drawText(staffTextBox,Qt::AlignLeft | Qt::AlignTop, data.value(3));
    staffTextBox.setLeft(staffTextBox.left() - dx3_2 - dx);
    painter->drawText(staffTextBox,Qt::AlignLeft | Qt::AlignTop, data.value(2));

    painter->restore();
}

void PluginItemDelegate::paintOther(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
}

void PluginItemDelegate::paintGrid(QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->save();

    QPen gridPen(QApplication::palette().color(QPalette::WindowText));
    gridPen.setStyle(Qt::DashLine);
    painter->setPen(gridPen);
    painter->drawLine(option.rect.bottomLeft(),option.rect.bottomRight());

    painter->restore();
}

void PluginItemDelegate::resetOptions()
{
    //настройка шрифтов
    bigFont.setPixelSize(14);
    bigBoldFont.setPixelSize(14);
    bigBoldFont.setBold(true);
    normFont.setPixelSize(12);
    boldFont.setPixelSize(12);
    boldFont.setBold(true);
    smallFont.setPixelSize(10);
    smallFont.setItalic(true);
    smallBoldFont.setPixelSize(10);
    smallBoldFont.setBold(true);
    smallBoldFont.setItalic(true);

    //настройка служебных строк
    protoText = tr("Протокол: ");
    pluginText = tr("Плагин: ");
    authorText = tr("Автор: ");
    licText = tr("Лицензия: ");

    //расчет отклонений dx для строк
    QFontMetrics bigFontMetrics(bigFont), normFontMetrics(normFont),smallFontMetrics(smallFont);
    dx1 = bigFontMetrics.size(Qt::TextSingleLine,protoText).width();
    dx2 = normFontMetrics.size(Qt::TextSingleLine,pluginText).width();
    dx3_1 = smallFontMetrics.size(Qt::TextSingleLine,authorText).width();
    dx3_2 = smallFontMetrics.size(Qt::TextSingleLine,licText).width();
}
