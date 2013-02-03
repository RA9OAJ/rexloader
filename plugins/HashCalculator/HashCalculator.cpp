#include <QtCore>
#include "HashCalculator.h"


HashCalculator::HashCalculator()
{
    QAction *act = new QAction(QString::fromUtf8("запуск"),this);
    m_run_action_list.append(act);

    mp_control_dialog = new QDialog();
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
}


QString HashCalculator::getLastError() const
{
    return QString::fromUtf8("");
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


QList<QAction*> HashCalculator::getRunAction() const
{
    return m_run_action_list;
}


void HashCalculator::run()
{
    //mp_control_dialog->exec();
}


Q_EXPORT_PLUGIN2(hashcalculator, HashCalculator)
