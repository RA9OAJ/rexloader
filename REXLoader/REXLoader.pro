#-------------------------------------------------
#
# Project created by QtCreator 2011-05-27T18:26:47
#
#-------------------------------------------------

QT       += core gui sql

TARGET = REXLoader
TEMPLATE = app

DESTDIR = ../usr/bin

SOURCES += main.cpp\
        rexwindow.cpp \
    titemmodel.cpp \
    addtaskdialog.cpp \
    emessagebox.cpp \
    treeitemmodel.cpp \
    settingsdialog.cpp

HEADERS  += rexwindow.h \
    titemmodel.h \
    addtaskdialog.h \
    emessagebox.h \
    treeitemmodel.h \
    settingsdialog.h

FORMS    += rexwindow.ui \
    addtaskdialog.ui \
    settingsdialog.ui

RESOURCES += \
    resources.qrc
