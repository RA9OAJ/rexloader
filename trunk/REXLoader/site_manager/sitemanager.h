#ifndef SITEMANAGER_H
#define SITEMANAGER_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QtSql/QtSql>

#include <../systemiconswrapper/systemiconswrapper.h>
#include "logindialog.h"

namespace Ui {
class SiteManager;
}

class SiteManager : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit SiteManager(QWidget *parent = 0);
    ~SiteManager();
    QString authData(const QUrl &url);

public slots:
    void authAction(int id_task, const QUrl &url);
    void updateIcons();
    void show();

protected slots:
    void authDataForUrl(const QUrl &url, const QString &auth);

protected:
    void closeEvent(QCloseEvent *e);
    
signals:
    void authEntered(int id_task, const QString &auth);

private:
    Ui::SiteManager *ui;
    QPoint prePos;
    QMap<QUrl,int> task_map;

    QSqlDatabase db;
};

#endif // SITEMANAGER_H
