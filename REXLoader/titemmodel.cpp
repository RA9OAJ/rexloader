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
    return gcolumn;
}

QVariant TItemModel::data(const QModelIndex &index, int role) const
{
    if(index.row() > grow || index.column() > gcolumn)return QVariant();
    qr->seek(index.row());
    if(role == Qt::DisplayRole)
        return qr->value(index.column());

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
        //case LInterface::ERROR_TASK: return QColor("#ff3030");
        //case -100: return QColor("#ff8787");
        case LInterface::ACCEPT_QUERY:
        case LInterface::SEND_QUERY:
        case LInterface::REDIRECT:
        case LInterface::STOPPING:
        case LInterface::ON_LOAD: return QIcon(":/appimages/start_24x24.png");
        case LInterface::FINISHED: return QIcon(":/appimages/finish_24x24.png");

        default: return QVariant();
        }
    }

    return QVariant();
}

QVariant TItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)return QVariant();

    if(!qr || gcolumn == 0)return QVariant();

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
    return createIndex(row,column,&qr->record().field(column));
}

TItemModel::~TItemModel()
{
    if(qr)delete(qr);
}
