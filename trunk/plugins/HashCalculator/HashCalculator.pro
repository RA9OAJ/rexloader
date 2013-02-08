TARGET         = hashcalculator
DESTDIR        = ../../usr/lib/rexloader/plugins
TEMPLATE       = lib
CONFIG        += plugin

DESTDIR = ../../usr/lib/rexloader/plugins

MOC_DIR        = build/moc_dir
OBJECTS_DIR    = build/obj_dir
UI_HEADERS_DIR = build/ui/h
UI_SOURCES_DIR = build/ui/cpp
RCC_DIR        = build/res

SOURCES       += HashCalculator.cpp HashCalculatorThread.cpp \
    controldialog.cpp

HEADERS       += HashCalculator.h HashCalculatorThread.h \
    controldialog.h

FORMS += \
    controldialog.ui
