/*
Copyright (C) 2012-2013  Sarvaritdinov R.

This file is part of REXLoader.

REXLoader is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

REXLoader is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "popuplist.h"

PopupList::PopupList(QWidget *parent)
{
    prnt = parent;
    if(parent)
    {
        move(parent->pos().x(),parent->pos().y() + parent->height() + 5);
        if(parent->parent())
            setParent(qobject_cast<QWidget*>(parent->parent()));

        resize(parent->width(), parent->height() * 1);
    }
    setUniformItemSizes(true);
    connect(this,SIGNAL(clicked(QModelIndex)),this,SLOT(selected(QModelIndex)));
    hide();
}

PopupList::~PopupList()
{
    clearVariants();
}

void PopupList::addVariant(const QString &str)
{
    if(!str.isEmpty() && !filters.contains(str))
        filters.append(str);
}

void PopupList::setFilter(const QString &str)
{
    clear();

    QStringList flt = filters.filter(QRegExp(str,Qt::CaseInsensitive,QRegExp::Wildcard));
    if(flt.isEmpty())
    {
        hide();
        return;
    }

    foreach (QString cur, flt)
    {
        QListWidgetItem *itm = new QListWidgetItem(cur);
        addItem(itm);
    }
    show();
}

void PopupList::clearVariants()
{
    filters.clear();
    clear();
    if(isVisible())
        hide();
}

void PopupList::show()
{
    QStyleOptionViewItem opt;
    opt.initFrom(this);
    int hght = itemDelegate()->sizeHint(opt,currentIndex()).height();
    if(prnt)
    {
        resize(prnt->width(), hght * qMin(this->count(),10) + 8);
        move(prnt->pos().x(), prnt->pos().y() + prnt->height() + 3);
    }
    QListWidget::show();
}

QStringList PopupList::getFiltersList() const
{
    return filters;
}

void PopupList::selected(QModelIndex itm)
{
    QString out = itm.data(Qt::DisplayRole).toString();
    emit filterSelected(out);
    prnt->setFocus();
    hide();
}
