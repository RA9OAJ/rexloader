#ifndef TITEMMODEL_H
#define TITEMMODEL_H

#include <QAbstractItemModel>
#include <QtSql/QtSql>

class TItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TItemModel(QObject *parent = 0);
    virtual ~TItemModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    /*virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
    virtual QMap<int,QVariant> itemData(const QModelIndex &index) const;*/
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

signals:

public slots:
    bool updateModel(const QSqlDatabase &db = QSqlDatabase());

protected:
    QSqlQuery *qr;
    int grow,gcolumn;

};

#endif // TITEMMODEL_H
