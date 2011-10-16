/*
Project: REXLoader (Downloader), Source file: EMessageBox.cpp
Copyright (C) 2011  Sarvaritdinov R.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "emessagebox.h"

int EMessageBox::obj_cnt = 0;

EMessageBox::EMessageBox(QWidget *parent) :
    QMessageBox(parent)
{
    ++obj_cnt;
    defBtn = 0;
    timeout = 30; //30 секунд
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(tickTimer()));
    setWindowModality(Qt::WindowModal);
    timer->start(1000);
    setWindowTitle("REXLoader - "+tr("File already exists!"));
    wtitle = windowTitle();
    moveToCenter();
}

EMessageBox::~EMessageBox()
{
    --obj_cnt;
}

void EMessageBox::setDefaultTimeout(int sec)
{
    if(sec < 0)return;
    timeout = sec;
    timer->start(1000);

    if(defBtn) defBtn->setText(btntext + QString(" (%1)").arg(QString::number(timeout)));
}

void EMessageBox::setDefaultButton(QPushButton *button)
{
    defBtn = button;
    if(!defBtn)return;
    btntext = defBtn->text();
    defBtn->setText(btntext + QString(" (%1)").arg(QString::number(timeout)));
    QMessageBox::setWindowTitle(wtitle + QString(" (%1)").arg(QString::number(timeout)));

    QMessageBox::setDefaultButton(button);
}

void EMessageBox::setDefaultButton(StandardButton button)
{
    QMessageBox::setDefaultButton(button);

    defBtn = defaultButton();
    if(!defBtn)return;
    btntext = defBtn->text();
    defBtn->setText(btntext + QString(" (%1)").arg(QString::number(timeout)));
    QMessageBox::setWindowTitle(wtitle + QString(" (%1)").arg(QString::number(timeout)));
}

void EMessageBox::tickTimer()
{
    if(!defBtn)return;

    --timeout;

    if(defBtn->text().indexOf(btntext) != 0)
        btntext = defBtn->text();
    defBtn->setText(btntext + QString(" (%1)").arg(QString::number(timeout)));
    QMessageBox::setWindowTitle(wtitle + QString(" (%1)").arg(QString::number(timeout)));

    if(timeout > 0)return;

    connect(this,SIGNAL(btnSelect()),defBtn,SIGNAL(clicked()));
    emit btnSelect();
}

EMessageBox::ActionTypes EMessageBox::myTypes() const
{
    return mytype;
}

void EMessageBox::setActionType(EMessageBox::ActionTypes type)
{
    mytype = type;
}

QString EMessageBox::myParams() const
{
    return myparam;
}

void EMessageBox::setParams(const QString &par)
{
    myparam = par;
}

void EMessageBox::moveToCenter()
{
    QDesktopWidget ds;
    QRect desktop = ds.availableGeometry();
    QPoint top_left = QPoint((desktop.bottomRight().x()-size().width())/2+20*(obj_cnt-1),(desktop.bottomRight().y()-size().height())/2+20*(obj_cnt-1));
    move(top_left);
}

void EMessageBox::show()
{
    QMessageBox::show();
    moveToCenter();
}

void EMessageBox::closeEvent(QCloseEvent *event)
{
    emit buttonClicked(defaultButton());
    QMessageBox::closeEvent(event);
}

void EMessageBox::setWindowTitle(const QString &title)
{
    wtitle = "REXLoader - "+title;
    QMessageBox::setWindowTitle(title);
}
