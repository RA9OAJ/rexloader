/*
Copyright (C) 2012-2013  Alexey Schukin
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

#include <QtCore>
#include "HashCalculator.h"


HashCalculator::HashCalculator():
    mp_control_dialog(0)
{
    //mp_hash_calc = new HashCalculatorThread(this);

    DataAction act;
    act.act_id = 1;
    act.act_title = tr("Подсчёт md5, sha1");
    m_action_list.append(act);
}


HashCalculator::~HashCalculator()
{
}


void HashCalculator::setFileName(const QStringList &file_name_list)
{
    m_file_name_list = file_name_list;
    //qDebug() << file_name_list;
}


void HashCalculator::release()
{
    delete mp_control_dialog;
    mp_control_dialog = 0;
}


QList<DataAction> HashCalculator::getActionList() const
{
    return m_action_list;
}


void HashCalculator::runAction(int act_id)
{
    if (mp_control_dialog == 0)
    {
        mp_control_dialog = new ControlDialog();
    }
    if (act_id == 1)
    {
        mp_control_dialog->setFileNames(m_file_name_list);
        mp_control_dialog->exec();
    }
}


QTranslator *HashCalculator::getTranslator(const QLocale &locale)
{
    translator = new QTranslator();
    QString slocale = ":/lang/";
    slocale += locale.name();
    if(!translator->load(slocale))
    {
        translator->deleteLater();
        translator = 0;
    }

    return translator;
}


QString HashCalculator::getLastError() const
{
    return QString("");
}


QStringList HashCalculator::pluginInfo() const
{
    QStringList pinfo;
    pinfo << QString("Plugin: ") + tr("HashCalculator");
    pinfo << QString("Authors: ") + tr("Alexey Schukin");
    pinfo << QString("Place: Ukraine, Kiev, 2012-2013");
    pinfo << QString("Build date: ") + QString("2013-02-03");
    pinfo << QString("Version: ") + QString("0.1");
    pinfo << QString("Contacts: mailto:mks-mail@ukr.net");
    pinfo << QString("Lic: GNU/GPL v3");
    pinfo << QString("Description: ") + tr("Модуль для подсчёта контрольных сумм MD5 и SHA1");
    return pinfo;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(hashcalculator, HashCalculator)
#endif
