QT += testlib core

CONFIG += console c++17 testcase
CONFIG -= app_bundle

TARGET = tst_calllookup

SOURCES += \
    ../../calllookup.cpp \
    tst_calllookup.cpp

INCLUDEPATH += ../../
