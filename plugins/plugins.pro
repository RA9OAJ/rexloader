TEMPLATE = subdirs

SUBDIRS = \
    HttpLoader

unix{
SUBDIRS += \
    NixNotifyPlugin
}
  
HEADERS = \
    LoaderInterface.h\
    NotifInterface.h
