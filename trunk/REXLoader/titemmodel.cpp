/*
Project: REXLoader (Downloader), Source file: TItemModel.cpp
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

#include "titemmodel.h"

TItemModel::TItemModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    qr = 0;
    grow=gcolumn=0;
}

bool TItemModel::updateModel(const QSqlDatabase &db)
{
    grow=gcolumn=0;
    if(qr)delete(qr);
    qr = 0;

    qr = new QSqlQuery("SELECT * FROM tasks;",db);
    if(!qr->exec())
    {
        reset();
        return false;
    }

    if(!qr->isSelect() || qr->size() < 0)
        while(qr->next())++grow;
    else grow = qr->size();

    gcolumn = qr->record().count();
    reset();
    return true;
}

int TItemModel::rowCount(const QModelIndex &parent) const
{
    if(!qr)return 0;
    return grow;
}

int TItemModel::columnCount(const QModelIndex &parent) const
{
    if(!qr)return 0;
    return gcolumn+1;
}

void TItemModel::updateRow(int row)
{
    for(int i=0; i< columnCount(QModelIndex())-1; i++)
    {
        QModelIndex ModelIndex = index(row, i, QModelIndex());
        emit dataChanged(ModelIndex, ModelIndex);
    }
}

void TItemModel::addToCache(int row, int col, const QVariant &value)
{
    if(!cache.contains(row))
        cache.insert(row,new QHash<int,QVariant>);

    cache.value(row)->insert(col,value);
}

void TItemModel::clearCache(int row)
{
    if(row < 0)
    {
        QList<int> keys = cache.keys();
        for(int i = 0; i < keys.size(); i++)
        {
            cache.value(keys.value(i))->clear();
            delete(cache.value(keys.value(i)));
            cache.remove(keys.value(i));
        }
        return;
    }
    cache.value(row)->clear();
    delete(cache.value(row));
    cache.remove(row);
}

QVariant TItemModel::myData(int row, int col) const
{
    qr->seek(row);

    if(cache.contains(row))
    {
        if(cache.value(row)->contains(col))
            return cache.value(row)->value(col);
    }

    return qr->value(col);
}

QVariant TItemModel::data(const QModelIndex &index, int role) const
{

    if(index.row() > grow || index.column() > gcolumn)return QVariant();
    qr->seek(index.row());
    int row = index.row();
    int col = index.column();

//##############################################################################################################
    if(role == Qt::DisplayRole)
    {
        if(index.column() == gcolumn) //добавление виртуальной колонки скорости скачивания
        {
            switch(myData(row,9).toInt())
            {
            case LInterface::ON_PAUSE:
            case LInterface::ERROR_TASK:
            case -100:
            case LInterface::FINISHED: return QVariant();

            default:
                QStringList _tmp = speedForHumans(curspeed.value(myData(row,0).toInt()));
                QString curspd_ = _tmp.value(0)+_tmp.value(1);
                return curspd_;
            }
        }

        if(index.column() == 2)
            return QDateTime::fromString(myData(row,2).toString(),"yyyy-MM-ddThh:mm:ss");

        if(index.column() == 3)
        {
            QString filename = QFileInfo(myData(row,3).toString()).fileName();
            if(filename.right(5) == ".rldr")filename = filename.left(filename.size()-20);
            return filename;
        }

        if(index.column() == 5)
        {
            QStringList sz = sizeForHumans(myData(row,5).toLongLong());
            return sz.value(0)+sz.value(1);
        }

        if(index.column() == 6)
        {
            switch(myData(row,9).toInt())
            {
            case LInterface::ACCEPT_QUERY:
            case LInterface::SEND_QUERY:
            case LInterface::REDIRECT:
            case LInterface::STOPPING:
            case LInterface::ON_LOAD: return secForHumans(myData(row,col).toInt());

            default: return QVariant();
            }
        }

        if(index.column() == 9)
        {
            switch(myData(row,9).toInt())
            {
            case LInterface::ON_PAUSE: return QString(tr("Suspended"));
            case LInterface::ERROR_TASK: return QString(tr("Error"));
            case -100: return QString(tr("Waiting"));
            case LInterface::ACCEPT_QUERY:
            case LInterface::SEND_QUERY:
            case LInterface::REDIRECT:
            case LInterface::STOPPING:
            case LInterface::ON_LOAD:
                if(myData(row,5).toLongLong()) return QString::number(myData(row,4).toLongLong()*100/myData(row,5).toLongLong())+QString("%");
                else return QString("0%");
            case LInterface::FINISHED: return QString(tr("Completed"));

            default: return QVariant();
            }
        }

        return myData(row,col);
    }

    if(role == Qt::BackgroundColorRole)
    {
        switch(myData(row,9).toInt())
        {
        case LInterface::ON_PAUSE: return QColor("#fff3a4");
        case LInterface::ERROR_TASK: return QColor("#ff5757");
        case -100: return QColor("#ffbdbd");
        case LInterface::ACCEPT_QUERY:
        case LInterface::SEND_QUERY:
        case LInterface::REDIRECT:
        case LInterface::STOPPING:
        case LInterface::ON_LOAD: return QColor("#b4e1b4");
        case LInterface::FINISHED: return QColor("#abc2c8");

        default: return QVariant();
        }
    }

    if(role == Qt::DecorationRole && index.column() == 9)
    {
        switch(myData(row,9).toInt())
        {
        case LInterface::ON_PAUSE: return QIcon(":/appimages/pause_24x24.png");
        case LInterface::ERROR_TASK: return QIcon(":/appimages/error_24x24.png");
        case -100: return QIcon(":/appimages/queue_24x24.png");
        case LInterface::ACCEPT_QUERY:
        case LInterface::SEND_QUERY:
        case LInterface::REDIRECT:
        case LInterface::STOPPING:
        case LInterface::ON_LOAD: return QIcon(":/appimages/start_24x24.png");
        case LInterface::FINISHED: return QIcon(":/appimages/finish_24x24.png");

        default: return QVariant();
        }
    }


    if(role == Qt::ToolTipRole)
    {
        QString tooltip;
        QString totalszStr;
        QString cursz;
        QString spd;
        QString percent;
        qint64 totalsz = myData(row,5).toLongLong();

        if(!myData(row,4).toLongLong() || !myData(row,5).toLongLong())
            percent = "0";
        else percent = QString::number(myData(row,4).toLongLong()*100/myData(row,5).toLongLong());

        QString filename = QFileInfo(myData(row,3).toString()).fileName();
        if(filename.right(5) == ".rldr")filename = filename.left(filename.size()-20);

        QStringList _tmp = sizeForHumans(totalsz);
        totalszStr = _tmp.value(0)+_tmp.value(1);
        _tmp.clear();
        _tmp = sizeForHumans(myData(row,4).toLongLong());
        cursz = _tmp.value(0)+_tmp.value(1);
        _tmp.clear();
        if(myData(row,9).toInt() == LInterface::ON_LOAD)
        {
            _tmp = speedForHumans(curspeed.value(myData(row,0).toLongLong()));
            spd = _tmp.value(0)+_tmp.value(1);
        }
        else spd = "---";

        tooltip = QString(tr("URL: %1\r\nFilename: %2\r\nTotal size: %3\r\nLeft: %4 (%5%)\r\nDown. speed: %6")).arg(myData(row,1).toString(),filename,totalszStr,cursz,percent,spd);
        return tooltip;
    }

    if(role == 100) //для сортировки
    {
        switch(index.column())
        {
        case 2: return QDateTime::fromString(myData(row,2).toString(),"yyyy-MM-ddThh:mm:ss");
        default: return myData(row,col);
        }
    }
    return QVariant();
}

QVariant TItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)return QVariant();

    if(!qr || gcolumn == 0)return QVariant();

    if(section == gcolumn && orientation == Qt::Horizontal) //добавление виртуальной колонки скорости скачивания
        return QString("Down. speed");

    if(orientation == Qt::Horizontal)
        return qr->record().field(section).name();
    else return QVariant();
}

QModelIndex TItemModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

QModelIndex TItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if(row > grow || column > gcolumn || !qr) return QModelIndex();
    qr->seek(row);

    if(column == gcolumn)
    {
        qint64 _spd = curspeed.value(qr->value(0).toInt());
        return createIndex(row,column,&_spd); //добавление виртуальной колонки скорости скачивания
    }

    return createIndex(row,column,&qr->record().field(column));
}

QStringList TItemModel::sizeForHumans(qint64 sz)
{
    QStringList outstrings;
    if(sz >= 1073741824)outstrings << QString::number((qint64)sz/1073741824.0,'f',2) << tr(" GB");
    else if(sz >= 1048576)outstrings << QString::number((qint64)sz/1048576.0,'f',1) << tr(" MB");
    else if(sz >= 1024)outstrings << QString::number((qint64)sz/1024.0,'f',1) << tr(" kB");
    else outstrings << QString::number(sz) << tr(" Bytes");

    return outstrings;
}

QStringList TItemModel::speedForHumans(qint64 sp, bool in_bytes, bool out_bytes)
{
    QStringList outstrings;
    if(!in_bytes)sp = sp/8; // в случае, если скорость в бит/с
    if(sp >= 1073741824)outstrings << (out_bytes ? QString::number((qint64)sp/1073741824.0,'f',1):QString::number((qint64)sp*8.0/1073741824.0,'f',1)) << (out_bytes ? tr(" GB/s"):tr(" Gbps"));
    else if(sp >= 1048576)outstrings << (out_bytes ? QString::number((qint64)sp/1048576.0,'f',1):QString::number((qint64)sp*8.0/1048576.0,'f',1)) << (out_bytes ? tr(" MB/s"):tr(" Mbps"));
    else if(sp >= 1024)outstrings << (out_bytes ? QString::number((qint64)sp/1024.0,'f',1):QString::number((qint64)sp*8.0/1024.0,'f',1)) << (out_bytes ? tr(" kB/s"):tr(" Kbps"));
    else outstrings << (out_bytes ? QString::number(sp):QString::number(sp*8)) << (out_bytes ? tr(" B/s"):tr(" bps"));;

    return outstrings;
}

QString TItemModel::secForHumans(int sec)
{
    int days,hours,minuts;
    days = hours = minuts = 0;
    QString out;

    days = sec / 86400;
    sec %= 86400;
    hours = sec/3600;
    sec %= 3600;
    minuts = sec/60;
    sec %= 60;

    if(days)
        out = tr("%1d %2h:%3m:%4s").arg(QString::number(days), QString::number(hours), QString::number(minuts,'g',2), QString::number(sec,'g',2));
    else if(hours)
        out = tr("%1h:%2m:%3s").arg(QString::number(hours), QString::number(minuts,'g',2), QString::number(sec,'g',2));
    else if(minuts)
        out = tr("%1:%2").arg(QString::number(minuts), QString::number(sec,'g',2));
    else
        out = tr("%1s").arg(QString::number(sec));

    return out;
}

bool TItemModel::setMetaData(int key, const QString &name,const QVariant &value)
{
    if(name == "speed")
    {
        if(value.toLongLong() != 0)
            curspeed.insert(key, value.toLongLong());
        else
            curspeed.remove(key);

        return true;
    }

    return false;

}

TItemModel::~TItemModel()
{
    if(qr)delete(qr);
}
