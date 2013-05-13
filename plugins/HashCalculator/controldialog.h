/*
Copyright (C) 2012-2013  Alexey Schukin

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

#ifndef CONTROLDIALOG_H
#define CONTROLDIALOG_H

#include <QDialog>
#include "HashCalculatorThread.h"

namespace Ui {
class ControlDialog;
}

class ControlDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ControlDialog(QWidget *parent = 0);
    ~ControlDialog();
    void setFileNames(const QStringList &file_name);
public slots:
    void progress(QString file_name, int percent);
    void calcFinished(QString file_name, QString md5_result, QString sha1_result);
    int exec();
private:
    Ui::ControlDialog *ui;
    HashCalculatorThread *mp_hash_calc_thread;
    int row;

private slots:
    void slotClose();
};

#endif // CONTROLDIALOG_H
