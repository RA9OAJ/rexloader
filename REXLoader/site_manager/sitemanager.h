#ifndef SITEMANAGER_H
#define SITEMANAGER_H

#include <QMainWindow>

namespace Ui {
class SiteManager;
}

class SiteManager : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit SiteManager(QWidget *parent = 0);
    ~SiteManager();
    
private:
    Ui::SiteManager *ui;
};

#endif // SITEMANAGER_H
