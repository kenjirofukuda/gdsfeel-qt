# -------------------------------------------------
# Project created by QtCreator 2009-06-26T11:39:10
# -------------------------------------------------
TARGET = GdsFeelQt
SUBDIRS = GdsFeelCore
CONFIG += ppc
CONFIG -= i386
debug {
  CONFIG += console
}
QT += xml
TEMPLATE = app
HEADERS += mainwindow.h \
    elementdrawer.h
SOURCES += mainwindow.cpp \
    main.cpp \
    elementdrawer.cpp
FORMS += mainwindow.ui
unix:LIBS += -L./GdsFeelCore/ \
    -lGdsFeelCore
win32:LIBS += -L./GdsFeelCore/debug/ \
    -lGdsFeelCore
LIBS += -L./opt/local/lib/
LIBS += -lz
