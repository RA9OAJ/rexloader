#ifndef EFILTERPROXYMODEL_H
#define EFILTERPROXYMODEL_H

#include <QAbstractProxyModel>
#include <QItemSelection>
#include <QStringList>
#include <QDateTime>
#include <QSize>
#include <QDebug>


//typedef QHash<int, QModelIndex> InternalIndex;
typedef QList<int> InternalRow;

bool operator >(const QVariant &val1, const QVariant &val2);
bool operator <(const QVariant &val1, const QVariant &val2);
bool operator <=(const QVariant &val1, const QVariant &val2);
bool operator >=(const QVariant &val1, const QVariant &val2);

struct EFFilter { //структура хранения данных фильтра
    int data_role;
    int filter_operator;
    QVariant filter_value;

    EFFilter& operator=(EFFilter eff)
    {
        data_role = eff.data_role;
        filter_operator = eff.filter_operator;
        filter_value = eff.filter_value;
        return *this;
    }

    bool operator==(EFFilter eff) const
    {
        if(eff.data_role == data_role
                && eff.filter_operator == filter_operator
                && eff.filter_value == filter_value)
            return true;
        return false;
    }
};

class EFilterProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    enum EFOperator{
        Equal = 0x1, // ==
        Not = 0x2, // !=
        Like = 0x4, // содержит подстроку
        Larger = 0x8, //больше
        Lesser = 0x10, //меньше
        In = 0x20, //в списке
        Between = 0x40 //в промежутке
    };

    explicit EFilterProxyModel(QObject *parent = 0);


    virtual QModelIndex buddy(const QModelIndex &index)const;
    virtual bool canFetchMore(const QModelIndex &parent) const;
    virtual int columnCount (const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual void fetchMore(const QModelIndex &parent);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual QModelIndex mapFromSource (const QModelIndex & sourceIndex) const;
    virtual QItemSelection mapSelectionFromSource(const QItemSelection &sourceSelection) const;
    virtual QItemSelection mapSelectionToSource(const QItemSelection &proxySelection) const;
    virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchStartsWith | Qt::MatchWrap)) const;
    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
    virtual QStringList mimeTypes() const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    virtual int  rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
    virtual void setSourceModel(QAbstractItemModel *sourceModel);
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    virtual bool hasIndex(int row, int column, const QModelIndex &parent) const;
    virtual QSize span(const QModelIndex &index) const;
    virtual Qt::DropActions supportedDropActions() const;
    QAbstractItemModel *sourceModel() const;
    
signals:
    
public slots:
    void addFilter(int key_column, int filter_role, int _operator_, const QVariant &filter_val);
    void clearFilter(int key_column);
    void clearAllFilters();

    void proxyDataChanget(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void proxyHeaderDataChanget(Qt::Orientation orientation, int first, int last);
    void proxyModelReset();
    void proxyRowsInsrted(const QModelIndex & parent, int start, int end);
    void proxyRowsMoved(const QModelIndex & sourceParent, int sourceStart, int sourceEnd, const QModelIndex & destinationParent, int destinationRow);
    void proxyRowsRemoved(const QModelIndex & parent, int start, int end);

protected:
    virtual void runSorting(int column, Qt::SortOrder order = Qt::AscendingOrder);
    virtual bool runFiltering(int row = 0, const QModelIndex &parent = QModelIndex());
    void reset();

private:
    bool matchFilters(int row, const QModelIndex &parent) const;
    void addRow(int row, const QModelIndex &parent);
    void deleteRow(int row, const QModelIndex &parent);
    void removeSrcMap(const QModelIndex &key);
    void insertSrcMap(const QModelIndex &key, const QModelIndex &val);
    void clearSrcMap();

private:
    QAbstractItemModel *_src; //ссылка на модель-источник
    QMultiHash<int, EFFilter> _filters; //фильтры
    QHash<QModelIndex,QModelIndex*> _srcmap; //хэш соответствия строк из модели отфильтрованным строкам
    QPair<int,int> _sort_param; //параметры сортировки

    QHash<QModelIndex, InternalRow> indexes; //хэш дерева индексов со связями
    int row_diff;
};

#endif // EFILTERPROXYMODEL_H
