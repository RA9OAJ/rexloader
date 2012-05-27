/*
Project: REXLoader (Downloader), Source file: logtreemodel.h
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

#ifndef LOGTREEMODEL_H
#define LOGTREEMODEL_H

#include <QAbstractItemModel>
#include <QColor>
#include <QFont>
#include <QDebug>
#include <QIcon>
#include <QStringList>
#include <QDateTime>

#include "../Httploader/LoaderInterface.h"

class LogTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit LogTreeModel(QObject *parent = 0);
    virtual ~LogTreeModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
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
    virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

signals:

public slots:
    void setMaxStringsCount(int max_cnt);
    void clearLog();
    void appendLog(int ms_type, const QString &title, const QString &more);
    void setLogColor(int m_type, const QColor &color);
    void setLogColor(const QHash<int,QColor> &colors);
    void setFont(int m_type, const QFont &font);
    void setFont(const QHash<int, QFont> &_fonts);
    void setFontColor(int m_type, const QColor &color);
    void setFontColor(const QHash<int,QColor> &colors);

protected:
    QString getTitle(const QModelIndex &index) const;

private:
    int max_rows;
    int rows_cnt;
    int column_cnt;
    int diff;
    int maxinternalid;

    QHash<QModelIndex, int> root_nodes; //основные узлы дерева
    QList<QVariant> root_values; //значения узлов
    QHash<QModelIndex, QVariant> sub_nodes; //дочерние узлы ветви дерева
    QHash<QModelIndex, int> links; //связи между дочерними и основными узлами (int - номер родительской строки)

    QHash<int, QColor> row_color;
    QHash<int, QFont> fonts;
    QHash<int, QColor> font_color;
};

#endif // LOGTREEMODEL_H
