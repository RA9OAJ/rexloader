#-------------------------------------------------
#
# Project created by QtCreator 2011-06-02T13:11:13
#
#-------------------------------------------------

QT       += core gui

TARGET = NoticeWindow
TEMPLATE = app


SOURCES += main.cpp\
        noticewindow.cpp \
    noticemanager.cpp

unix: LIBS += -L/usr/lib64 -L/usr/lib -lX11

HEADERS  += noticewindow.h \
    noticemanager.h

RESOURCES += \
    resources.qrc
