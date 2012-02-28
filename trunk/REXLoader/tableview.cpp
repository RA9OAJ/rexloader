/*
Project: REXLoader (Downloader), Source file: tableview.cpp
Copyright (C) 2011  Sarvaritdinov R.

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

#include "tableview.h"

TableView::TableView(QWidget *parent) :
    QTableView(parent)
{
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
