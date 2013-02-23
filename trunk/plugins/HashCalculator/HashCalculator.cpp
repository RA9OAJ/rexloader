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
        mp_control_dialog->setFileName(m_file_name_list.value(0));
        mp_control_dialog->exec();
    }
}


QTranslator *HashCalculator::getTranslator(const QLocale &locale)
{
    return 0;
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
    pinfo << QString("Lic: GNU/LGPL v2.1");
    pinfo << QString("Description: ") + tr("Модуль для подсчёта контрольных сумм MD5 и SHA1");
    return pinfo;
}


Q_EXPORT_PLUGIN2(hashcalculator, HashCalculator)
