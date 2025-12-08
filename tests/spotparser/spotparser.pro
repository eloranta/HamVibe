QT += testlib core

CONFIG += console c++17 testcase
CONFIG -= app_bundle

TARGET = tst_spotparser

SOURCES += \
    ../../spotparser.cpp \
    tst_spotparser.cpp

INCLUDEPATH += ../../
