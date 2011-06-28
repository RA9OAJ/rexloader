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
    if(!qr->exec())return false;

    if(!qr->isSelect() || qr->size() < 0)
        while(qr->next())++grow;
    else grow = qr->size();

    gcolumn = qr->record().count();
    qDebug()<<grow<<gcolumn;
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
    qDebug()<<"1111"<<index;
    if(index.row() > grow || index.column() > gcolumn)return QVariant();
    qr->seek(index.row());
    if(role == Qt::DisplayRole)
        return qr->value(index.column());

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
