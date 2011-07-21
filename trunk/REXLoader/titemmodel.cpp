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

QVariant TItemModel::data(const QModelIndex &index, int role) const
{
    if(index.row() > grow || index.column() > gcolumn)return QVariant();
    qr->seek(index.row());

    if(role == Qt::DisplayRole)
    {
        if(index.column() == gcolumn) //добавление виртуальной колонки скорости скачивания
        {
            switch(qr->value(9).toInt())
            {
            case LInterface::ON_PAUSE:
            case LInterface::ERROR_TASK:
            case -100:
            case LInterface::FINISHED: return QVariant();

            default:
                QStringList _tmp = speedForHumans(curspeed.value(qr->value(0).toInt()));
                QString curspd_ = _tmp.value(0)+_tmp.value(1);
                return curspd_;
            }
        }

        if(index.column() == 9)
        {
            switch(qr->value(9).toInt())
            {
            case LInterface::ON_PAUSE: return QString(tr("Suspended"));
            case LInterface::ERROR_TASK: return QString(tr("Error"));
            case -100: return QString(tr("Waiting"));
            case LInterface::ACCEPT_QUERY:
            case LInterface::SEND_QUERY:
            case LInterface::REDIRECT:
            case LInterface::STOPPING:
            case LInterface::ON_LOAD:
                if(qr->value(5).toInt()) return QString::number(qr->value(4).toInt()*100/qr->value(5).toInt())+QString("%");
                else return QString("0%");
            case LInterface::FINISHED: return QString(tr("Completed"));

            default: return QVariant();
            }
        }

        return qr->value(index.column());
    }

    if(role == Qt::BackgroundColorRole)
    {
        switch(qr->value(9).toInt())
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
        switch(qr->value(9).toInt())
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
        qint64 totalsz = qr->value(5).toInt();

        if(!qr->value(4).toInt() || !qr->value(5).toInt())
            percent = "0";
        else percent = QString::number(qr->value(4).toInt()*100/qr->value(5).toInt());

        QStringList _tmp = sizeForHumans(totalsz);
        totalszStr = _tmp.value(0)+_tmp.value(1);
        _tmp.clear();
        _tmp = sizeForHumans(qr->value(4).toInt());
        cursz = _tmp.value(0)+_tmp.value(1);
        _tmp.clear();
        if(qr->value(9).toInt() == LInterface::ON_LOAD)
        {
            _tmp = speedForHumans(curspeed.value(qr->value(0).toInt()));
            spd = _tmp.value(0)+_tmp.value(1);
        }
        else spd = "---";

        tooltip = QString(tr("URL: %1\r\nFilename: %2\r\nTotal size: %3\r\nLeft: %4 (%5%)\r\nDown. speed: %6")).arg(qr->value(1).toString(),qr->value(3).toString(),totalszStr,cursz,percent,spd);
        return tooltip;
    }

    if(role == 100) //для сортировки
        return qr->value(index.column());

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
    if(sz >= 1073741824)outstrings << QString::number(sz/1073741824) << tr(" GB");
    else if(sz >= 1048576)outstrings << QString::number(sz/1048576) << tr(" MB");
    else if(sz >= 1024)outstrings << QString::number(sz/1024) << tr(" kB");
    else outstrings << QString::number(sz) << tr(" Bytes");

    return outstrings;
}

QStringList TItemModel::speedForHumans(qint64 sp, bool in_bytes)
{
    QStringList outstrings;
    if(in_bytes)sp = sp/8; // в случае, если скорость в байт/с
    if(sp >= 1073741824)outstrings << QString::number(sp/1073741824) << (in_bytes ? tr(" GB/s"):tr(" Gbps"));
    else if(sp >= 1048576)outstrings << QString::number(sp/1048576) << (in_bytes ? tr(" MB/s"):tr(" Mbps"));
    else if(sp >= 1024)outstrings << QString::number(sp/1024) << (in_bytes ? tr(" kB/s"):tr(" Kbps"));
    else outstrings << QString::number(sp) << (in_bytes ? tr(" B/s"):tr(" bps"));;

    return outstrings;
}

TItemModel::~TItemModel()
{
    if(qr)delete(qr);
}
