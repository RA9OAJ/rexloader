QT += network
CONFIG += plugin
TARGET = FtpLoader
TEMPLATE = lib
CODECFORTR = UTF-8

DESTDIR = ../../usr/lib/rexloader/plugins

SOURCES = ftploader.cpp \
          gtcpsocket.cpp \
    ftpsection.cpp

HEADERS = ftploader.h \
          gtcpsocket.h \
    ftpsection.h

#TRANSLATIONS = lang/ru_RU.ts

#RESOURCES += \
#    resources.qrc

MOC_DIR        = build/moc_dir
OBJECTS_DIR    = build/obj_dir
UI_HEADERS_DIR = build/ui/h
UI_SOURCES_DIR = build/ui/cpp
RCC_DIR        = build/res

#include (lang/locales.pri)
