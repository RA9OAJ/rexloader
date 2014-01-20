TEMPLATE = subdirs

SUBDIRS = HttpLoader HashCalculator #FtpLoader

unix{
    SUBDIRS += NixNotifyPlugin
}
  
HEADERS = LoaderInterface.h NotifInterface.h FileInterface.h
