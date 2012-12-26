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
    row_colors.insert((int)LInterface::ON_PAUSE, QColor("#fff3a4"));
    row_colors.insert((int)LInterface::ERROR_TASK, QColor("#ff5757"));
    row_colors.insert(-100, QColor("#ffbdbd"));
    row_colors.insert((int)LInterface::ON_LOAD, QColor("#b4e1b4"));
    row_colors.insert((int)LInterface::FINISHED, QColor("#abc2c8"));
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

bool TItemModel::silentUpdateModel(const QSqlDatabase &db)
{
    int lastgrow =grow;
    grow=gcolumn=0;
    if(qr)delete(qr);
    qr = 0;

    qr = new QSqlQuery("SELECT * FROM tasks ORDER BY id ASC;",db);
    if(!qr->exec())
    {
        reset();
        return false;
    }

    if(!qr->isSelect() || qr->size() < 0)
        while(qr->next())++grow;
    else grow = qr->size();

    if(grow > lastgrow)
        beginInsertRows(QModelIndex(),lastgrow,grow-1);

    gcolumn = qr->record().count();

    if(grow > lastgrow)
        endInsertRows();
    return true;
}

int TItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if(!qr)return 0;
    return grow;
}

int TItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if(!qr)return 0;
    return gcolumn+2;
}

void TItemModel::updateRow(int row)
{
    for(int i=0; i <= columnCount(QModelIndex())+1; i++)
    {
        QModelIndex ModelIndex = index(row, i, QModelIndex());
        emit dataChanged(ModelIndex, ModelIndex);
        qApp->processEvents();
    }
}

void TItemModel::updateRow()
{
    int row = upd_queue.takeFirst();
    updateRow(row);
}

void TItemModel::addToCache(int row, int col, const QVariant &value)
{
    /*if(!cache.contains(row))
        cache.insert(row,new QHash<int,QVariant>);*/
    cache[row].insert(col,value);
}

void TItemModel::clearCache(int row)
{
    if(row < 0)
    {
        cache.clear();
        return;
    }
    cache[row].clear();
    cache.remove(row);
}

QVariant TItemModel::myData(int row, int col) const
{
    qr->seek(row);

    if(cache.contains(row))
        if(cache[row].contains(col))return cache[row].value(col);

    return qr->value(col);
}

QVariant TItemModel::data(const QModelIndex &index, int role) const
{

    if(index.row() > grow || index.column() > gcolumn+1)return QVariant();
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

        if(index.column() == gcolumn+1) //добавление виртуальной колонки оставшегося времени
        {
            switch(myData(row,9).toInt())
            {
            case LInterface::ON_PAUSE:
            case LInterface::ERROR_TASK:
            case -100:
            case LInterface::FINISHED: return QVariant();

            default:
                if(myData(row,11).toLongLong() == 0 || myData(row,5).toLongLong() == 0)return QVariant();
                int secToLeft = (myData(row,5).toLongLong()-myData(row,4).toLongLong())/1024/myData(row,11).toLongLong();
                return secForHumans(secToLeft);
            }
        }

        if(index.column() == 2)
        {
            QDateTime tm = QDateTime::fromString(myData(row,2).toString(),"yyyy-MM-ddThh:mm:ss");
            return tm.toString("dd.MM.yyyy hh:mm:ss");
        }

        if(index.column() == 3)
        {
            QString filename = QFileInfo(myData(row,3).toString()).fileName();
            if(filename.right(5) == ".rldr")filename = filename.left(filename.size()-20);
            return filename;
        }

        if(index.column() == 5)
        {
            if(!myData(row,5).toLongLong() && !myData(row,4).toLongLong()) return tr("Н/д");
            QStringList sz;
            if(!myData(row,5).toLongLong() && myData(row,4).toLongLong()) sz = sizeForHumans(myData(row,4).toLongLong());
            else sz = sizeForHumans(myData(row,5).toLongLong());
            return sz.value(0)+sz.value(1);
        }

        if(index.column() == 6)
        {
            if(myData(row,col).toInt())return secForHumans(myData(row,col).toInt());
            return QVariant();
        }

        if(index.column() == 9)
        {
            QString percent = tr("Н/д");
            QString altpersent;
            if(myData(row,5).toLongLong())
            {
                percent = QString::number(myData(row,4).toLongLong()*100/myData(row,5).toLongLong())+QString("%");
                altpersent = QString(" (%1)").arg(percent);
            }

            switch(myData(row,9).toInt())
            {
            case LInterface::ON_PAUSE: return tr("Остановлено") + altpersent;
            case LInterface::ERROR_TASK: return tr("Ошибка") + altpersent;
            case -100: return QString(tr("Ожидание")) + altpersent;
            case LInterface::ACCEPT_QUERY:
            case LInterface::SEND_QUERY:
            case LInterface::REDIRECT:
            case LInterface::STOPPING:
            case LInterface::ON_LOAD:
                return percent;
            case LInterface::FINISHED: return tr("Завершено");

            default: return QVariant();
            }
        }

        if(index.column() == 11) //добавление виртуальной колонки скорости скачивания
        {
            switch(myData(row,9).toInt())
            {
            case LInterface::ON_PAUSE:
            case LInterface::ERROR_TASK:
            case -100:
            case LInterface::FINISHED: return QVariant();

            default:
                QStringList _tmp = speedForHumans(myData(row,col).toLongLong());
                QString curspd_ = _tmp.value(0)+_tmp.value(1);
                return curspd_;
            }
        }

        return myData(row,col);
    }

    if(role == Qt::BackgroundColorRole)
    {
        switch(myData(row,9).toInt())
        {
        case LInterface::ON_PAUSE: return row_colors.value((int)LInterface::ON_PAUSE);//QColor("#fff3a4");
        case LInterface::ERROR_TASK: return row_colors.value((int)LInterface::ERROR_TASK);//QColor("#ff5757");
        case -100: return row_colors.value(-100);//QColor("#ffbdbd");
        case LInterface::ACCEPT_QUERY:
        case LInterface::SEND_QUERY:
        case LInterface::REDIRECT:
        case LInterface::STOPPING:
        case LInterface::ON_LOAD: return row_colors.value((int)LInterface::ON_LOAD);//QColor("#b4e1b4");
        case LInterface::FINISHED: return row_colors.value((int)LInterface::FINISHED);//QColor("#abc2c8");

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

    if(role == Qt::FontRole && row_fonts.contains(myData(row,9).toInt()))
        return row_fonts.value(myData(row,9).toInt());

    if(role == Qt::TextColorRole && row_font_colors.contains(myData(row,9).toInt()))
        return row_font_colors.value(myData(row,9).toInt());

    if(role == Qt::ToolTipRole)
    {
        QString tooltip;
        QString totalszStr;
        QString cursz;
        QString spd;
        QString percent;
        qint64 totalsz = myData(row,5).toLongLong();

        if(!myData(row,4).toLongLong() || !myData(row,5).toLongLong())
            percent = "";
        else percent = "(" + QString::number(myData(row,4).toLongLong()*100/myData(row,5).toLongLong()) + "%)";

        QString filename = QFileInfo(myData(row,3).toString()).fileName();
        if(filename.right(5) == ".rldr")filename = filename.left(filename.size()-20);

        QStringList _tmp = sizeForHumans(totalsz);
        if(!totalsz) totalszStr = tr("Н/д");
        else totalszStr = _tmp.value(0)+_tmp.value(1);
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

        tooltip = QString(tr("URL: %1<br>Имя файла: %2<br>Размер: %3<br>Завершено: %4 %5<br><font color='#00bb00'>Скорость: %6</font>")).arg(shortUrl(myData(row,1).toString()),shortUrl(filename),totalszStr,cursz,percent,spd);
        return tooltip;
    }

    if(role == Qt::TextColorRole)
    {
        return QColor("#111111");
    }

    if(role == Qt::TextAlignmentRole)
    {
        if(index.column() == gcolumn || index.column() == gcolumn + 1)
            return (QVariant)(Qt::AlignRight | Qt::AlignVCenter);

        switch(index.column())
        {
        case 2:
        case 4:
        case 5:
        case 6: return (QVariant)(Qt::AlignRight | Qt::AlignVCenter);

        default: return QVariant();
        }
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
        return QString(tr("Скорость"));

    if(section == gcolumn+1 && orientation == Qt::Horizontal) //добавление виртуальной колонки
        return QString(tr("До завершения"));

    if(orientation == Qt::Horizontal)
        switch(section)
        {
        case 2: return tr("Дата создания");
        case 3: return tr("Файл");
        case 5: return tr("Размер");
        case 6: return tr("Время скачивания");
        case 9: return tr("Статус");
        case 12: return tr("Примечание");

        default: return qr->record().field(section).name();
        }
    else return QVariant();
}

QModelIndex TItemModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

QModelIndex TItemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if(row > grow || column > gcolumn+1 || !qr) return QModelIndex();
    qr->seek(row);

    if(column == gcolumn)
    {
        qint64 _spd = curspeed.value(qr->value(0).toInt());
        return createIndex(row,column,&_spd); //добавление виртуальной колонки скорости скачивания
    }

    if(column == gcolumn + 1)
    {

        int sec = 0;
        if(myData(row,11).toLongLong() != 0)sec = (myData(row,5).toLongLong()-myData(row,4).toLongLong())/myData(row,11).toLongLong();;
        return createIndex(row,column,&sec); //добавление виртуальной колонки
    }
    QVariant val = qr->value(column);
    return createIndex(row,column,&val);
}

QStringList TItemModel::sizeForHumans(qint64 sz)
{
    QStringList outstrings;
    if(sz >= 1073741824)outstrings << QString::number((qint64)sz/1073741824.0,'f',2) << tr(" ГБ");
    else if(sz >= 1048576)outstrings << QString::number((qint64)sz/1048576.0,'f',1) << tr(" МБ");
    else if(sz >= 1024)outstrings << QString::number((qint64)sz/1024.0,'f',1) << tr(" кБ");
    else outstrings << QString::number(sz) << tr(" Байт");

    return outstrings;
}

QStringList TItemModel::speedForHumans(qint64 sp, bool in_bytes, bool out_bytes)
{
    QStringList outstrings;
    if(in_bytes && !out_bytes)sp = sp*8;
    if(!in_bytes && out_bytes)sp = sp/8;
    if(sp >= 1073741824)outstrings << QString::number((qint64)sp/1073741824.0,'f',1) << (out_bytes ? tr(" ГБ/с"):tr(" Gbps"));
    else if(sp >= 1048576)outstrings << QString::number((qint64)sp/1048576.0,'f',1) << (out_bytes ? tr(" МБ/с"):tr(" Mbps"));
    else if(sp >= 1024)outstrings << QString::number((qint64)sp/1024.0,'f',1) << (out_bytes ? tr(" кБ/с"):tr(" Kbps"));
    else outstrings << QString::number(sp) << (out_bytes ? tr(" Байт/с"):tr(" bps"));;

    return outstrings;
}

QString TItemModel::secForHumans(int sec)
{
    if(sec < 0) return tr("Н/д");
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
        out = tr("%1д %2ч %3м %4с").arg(QString::number(days), QString::number(hours), QString::number(minuts), QString::number(sec));
    else if(hours)
        out = tr("%1ч %2м %3с").arg(QString::number(hours), QString::number(minuts), QString::number(sec));
    else if(minuts)
        out = tr("%1м %2с").arg(QString::number(minuts), QString::number(sec));
    else
        out = tr("%1с").arg(QString::number(sec));

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

QString TItemModel::shortUrl(QString url, int max_len)
{
    if(url.size() <= max_len) return url;
    int pos = url.size()/2 - (url.size() + 3 - max_len)/2;
    url.replace(pos,(url.size() + 3 - max_len),"...");
    return url;
}

void TItemModel::addToUpdateRowQueue(int row)
{
    upd_queue.append(row);
}

void TItemModel::setRowColor(int status, const QColor &color)
{
    row_colors.insert(status, color);
}

void TItemModel::setRowFont(int status, const QFont &font)
{
    row_fonts.insert(status,font);
}

void TItemModel::setRowFontColor(int status, const QColor &font_color)
{
    row_font_colors.insert(status,font_color);
}
