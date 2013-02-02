#ifndef HASHCALCULATOR_H
#define HASHCALCULATOR_H

#include <QtCore>
#include <QDialog>
#include "../FileInterface.h"

class HashCalculator : public QObject,
                       public FileInterface
{
    Q_OBJECT
    Q_INTERFACES(FileInterface)

public:
    HashCalculator();
    virtual ~HashCalculator();
    //
    virtual void setFileName(const QString &file_name);
    virtual void release();
    virtual QString getLastError() const;
    virtual QStringList pluginInfo() const;
    virtual QAction *getRunAction() const;

private:
    QString m_file_name;
    QDialog *mp_control_dialog;
    QAction *mp_run_action;

private slots:
    void run();
};

#endif // HASHCALCULATOR_H
