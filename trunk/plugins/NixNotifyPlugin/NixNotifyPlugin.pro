#-------------------------------------------------
#
# Project created by QtCreator 2012-12-12T12:26:00
#
#-------------------------------------------------

QT       += dbus
CONFIG += plugin
TARGET = NixNotifyPlugin
TEMPLATE = lib

DESTDIR = ../../usr/lib/rexloader/plugins

SOURCES += nixnotifyplugin.cpp

HEADERS += nixnotifyplugin.h\

RESOURCES += \
    resources.qrc

MOC_DIR        = build/moc_dir
OBJECTS_DIR    = build/obj_dir
UI_HEADERS_DIR = build/ui/h
UI_SOURCES_DIR = build/ui/cpp
RCC_DIR        = build/res



