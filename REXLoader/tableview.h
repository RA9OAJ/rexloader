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

#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>
#include <QKeyEvent>

class TableView : public QTableView
{
    Q_OBJECT
public:
    explicit TableView(QWidget *parent = 0);

signals:
    void enterPressed();
    void enterReleased();
    void showTaskProp();
    void clicked(int table_id);

protected slots:
    void sendSelectSignal(const QModelIndex &idx);
    virtual void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
};

#endif // TABLEVIEW_H
