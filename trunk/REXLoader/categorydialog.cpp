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
    if(internal_id == -1)
    {
        int parent_id = 1;
        QModelIndex parent = model->index(0,0);
        QItemSelectionModel *selection = ui->treeView->selectionModel();

        parent = selection->selectedIndexes().value(0,parent);
        parent_id = model->data(model->index(parent.row(), 1, parent.parent()),100).toInt();


        QSqlQuery qr(mydb);
        qr.prepare("INSERT INTO categories(title, dir, extlist, parent_id) VALUES (:title, :dir, :extlist, :parent)");
        qr.bindValue("title", ui->cattitle->text());
        qr.bindValue("dir", ui->catpath->text());
        qr.bindValue("extlist", ui->textEdit->document()->toPlainText());
        qr.bindValue("parent", parent_id);

        if(!qr.exec())
        {
            //Запись в журнал ошибок
            qDebug()<<"void CategoryDialog::applyCategory(1): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
            return;
        }

        emit canUpdateModel(ui->cattitle->text(), model->rowCount(parent), parent_id);

        QPushButton *btn = qobject_cast<QPushButton*>(sender());
        if(btn == ui->buttonBox->button(QDialogButtonBox::Ok))accept();
        else
        {
            ui->treeView->setEnabled(false);
            qr.clear();
            qr.prepare("SELECT id FROM categories WHERE title=:title AND parent_id=:parent");
            qr.bindValue("title", ui->cattitle->text());
            qr.bindValue("parent",parent_id);

            if(!qr.exec())
            {
                //Запись в журнал ошибок
                qDebug()<<"void CategoryDialog::applyCategory(2): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
                accept();
                return;
            }

            qr.next();
            internal_id = qr.value(0).toInt();
        }
        return;
    }

}

void CategoryDialog::initialize()
{
    internal_id = -1;
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
    btn->setEnabled(false);
    connect(btn,SIGNAL(released()),this,SLOT(applyCategory()));
    btn = ui->buttonBox->button(QDialogButtonBox::Cancel);
    connect(btn,SIGNAL(released()),this,SLOT(reject()));
    btn = ui->buttonBox->button(QDialogButtonBox::Apply);
    btn->setEnabled(false);
    ui->error_label->setVisible(false);
    connect(btn,SIGNAL(released()),this,SLOT(applyCategory()));
    connect(ui->cattitle,SIGNAL(textEdited(QString)),this,SLOT(formValidator()));

    connect(ui->toolButton,SIGNAL(released()),this,SLOT(showDirDialog()));
}

void CategoryDialog::setParentCategory(int parent)
{
    QItemSelectionModel *selection = ui->treeView->selectionModel();
    QModelIndex index = model->indexById(parent);
    selection->select(model->index(index.row(),0,index.parent()),QItemSelectionModel::Select);
    while(index != model->index(0,0))
    {
        ui->treeView->setExpanded(index,true);
        index = index.parent();
    }
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

void CategoryDialog::setCategoryDir(const QString &dir)
{
    ui->catpath->setText(dir);
}

void CategoryDialog::formValidator()
{
    if(!ui->cattitle->text().isEmpty())
    {
        int parent_id = 1;
        QModelIndex parent = model->index(0,0);
        QItemSelectionModel *selection = ui->treeView->selectionModel();
        parent = selection->selectedIndexes().value(0,parent);
        parent_id = model->data(model->index(parent.row(), 1, parent.parent()),100).toInt();

        QSqlQuery qr(mydb);
        qr.prepare("SELECT COUNT(*) FROM categories WHERE title=:title AND parent_id=:parent");
        qr.bindValue("title",ui->cattitle->text());
        qr.bindValue("parent",parent_id);

        if(!qr.exec())
        {
            //Запись в журнал ошибок
            qDebug()<<"void CategoryDialog::formValidator(): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
            ui->error_label->setText(tr("Internal error in SQL."));
            ui->error_label->setVisible(true);
            return;
        }

        if(qr.next() && qr.value(0).toInt() == 0)
        {
            QPushButton *btn = ui->buttonBox->button(QDialogButtonBox::Ok);
            btn->setEnabled(true);
            btn = ui->buttonBox->button(QDialogButtonBox::Apply);
            btn->setEnabled(true);
            ui->error_label->setVisible(false);
            return;
        }
        else
        {
            ui->error_label->setText(tr("Category with that name already exists."));
            ui->error_label->setVisible(true);
        }
    }

    QPushButton *btn = ui->buttonBox->button(QDialogButtonBox::Ok);
    btn->setEnabled(false);
    btn = ui->buttonBox->button(QDialogButtonBox::Apply);
    btn->setEnabled(false);
}
