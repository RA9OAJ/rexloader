#include <QtCore>
#include "HashCalculator.h"


HashCalculator::HashCalculator()
{
    mp_control_dialog = 0;
    mp_run_action = 0;
}


HashCalculator::~HashCalculator()
{
}


void HashCalculator::setFileName(const QString &file_name)
{
    m_file_name = file_name;
    mp_control_dialog = new QDialog();
    mp_run_action = new QAction(QString::fromUtf8("123"),this);
}


void HashCalculator::release()
{
    delete mp_control_dialog;
    delete mp_run_action;
}


QString HashCalculator::getLastError() const
{
    return QString::fromUtf8("");
}


QStringList HashCalculator::pluginInfo() const
{
    QStringList list;

    list << QString::fromUtf8("Модуль расчёта контрольных сумм");
    return list;
}


QAction *HashCalculator::getRunAction() const
{
    return mp_run_action;
}


void HashCalculator::run()
{
    mp_control_dialog->exec();
}


Q_EXPORT_PLUGIN2(hashcalculator, HashCalculator)
