#-------------------------------------------------
#
# Project created by QtCreator 2012-03-15T20:03:11
#
#-------------------------------------------------

QT       += core gui network

TARGET = minigui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    joystick.cpp

HEADERS  += mainwindow.h \
    joystick.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    launch.sh \
    mini2440.skin/mini2440.skin \
    CMakeLists.txt

RESOURCES += \
    res.qrc
