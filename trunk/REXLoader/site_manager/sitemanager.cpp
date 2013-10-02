#include "sitemanager.h"
#include "ui_sitemanager.h"

SiteManager::SiteManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SiteManager)
{
    ui->setupUi(this);
}

SiteManager::~SiteManager()
{
    delete ui;

    setVisible(false);
    db = QSqlDatabase::database();
}

void SiteManager::updateIcons()
{
}

void SiteManager::show()
{
    if(isHidden() && !prePos.isNull())
        move(prePos);

    if(isVisible())
        activateWindow();
    else QMainWindow::show();
}

void SiteManager::closeEvent(QCloseEvent *e)
{
    if(isVisible())
    {
        prePos = pos();
        hide();
    }
    e->accept();
}
