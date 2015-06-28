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

#include "tableview.h"
#include <QDebug>

TableView::TableView(QWidget *parent) :
    QTableView(parent)
{
    connect(this,SIGNAL(clicked(QModelIndex)),SLOT(sendSelectSignal(QModelIndex)));
}

void TableView::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::KeypadModifier)
    {
        switch(event->key())
        {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit enterPressed();
            event->accept();
            return;

        default: break;
        }
    }

    if(event->modifiers() == Qt::CTRL || event->modifiers() == (Qt::CTRL | Qt::KeypadModifier))
    {
        switch(event->key())
        {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit showTaskProp();
            event->accept();
            return;

        default:break;
        }
    }

    QTableView::keyPressEvent(event);
}

void TableView::keyReleaseEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::KeypadModifier)
    {
        switch(event->key())
        {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit enterReleased();
            event->accept();
            return;

        default: break;
        }
    }

    QTableView::keyReleaseEvent(event);
}

void TableView::sendSelectSignal(const QModelIndex &idx)
{
    if(!model()) return;

    QModelIndex _idx = model()->index(idx.row(),0,idx.parent());
    int table_id = model()->data(_idx,100).toInt();
    emit clicked(table_id);
}

void TableView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTableView::selectionChanged(selected,deselected);

    if(!selected.count())
        emit clicked(0);
}
