#include "titemmodel.h"

TItemModel::TItemModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    qr = 0;
    row=column=0;
}

bool TItemModel::updateModel(const QSqlDatabase &db)
{
    row=column=0;
    if(qr)delete(qr);
    qr = 0;

    qr = new QSqlQuery("SELECT * FROM tasks;",db);
    if(!qr->exec())return false;

    if(!qr->isSelect() || qr->size() < 0)
        while(qr->next())++row;
    else row = qr->size();

    column = qr->record().count();
    qDebug()<<row<<column;
    return true;
}

int TItemModel::rowCount(const QModelIndex &parent) const
{
    if(!qr)return 0;
    return row;
}

int TItemModel::columnCount(const QModelIndex &parent) const
{
    if(!qr)return 0;
    return column;
}

QVariant TItemModel::data(const QModelIndex &index, int role) const
{
    qDebug()<<"1111"<<index;
    if(index.row() > row || index.column() > column)return QVariant();
    qr->seek(index.row());
    if(role == Qt::DisplayRole)
        return qr->value(index.column()).toString();

    return qr->value(index.column());
}

QVariant TItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)return QVariant();

    if(!qr || column == 0)return QVariant();

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
    return QModelIndex();
}

TItemModel::~TItemModel()
{
    if(qr)delete(qr);
}
