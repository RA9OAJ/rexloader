#-------------------------------------------------
#
# Project created by QtCreator 2011-05-27T18:26:47
#
#-------------------------------------------------

QT       += core gui sql

TARGET = REXLoader
TEMPLATE = app

DESTDIR = ./bin

SOURCES += main.cpp\
        rexwindow.cpp \
    titemmodel.cpp

HEADERS  += rexwindow.h \
    titemmodel.h

FORMS    += rexwindow.ui

RESOURCES += \
    resources.qrc
