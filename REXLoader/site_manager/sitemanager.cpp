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
}
