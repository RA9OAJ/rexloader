#ifndef EFILTERPROXYMODEL_H
#define EFILTERPROXYMODEL_H

#include <QAbstractProxyModel>
#include <QItemSelection>
#include <QStringList>
#include <QSize>

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
        Equal = 0x0, // ==
        Not = 0x1, // !=
        Like = 0x2, // содержит подстроку
        Larger = 0x4, //больше
        Lesser = 0x8, //меньше
        In = 0x10, //в списке
        Between = 0x20 //в промежутке
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
    virtual QSize span(const QModelIndex &index) const;
    virtual Qt::DropActions supportedDropActions() const;
    QAbstractItemModel *sourceModel() const;
    
signals:
    
public slots:
    void addFilter(int key_column, int filter_role, int _operator_, const QVariant &filter_val);
    void clearFilter(int key_column);
    void clearAllFilters();

private:
    QAbstractItemModel *_src; //ссылка на модель-источник
    QMultiHash<int, EFFilter> _filters; //фильтры
    QHash<int, int> _srcmap; //хэш соответствия строк из модели отфильтрованным строкам
};

#endif // EFILTERPROXYMODEL_H
