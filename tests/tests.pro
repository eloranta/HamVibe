QT += testlib widgets
CONFIG += testcase c++17
TEMPLATE = app
TARGET = tst_frequencylabel

INCLUDEPATH += ..

SOURCES += \
    tst_frequencylabel.cpp \
    ../frequencylabel.cpp

HEADERS += \
    ../frequencylabel.h
