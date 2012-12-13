/*
Project: REXLoader (Downloader), Source file: treeitemmodel.h
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

#ifndef TREEITEMMODEL_H
#define TREEITEMMODEL_H

#include <QAbstractItemModel>
#include <QtSql/QtSql>
#include "../plugins/LoaderInterface.h"

class TreeItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeItemModel(QObject *parent = 0);
    virtual ~TreeItemModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual bool hasChildren(const QModelIndex &parent) const;
    virtual bool hasIndex(int row, int column, const QModelIndex &parent) const;
    virtual Qt::ItemFlags flags(const QModelIndex & index) const;
    virtual bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::DropActions supportedDropActions() const;
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
    virtual QMimeData* mimeData(const QModelIndexList & indexes)const;
    QStringList mimeTypes()const;
    QFont getFont() const;
    void updateRow(int row, const QModelIndex &parent = QModelIndex());
    void updateRow(const QModelIndex &index);
    QList<QModelIndex> parentsInTree() const;
    virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());
    void setIgnoreFilters(bool ignore);
    QModelIndex indexById(int id) const;

public slots:
    bool updateModel(const QSqlDatabase &db = QSqlDatabase());
    void setFont(const QFont &fnt);
    bool silentUpdate(const QSqlDatabase &db = QSqlDatabase());

protected:
    int taskCount(const QModelIndex &index, bool incomplete = false) const;
    QVariant iconByIndex(const QModelIndex &idx) const;

private:
    void addFiltersSubtree();

    QSqlQuery *qr;
    int gcol;
    bool ignore_flag;

    QHash<QModelIndex,QVariant> nodes; //хэш всех узлов дерева
    QHash<QModelIndex,QModelIndex> link; //связи узлов между собой
    QFont font;
};

#endif // TREEITEMMODEL_H
