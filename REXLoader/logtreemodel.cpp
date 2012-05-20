/*
Project: REXLoader (Downloader), Source file: logtreemodel.cpp
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

#include "logtreemodel.h"

LogTreeModel::LogTreeModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    diff = 0;
    rows_cnt = 0;
    column_cnt = 4;
    max_rows = 1000;
    maxinternalid = -1;
}

LogTreeModel::~LogTreeModel()
{
}

QVariant LogTreeModel::data(const QModelIndex &index, int role) const
{
    if(index.parent() == QModelIndex())
    {

        if(role == Qt::DisplayRole)
        {
            int id = root_nodes.value(index,-1);
            if(id < 0) return QVariant();

            if(!index.column())
                return getTitle(index);

            return root_values.value(id);
        }

        if(role == Qt::DecorationRole)
        {
            int id = root_nodes.value(LogTreeModel::index(index.row(),2));
            int mtype = root_values.value(id).toInt();

            switch(mtype)
            {
            case LInterface::MT_IN: return QIcon(":/appimages/in_arrow.png");
            case LInterface::MT_OUT: return QIcon(":/appimages/out_arrow.png");
            default: return QVariant();
            }
        }

        if(role == 100)
        {
            int id = root_nodes.value(index);
            return root_values.value(id,QVariant());
        }

        return QVariant();

    }
    else
    {
        switch(role)
        {
        case Qt::DisplayRole: return sub_nodes.value(index);

        default: return QVariant();
        }
    }
}

int LogTreeModel::rowCount(const QModelIndex &parent) const
{
    if(parent == QModelIndex())
        return rows_cnt;
    int id = root_nodes.value(parent,-1) + diff*columnCount();
    return links.keys(id).size();
}

int LogTreeModel::columnCount(const QModelIndex &parent) const
{
    if(parent == QModelIndex())
        return column_cnt;

    return 1;
}

QModelIndex LogTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent == QModelIndex())
    {
        QModelIndex idx = root_nodes.key(column+row*columnCount(),QModelIndex());
        return idx;
    }

    int id = root_nodes.value(parent,-1);
    if(id > -1)
    {
        id += diff*columnCount();
        QModelIndexList idxs = links.keys(id);
        QModelIndex idx;
        foreach(idx,idxs)
            if(idx.row() == row && idx.column() == column)
                return idx;
    }
    return QModelIndex();
}

QModelIndex LogTreeModel::parent(const QModelIndex &child) const
{
    int prnt_id = links.value(child,-1);
    if(prnt_id == -1) return QModelIndex();

    prnt_id -= diff*columnCount();
    QModelIndex prnt = root_nodes.key(prnt_id, QModelIndex());
    return prnt;
}

bool LogTreeModel::hasChildren(const QModelIndex &parent) const
{
    if(parent == QModelIndex() && rowCount()) return true;
    int prnt_id = root_nodes.value(parent, -1);
    if(prnt_id < 0) return false;

    prnt_id +=diff*columnCount();
    QModelIndex idx = links.key(prnt_id,QModelIndex());
    if(idx != QModelIndex())
        return true;

    return false;
}

bool LogTreeModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
    if(LogTreeModel::index(row,column,parent) == QModelIndex())
        return false;

    return true;
}

Qt::ItemFlags LogTreeModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index);
}

bool LogTreeModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if(row == rowCount(parent))
    {
        beginInsertRows(parent,row,row+count);

        if(parent == QModelIndex())
        {
            for(int i = 0; i < count; ++i)
            {
                for(int y = 0; y < columnCount(parent); ++y)
                { 
                    QModelIndex newindex = LogTreeModel::createIndex(row+i,y, ++maxinternalid);
                    root_nodes.insert(newindex,row*columnCount(parent)+y);
                    root_values.append(QVariant());
                }
                ++rows_cnt;
            }
        }
        else
        {
            for(int i = 0; i < count; ++i)
            {
                for(int y = 0; y < columnCount(parent); ++y)
                {
                    int parent_id = root_nodes.value(parent) + diff*columnCount();
                    QModelIndex newindex = LogTreeModel::createIndex(row+i,y, ++maxinternalid);
                    sub_nodes.insert(newindex,QVariant());
                    links.insert(newindex, parent_id);
                }
            }
        }
        endInsertRows();
        return true;
    }
    return false;
}

bool LogTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!hasIndex(index.row(),index.column(),index.parent())) return false;

    if(index.parent() == QModelIndex())
    {
        int id = root_nodes.value(index);
        root_values[id] = value;
    }
    else sub_nodes.insert(index,value);
    emit dataChanged(index,index);
    return true;
}

Qt::DropActions LogTreeModel::supportedDropActions() const
{
    return QAbstractItemModel::supportedDropActions();
}

bool LogTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    return QAbstractItemModel::dropMimeData(data,action,row,column,parent);
}

QMimeData * LogTreeModel::mimeData(const QModelIndexList &indexes) const
{
    return QAbstractItemModel::mimeData(indexes);
}

QStringList LogTreeModel::mimeTypes() const
{
    return QAbstractItemModel::mimeTypes();
}

bool LogTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(parent != QModelIndex() || row) return false;
    if(rowCount() <= count) return false;
    if(rowCount() == count)
    {
        clearLog();
        return true;
    }

    beginRemoveRows(parent,0,count);
    root_values = root_values.mid(count*columnCount());

    for(int i = 0; i < count; ++i)
    {
        QModelIndex idx = LogTreeModel::index(row + i, 0);
        if(hasChildren(idx))
        {
            int id = root_nodes.value(idx) + diff*columnCount();
            QModelIndexList lst = links.keys(id);
            QModelIndex cur;
            foreach(cur,lst)
            {
                links.remove(cur);
                sub_nodes.remove(cur);
            }
        }

        for(int y = 0; y < columnCount(); ++y)
        {
            QModelIndex cur = LogTreeModel::index(rowCount() - i - 1, y);
            root_nodes.remove(cur);
        }
    }
    diff += count;
    rows_cnt -= count;
    endRemoveRows();

    return true;
}

void LogTreeModel::setMaxStringsCount(int max_cnt)
{
    max_rows = max_cnt;
}

void LogTreeModel::clearLog()
{
    beginRemoveRows(QModelIndex(),0,rowCount()-1);
    rows_cnt = 0;
    maxinternalid = 0;
    root_nodes.clear();
    root_values.clear();
    diff = 0;
    sub_nodes.clear();
    links.clear();
    endRemoveRows();
}

void LogTreeModel::appendLog(int ms_type, const QString &title, const QString &more)
{
    if(rowCount() == max_rows)
        removeRow(0);

    insertRow(rowCount());
    QModelIndex idx = LogTreeModel::index(rowCount()-1,0);
    setData(idx,title);
    idx = LogTreeModel::index(rowCount()-1,1);
    setData(idx,QDateTime::currentDateTime());
    idx = LogTreeModel::index(rowCount()-1,2);
    setData(idx,ms_type);
    idx = LogTreeModel::index(rowCount()-1,3);
    setData(idx,rowCount()-1);

    if(!more.isEmpty())
    {
        QModelIndex prnt_idx = LogTreeModel::index(rowCount()-1,0);
        insertRow(0,prnt_idx);
        idx = LogTreeModel::index(0,0,prnt_idx);
        setData(idx,more);
    }
}

void LogTreeModel::setLogColor(int m_type, const QColor &color)
{
    string_color.insert(m_type,color);
}

void LogTreeModel::setFont(int m_type, const QFont &font)
{
    fonts.insert(m_type,font);
}

void LogTreeModel::setFont(int m_type, const QColor &color)
{
    font_color.insert(m_type, color);
}

QString LogTreeModel::getTitle(const QModelIndex &index) const
{
    QModelIndex idx = LogTreeModel::index(index.row(), 0, index.parent());
    int id = root_nodes.value(idx);
    QString out = QString("[%1]: %2").arg(root_values.value(id+1).toDateTime().toString("dd.MM.yyyy hh:mm:ss"),root_values.value(id).toString());
    return out;
}

