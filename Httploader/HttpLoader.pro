QT -= gui
QT += network
CONFIG += plugin
TARGET = HttpLoader
TEMPLATE = lib
CODECFORTR = UTF-8

unix: {
    LIBS += -lz
}
DESTDIR = ../usr/lib/rexloader/plugins

SOURCES = httploader.cpp \
    httpsection.cpp \
    gtcpsocket.cpp

HEADERS = httploader.h \
    LoaderInterface.h \
    httpsection.h \
    gtcpsocket.h

TRANSLATIONS = lang/ru_RU.ts

RESOURCES += \
    resources.qrc
