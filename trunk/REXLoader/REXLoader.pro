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
    settingsdialog.cpp \
    importdialog.cpp \
    categorydialog.cpp

HEADERS  += rexwindow.h \
    titemmodel.h \
    addtaskdialog.h \
    emessagebox.h \
    treeitemmodel.h \
    settingsdialog.h \
    importdialog.h \
    categorydialog.h

FORMS    += rexwindow.ui \
    addtaskdialog.ui \
    settingsdialog.ui \
    importdialog.ui \
    categorydialog.ui

TRANSLATIONS += lang/ru_RU.ts

RESOURCES += \
    resources.qrc
