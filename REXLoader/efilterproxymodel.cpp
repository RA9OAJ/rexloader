#include "efilterproxymodel.h"

EFilterProxyModel::EFilterProxyModel(QObject *parent) :
    QAbstractProxyModel(parent)
{
}

QModelIndex EFilterProxyModel::buddy(const QModelIndex &index) const
{
}

bool EFilterProxyModel::canFetchMore(const QModelIndex &parent) const
{
}

int EFilterProxyModel::columnCount(const QModelIndex &parent) const
{
}

QVariant EFilterProxyModel::data(const QModelIndex &index, int role) const
{
}

void EFilterProxyModel::fetchMore(const QModelIndex &parent)
{
}

Qt::ItemFlags EFilterProxyModel::flags(const QModelIndex &index) const
{
}

bool EFilterProxyModel::hasChildren(const QModelIndex &parent) const
{
}

QVariant EFilterProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
}

QModelIndex EFilterProxyModel::index(int row, int column, const QModelIndex &parent) const
{
}

bool EFilterProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
}

bool EFilterProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
}

QModelIndex EFilterProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
}

QItemSelection EFilterProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
}

QItemSelection EFilterProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
}

QModelIndex EFilterProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
}

QModelIndexList EFilterProxyModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
}

QMimeData *EFilterProxyModel::mimeData(const QModelIndexList &indexes) const
{
}

QStringList EFilterProxyModel::mimeTypes() const
{
}

QModelIndex EFilterProxyModel::parent(const QModelIndex &child) const
{
}

bool EFilterProxyModel::removeColumns(int column, int count, const QModelIndex &parent)
{
}

bool EFilterProxyModel::removeRows(int row, int count, const QModelIndex &parent)
{
}

int EFilterProxyModel::rowCount(const QModelIndex &parent) const
{
}

bool EFilterProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(sourceModel())
    {
        return sourceModel()->setData(mapToSource(index),value, role);
    }
    return false;
}

bool EFilterProxyModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
}

void EFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    _src = sourceModel;
}

void EFilterProxyModel::sort(int column, Qt::SortOrder order)
{
}

QSize EFilterProxyModel::span(const QModelIndex &index) const
{
}

Qt::DropActions EFilterProxyModel::supportedDropActions() const
{
}

QAbstractItemModel *EFilterProxyModel::sourceModel() const
{
    return _src;
}

void EFilterProxyModel::addFilter(int key_column, int filter_role, const QString &filter)
{
}

void EFilterProxyModel::clearFilter(int key_column)
{
}

void EFilterProxyModel::clearAllFilters()
{
}
