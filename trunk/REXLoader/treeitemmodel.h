/*
<one line to give the program's name and a brief idea of what it does.>
Copyright (C) <year>  <name of author>

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

#ifndef TREEITEMMODEL_H
#define TREEITEMMODEL_H

#include <QAbstractItemModel>
#include <QtSql/QtSql>

class TreeItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeItemModel(QObject *parent = 0);
    virtual ~TreeItemModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual bool hasChildren(const QModelIndex &parent) const;
    virtual bool hasIndex(int row, int column, const QModelIndex &parent) const;
    virtual Qt::ItemFlags flags(const QModelIndex & index) const;

signals:

public slots:
    bool updateModel(const QSqlDatabase &db = QSqlDatabase());

private:
    void addFiltersSubtree();

    QSqlQuery *qr;
    int gcol;

    QHash<QModelIndex,QVariant> nodes; //хэш всех узлов дерева
    QHash<QModelIndex,QModelIndex> link; //связи узлов между собой
};

#endif // TREEITEMMODEL_H
