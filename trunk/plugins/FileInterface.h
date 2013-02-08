#ifndef FILEINTERFACE_H
#define FILEINTERFACE_H

#include <QtPlugin>
#include <QString>
#include <QStringList>
#include <QAction>
#include <QList>
#include <QTranslator>

/*
    Интерфейс для модулей манипулирующих загружеными
    файлами.
*/

struct DataAction //структура для передачи данных о возможных действиях плагина
{
    int act_id; // идентификатор дейсвия
    QString act_title; // заголовок действия
    QIcon act_icon; // иконка действия
};

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
    // список пар идентификатор_команды-заголовок_действия
    virtual QList<DataAction> getActionList() const = 0;
    // запускает действие по идентификатору действия
    virtual void runAction(int act_id) = 0;
    //возвращает указатель на транслятор для указанной локали, либо 0 при отсутствии транслятора
    virtual QTranslator* getTranslator(const QLocale &locale) =0;
};

Q_DECLARE_INTERFACE(FileInterface, "local.rav.RExLoader.FileInterface/0.1a")

#endif // FILEINTERFACE_H
