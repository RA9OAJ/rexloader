/*
Project: REXLoader (Downloader), Source file: TItemModel.h
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

#ifndef TITEMMODEL_H
#define TITEMMODEL_H

#include <QAbstractItemModel>
#include <QtSql/QtSql>
#include "../plugins/LoaderInterface.h"

class TItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TItemModel(QObject *parent = 0);
    virtual ~TItemModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    bool setMetaData(int key, const QString &name, const QVariant &value);
    void addToCache(int row, int col, const QVariant &value);
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void setSpdFormat(bool out_bytes = false);
    static QStringList sizeForHumans(qint64 sz);
    static QStringList speedForHumans(qint64 sp, bool in_bytes = true, bool out_bytes = false);
    static QString secForHumans(int sec);
    static QString shortUrl(QString url, int max_len = 72);
    void setRowColor(int status, const QColor &color);
    void setRowFont(int status, const QFont &font);
    void setRowFontColor(int status, const QColor &font_color);

public slots:
    void updateRow(int row);
    void updateRow();
    void addToUpdateRowQueue(int row);
    bool updateModel(const QSqlDatabase &db = QSqlDatabase());
    bool silentUpdateModel(const QSqlDatabase &db = QSqlDatabase());
    void clearCache(int row = -1);

private:
    QVariant myData(int row, int col) const;

    QSqlQuery *qr;
    int grow,gcolumn;
    QHash<int,qint64>curspeed;
    QHash<int,QColor>row_colors;
    QHash<int,QFont>row_fonts;
    QHash<int,QColor>row_font_colors;
    QHash<int, QHash<int,QVariant> >cache;
    QList<int> upd_queue;
    bool out_spdf;
};

#endif // TITEMMODEL_H
