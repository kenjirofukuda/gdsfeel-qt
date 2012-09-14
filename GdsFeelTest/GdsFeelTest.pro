QT += testlib
QT += xml
CONFIG += testlib
CONFIG += console
SOURCES += testconfig.cpp \
    testlibrary.cpp
unix:LIBS += -L../GdsFeelCore/ \
             -lGdsFeelCore
win32:LIBS += -L../GdsFeelCore/debug/ \
              -lGdsFeelCore
