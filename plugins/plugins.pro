TEMPLATE = subdirs

SUBDIRS = HttpLoader HashCalculator

unix{
    SUBDIRS += NixNotifyPlugin
}
  
HEADERS = LoaderInterface.h NotifInterface.h FileInterface.h