#-------------------------------------------------
#
# Project created by QtCreator 2011-05-27T18:26:47
#
#-------------------------------------------------

QT       += core gui sql

unix{
QT += dbus
}

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
    categorydialog.cpp \
    filenamevalidator.cpp \
    taskdialog.cpp \
    pluginmanager.cpp \
    tableview.cpp \
    colorbutton.cpp \
    fontselectbutton.cpp \
    logtreemodel.cpp \
    logmanager.cpp \
    pluginlistmodel.cpp \
    pluginitemdelegate.cpp \
    shutdownmanager.cpp \
    taskscheduler.cpp \
    importmaster.cpp

HEADERS  += rexwindow.h \
    titemmodel.h \
    addtaskdialog.h \
    emessagebox.h \
    treeitemmodel.h \
    settingsdialog.h \
    importdialog.h \
    categorydialog.h \
    filenamevalidator.h \
    taskdialog.h \
    pluginmanager.h \
    tableview.h \
    colorbutton.h \
    fontselectbutton.h \
    logtreemodel.h \
    logmanager.h \
    pluginlistmodel.h \
    pluginitemdelegate.h \
    shutdownmanager.h \
    taskscheduler.h \
    importmaster.h

FORMS    += rexwindow.ui \
    addtaskdialog.ui \
    settingsdialog.ui \
    importdialog.ui \
    categorydialog.ui \
    taskdialog.ui

RESOURCES += \
    resources.qrc

CODECFORTR = UTF-8
CODECFORSRC = UTF-8

include(locales/locales.pri)

