#ifndef FILEINTERFACE_H
#define FILEINTERFACE_H

#include <QtPlugin>
#include <QString>
#include <QStringList>
#include <QAction>
#include <QList>

/*
    Интерфейс для модулей манипулирующих загружеными
    файлами.
*/


class FileInterface
{
public:
    virtual ~FileInterface() {};
    // функция инициализации модуля
    virtual void setFileName(const QStringList &file_name_list) = 0;
    // функция освобождения захваченых ресурсов
    virtual void release() = 0;
    // текстовое описание последней ошибки
    virtual QString getLastError() const = 0;
    // возвращает данные о модуле и его авторах
    virtual QStringList pluginInfo() const = 0;
    // действие запускающее основную функцию модуля
    virtual QList<QAction*> getRunAction() const = 0;
};

Q_DECLARE_INTERFACE(FileInterface, "local.rav.RExLoader.FileInterface/0.1a")

#endif // FILEINTERFACE_H
