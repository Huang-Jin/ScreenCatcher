#-------------------------------------------------
#
# Project created by QtCreator 2017-09-03T15:55:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScreenCatcher
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    screenshotform.cpp \
    iconhelper.cpp

HEADERS  += mainwindow.h \
    screenshotform.h     \
    iconhelper.h

FORMS    += mainwindow.ui \
    screenshotform.ui

RESOURCES += \
    resources.qrc

RC_FILE=icon.rc

include(./3rdparty/QxtGlobalShortcut/QxtGlobalShortcut.pri)
