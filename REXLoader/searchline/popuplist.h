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

#ifndef POPUPLIST_H
#define POPUPLIST_H

#include <QListWidget>

class PopupList : public QListWidget
{
    Q_OBJECT
public:
    explicit PopupList(QWidget *parent = 0);
    virtual ~PopupList();

public slots:
    void addVariant(const QString &str);
    void setFilter(const QString &str);
    void clearVariants();
    void show();
    QStringList getFiltersList() const;
    
signals:
    void filterSelected(const QString &str);

private slots:
    void selected(QModelIndex itm);

private:
    QStringList filters;
    QWidget *prnt;
};

#endif // POPUPLIST_H
