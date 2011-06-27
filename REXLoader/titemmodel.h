#ifndef TITEMMODEL_H
#define TITEMMODEL_H

#include <QAbstractItemModel>

class TItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TItemModel(QObject *parent = 0);
    virtual ~TItemModel();

    virtual QVriant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual void setData(const QModelIndex &index, const QVariant &value, int role);
    virtual QMap<int,QVariant> itemData(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const = 0;

signals:

public slots:

};

#endif // TITEMMODEL_H
