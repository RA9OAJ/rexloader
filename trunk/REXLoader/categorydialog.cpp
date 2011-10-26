#include "categorydialog.h"
#include "ui_categorydialog.h"

CategoryDialog::CategoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CategoryDialog)
{
    ui->setupUi(this);

    mydb = QSqlDatabase::database();

    initialize();
}

CategoryDialog::CategoryDialog(QSqlDatabase &db_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CategoryDialog)
{
    ui->setupUi(this);

    mydb = db_;

    initialize();
}

CategoryDialog::~CategoryDialog()
{
    delete ui;
}

void CategoryDialog::applyCategory()
{
    int parent_id = 1;
    QModelIndex parent = model->index(0,0);
    QItemSelectionModel *selection = ui->treeView->selectionModel();
    qDebug()<<selection;
    if(selection->selectedRows(0).size() > 0)
    {
        parent = selection->selectedRows(0).value(0);
        parent_id = model->data(model->index(parent.row(), 1, parent.parent()),100).toInt();
        qDebug()<<parent_id;
    }

    QSqlQuery qr(mydb);
    qr.prepare("INSERT INTO categories(title, dir, extlist, parent_id) VALUES (:title, :dir, :extlist, :parent)");
    qr.bindValue("title", ui->cattitle->text());
    qr.bindValue("dir", ui->catpath->text());
    qr.bindValue("extlist", ui->textEdit->document()->toPlainText());
    qr.bindValue("parent", parent_id);

    if(!qr.exec())
    {
        //Запись в журнал ошибок
        qDebug()<<"void CategoryDialog::applyCategory(): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
        return;
    }

    emit canUpdateModel(ui->cattitle->text(), model->rowCount(parent), parent_id);

    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if(btn == ui->buttonBox->button(QDialogButtonBox::Ok))accept();
}

void CategoryDialog::initialize()
{
    setAttribute(Qt::WA_DeleteOnClose);
    model = new TreeItemModel(this);
    model->setIgnoreFilters(true);
    model->updateModel(mydb);
    ui->treeView->setModel(model);
    ui->treeView->setAnimated(true);
    ui->treeView->header()->hide();
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
    ui->treeView->hideColumn(4);
    ui->treeView->setExpanded(model->index(0,0),true);

    QPushButton *btn = ui->buttonBox->button(QDialogButtonBox::Ok);
    connect(btn,SIGNAL(released()),this,SLOT(applyCategory()));
    btn = ui->buttonBox->button(QDialogButtonBox::Cancel);
    connect(btn,SIGNAL(released()),this,SLOT(reject()));
    btn = ui->buttonBox->button(QDialogButtonBox::Apply);
    connect(btn,SIGNAL(released()),this,SLOT(applyCategory()));

    connect(ui->toolButton,SIGNAL(released()),this,SLOT(showDirDialog()));
}

void CategoryDialog::setParentCategory(int parent)
{
    QItemSelectionModel *selection = ui->treeView->selectionModel();
    QModelIndex index = model->indexById(parent);
    selection->select(model->index(index.row(),0,index.parent()),QItemSelectionModel::Select);
}

void CategoryDialog::showDirDialog()
{
    QFileDialog *dlg = new QFileDialog(this);
    dlg->setDirectory(ui->cattitle->text());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFileMode(QFileDialog::DirectoryOnly);
    dlg->setWindowTitle(tr("Select Directory"));
    dlg->setOption(QFileDialog::DontUseNativeDialog);
    dlg->setModal(true);
    connect(dlg,SIGNAL(fileSelected(QString)),this,SLOT(setCategoryDir(QString)));
    dlg->show();
}

bool CategoryDialog::isValid() const
{
    return true;
}

void CategoryDialog::setCategoryDir(const QString &dir)
{
    ui->catpath->setText(dir);
}
