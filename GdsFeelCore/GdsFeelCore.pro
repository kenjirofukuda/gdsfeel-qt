#CONFIG += debug
CONFIG += relase
CONFIG += staticlib
#CONFIG -= i386
#CONFIG += ppc
debug {
  CONFIG += console
}
QT += xml
TEMPLATE = lib
#HEADERS += -I/opt/local/include
HEADERS += structure.h \
    qzipwriter_p.h \
    qzipreader_p.h \
    library.h \
    config.h \
    element.h \
    station.h \
    layer.h \
    layers.h
SOURCES += structure.cpp \
    qzip.cpp \
    library.cpp \
    config.cpp \
    element.cpp \
    station.cpp \
    layer.cpp \
    layers.cpp
LIBS += -L./opt/local/lib/
LIBS += -lz
