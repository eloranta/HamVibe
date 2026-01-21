QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

INCLUDEPATH += "C:/Program Files/hamlib-w64-4.6.5/include"
LIBS += "C:/Program Files/hamlib-w64-4.6.5/lib/gcc/libhamlib.dll.a"

TARGET = HamVibe

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    frequencylabel.cpp \
    rig.cpp

HEADERS += \
    mainwindow.h \
    frequencylabel.h \
    rig.h

FORMS += \
    mainwindow.ui

QSS_FILE = hamvibe.qss
DISTFILES += $$QSS_FILE
WIN_COPY = cmd /c copy /y
HAMLIB_DLL = $$shell_path(C:/Program Files/hamlib-w64-4.6.5/bin/libhamlib-4.dll)
LIBUSB_DLL = $$shell_path(C:/Program Files/hamlib-w64-4.6.5/bin/libusb-1.0.dll)
CONFIG(debug, debug|release) {
    BUILD_SUBDIR = debug
} else {
    BUILD_SUBDIR = release
}
win32: QMAKE_POST_LINK += $$WIN_COPY $$shell_path($$PWD/$$QSS_FILE) $$shell_path($$OUT_PWD/$$DESTDIR/$$BUILD_SUBDIR)

# TODO: copy dll win32: QMAKE_POST_LINK += $$quote($$WIN_COPY \"$$HAMLIB_DLL\" \"$$shell_path($$OUT_PWD/$$(DESTDIR)/$$BUILD_SUBDIR/libhamlib-4.dll)\")
# TODO: copy dll win32: QMAKE_POST_LINK += $$quote($$WIN_COPY \"$$LIBUSB_DLL\" \"$$shell_path($$OUT_PWD/$$(DESTDIR)/$$BUILD_SUBDIR/libusb-1.0.dll)\")
# TODO:else: QMAKE_POST_LINK += $$QMAKE_COPY $$shell_path($$PWD/$$QSS_FILE) $$shell_path($$OUT_PWD/$$(DESTDIR)/$$QSS_FILE)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
