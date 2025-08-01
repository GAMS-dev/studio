QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    reportdialog.cpp \
    watchdog.cpp \
    ../src/commonpaths.cpp \
    ../src/exception.cpp

HEADERS += \
    checksystem_mac.h \
    checksystem_unix.h \
    reportdialog.h \
    watchdog.h \
    ../src/commonpaths.h \
    ../src/exception.h

FORMS += \
    reportdialog.ui

win32 {
LIBS += -ladvapi32
SOURCES += checksystem_win.cpp
HEADERS += checksystem_win.h
}

# macx {
# SOURCES += checksystem_mac.cpp
# HEADERS += checksystem_mac.h
# }

# unix {
# SOURCES += checksystem_unix.cpp
# HEADERS += checksystem_unix.h
# }

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
