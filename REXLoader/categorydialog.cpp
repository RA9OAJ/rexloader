/*
Copyright (C) 2012-2013  Sarvaritdinov R.

This file is part of REXLoader.

REXLoader is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

REXLoader is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
    QSqlQuery qr(mydb);

    if(internal_id == -1)
    {
        int parent_id = 1;
        QModelIndex parent = model->index(0,0);
        QItemSelectionModel *selection = ui->treeView->selectionModel();

        parent = selection->selectedIndexes().value(0,parent);
        parent_id = model->data(model->index(parent.row(), 1, parent.parent()),100).toInt();

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

        myrow =  model->rowCount(parent);

        emit canUpdateModel(ui->cattitle->text(), myrow, parent_id, 0);

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
            myparent_id = parent_id;
        }
        return;
    }

    qr.prepare("UPDATE categories SET title=:title, dir=:dir, extlist=:ext WHERE id=:id");
    qr.bindValue("title", ui->cattitle->text());
    qr.bindValue("dir", ui->catpath->text());
    qr.bindValue("ext", ui->textEdit->document()->toPlainText());
    qr.bindValue("id", internal_id);

    if(!qr.exec())
    {
        //Запись в журнал ошибок
        qDebug()<<"void CategoryDialog::applyCategory(3): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
        accept();
        return;
    }

    emit canUpdateModel(QString(), myrow, myparent_id, internal_id);

    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if(btn == ui->buttonBox->button(QDialogButtonBox::Ok))accept();
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
    FileNameValidator *validator = new FileNameValidator(this);
    ui->cattitle->setValidator(validator);

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
    connect(ui->textEdit,SIGNAL(textChanged()),this,SLOT(formValidator()));

    connect(ui->toolButton,SIGNAL(released()),this,SLOT(showDirDialog()));
}

void CategoryDialog::setParentCategory(int parent)
{
    QItemSelectionModel *selection = ui->treeView->selectionModel();
    QModelIndex index = model->indexById(parent);
    selection->select(model->index(index.row(),0,index.parent()),QItemSelectionModel::Select);
    while(index != model->index(0,0) && index != QModelIndex())
    {
        ui->treeView->setExpanded(index,true);
        index = index.parent();
    }
}

void CategoryDialog::showDirDialog()
{
    QFileDialog *dlg = new QFileDialog(this);
    dlg->setDirectory(ui->catpath->text());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFileMode(QFileDialog::DirectoryOnly);
    dlg->setWindowTitle(tr("Выбор директории"));
    dlg->setOption(QFileDialog::DontUseNativeDialog);
    dlg->setModal(true);
    connect(dlg,SIGNAL(fileSelected(QString)),this,SLOT(setCategoryDir(QString)));
    dlg->show();
}

void CategoryDialog::setCategoryDir(const QString &dir)
{
    ui->catpath->setText(dir);
    formValidator();
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
            ui->error_label->setText(tr("Внутренняя ошибка SQL."));
            ui->error_label->setVisible(true);
            return;
        }

        if((qr.next() && qr.value(0).toInt() == 0) || internal_id != -1)
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
            ui->error_label->setText(tr("Категория с таким именем уже сущетвует."));
            ui->error_label->setVisible(true);
        }
    }

    QPushButton *btn = ui->buttonBox->button(QDialogButtonBox::Ok);
    btn->setEnabled(false);
    btn = ui->buttonBox->button(QDialogButtonBox::Apply);
    btn->setEnabled(false);
}

void CategoryDialog::setCategory(int id, int parent)
{
    ui->treeView->setEnabled(false);
    if(id > 0 && id < 7)ui->cattitle->setReadOnly(true);
    internal_id = id;
    myparent_id = parent;
    myrow = model->indexById(id).row();
}

void CategoryDialog::setCategoryExtList(const QString &extlist)
{
    ui->textEdit->document()->setPlainText(extlist);
}

void CategoryDialog::setCategoryTitle(const QString &title)
{
    ui->cattitle->setText(title);
}
