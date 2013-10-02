#ifndef SITEMANAGER_H
#define SITEMANAGER_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QtSql/QtSql>

#include <../systemiconswrapper/systemiconswrapper.h>

namespace Ui {
class SiteManager;
}

class SiteManager : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit SiteManager(QWidget *parent = 0);
    ~SiteManager();

public slots:
    void updateIcons();
    void show();

protected:
    void closeEvent(QCloseEvent *e);
    
private:
    Ui::SiteManager *ui;
    QPoint prePos;

    QSqlDatabase db;
};

#endif // SITEMANAGER_H
