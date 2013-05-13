/*
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
