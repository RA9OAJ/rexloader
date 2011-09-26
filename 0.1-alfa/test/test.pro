#-------------------------------------------------
#
# Project created by QtCreator 2011-03-10T14:01:24
#
#-------------------------------------------------

QT       += core gui network plugin

TARGET = test
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp
        #httploader/gtcpsocket.cpp \
        #httploader/httpsection.cpp \
        #httploader/httploader.cpp
        #httploader/LoaderInterface.h

HEADERS  += dialog.h\
        #httploader/gtcpsocket.h\
        #httploader/httpsection.h\
        #httploader/httploader.h \
        ../HttpLoader/LoaderInterface.h

FORMS    += dialog.ui

TRANSLATIONS = test_ru.ts

CODECFORTR = UTF8

CODECFORSRC = UTF8
