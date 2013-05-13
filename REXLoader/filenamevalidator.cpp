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

#include "filenamevalidator.h"

FileNameValidator::FileNameValidator(QObject *parent) :
    QValidator(parent)
{
}

FileNameValidator::State FileNameValidator::validate(QString &input, int &pos) const
{
    Q_UNUSED(pos)
    if(input.indexOf(QRegExp("[\\\\/:*\?\"<>|]")) > -1)
    {
        input.replace(QRegExp("[\\\\/:*\?\"|]"),"_");
        input.replace(QRegExp("[<]"),"[");
        input.replace(QRegExp("[>]"),"]");
    }
    return QValidator::Acceptable;
}

void FileNameValidator::fixup(QString &input) const
{
    Q_UNUSED(input)
}
