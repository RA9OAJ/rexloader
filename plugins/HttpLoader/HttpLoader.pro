QT -= gui
QT += network
CONFIG += plugin
TARGET = HttpLoader
TEMPLATE = lib
CODECFORTR = UTF-8

unix: {
    LIBS += -lz
}
DESTDIR = ../../usr/lib/rexloader/plugins

SOURCES = httploader.cpp \
    httpsection.cpp \
    gtcpsocket.cpp

HEADERS = httploader.h \
    httpsection.h \
    gtcpsocket.h

TRANSLATIONS = lang/ru_RU.ts

RESOURCES += \
    resources.qrc

MOC_DIR        = build/moc_dir
OBJECTS_DIR    = build/obj_dir
UI_HEADERS_DIR = build/ui/h
UI_SOURCES_DIR = build/ui/cpp
RCC_DIR        = build/res

include (lang/locales.pri)
