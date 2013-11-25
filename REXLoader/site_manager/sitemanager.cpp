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

QString SiteManager::authData(const QUrl &url)
{
    return QString();
}

void SiteManager::authAction(int id_task, const QUrl &url)
{
    task_map.insert(url,id_task);
    LoginDialog *dlg = new LoginDialog(this);
    dlg->setDefaultButton(QDialogButtonBox::Cancel,60);
    connect(dlg,SIGNAL(authData(QUrl,QString)),this,SLOT(authDataForUrl(QUrl,QString)));
    dlg->getAuth(url);
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

void SiteManager::authDataForUrl(const QUrl &url, const QString &auth)
{
    int id_task = task_map.value(url,-1);
    if(id_task != -1)
    {
        emit authEntered(id_task,auth);
        task_map.remove(url);
    }
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
