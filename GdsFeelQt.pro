# -------------------------------------------------
# Project created by QtCreator 2009-06-26T11:39:10
# -------------------------------------------------
CONFIG += c++11
TARGET = GdsFeelQt
SUBDIRS = GdsFeelCore
debug {
  CONFIG += console
}
QT += widgets
QT += xml
TEMPLATE = app
HEADERS += mainwindow.h \
    elementdrawer.h
SOURCES += mainwindow.cpp \
    main.cpp \
    elementdrawer.cpp
FORMS += mainwindow.ui
LIBS += -L$$PWD/GdsFeelCore/ \
    -lGdsFeelCore
LIBS += -lz
