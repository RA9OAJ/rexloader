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

#ifndef HASHCALCULATOR_H
#define HASHCALCULATOR_H

#include <QtCore>
#include <QDialog>
#include "../FileInterface.h"
#include "controldialog.h"
//#include "HashCalculatorThread.h"

class HashCalculator : public QObject,
                       public FileInterface
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "hashcalculator")
#endif
    Q_INTERFACES(FileInterface)

public:
    HashCalculator();
    virtual ~HashCalculator();
    //
    virtual void setFileName(const QStringList &file_name);
    virtual void release();
    virtual QString getLastError() const;
    virtual QStringList pluginInfo() const;
    virtual QList<DataAction> getActionList() const;
    virtual void runAction(int act_id);
    virtual QTranslator* getTranslator(const QLocale &locale);

private:
    QStringList m_file_name_list;
    ControlDialog *mp_control_dialog;
    QList<DataAction> m_action_list;
    QTranslator *translator;
    //HashCalculatorThread *mp_hash_calc;

private slots:
    //void run();
};

#endif // HASHCALCULATOR_H
