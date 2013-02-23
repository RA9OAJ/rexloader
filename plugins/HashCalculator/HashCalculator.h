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
    //HashCalculatorThread *mp_hash_calc;

private slots:
    //void run();
};

#endif // HASHCALCULATOR_H
