#-------------------------------------------------
#
# Project created by QtCreator 2018-06-15T12:46:27
#
#-------------------------------------------------

QT       += widgets core gui printsupport network

TARGET = SWIM
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

win32 {
    RC_ICONS = icons/NHERI-SWIM-Icon.ico
} else {
    mac {
    ICON = icons/NHERI-SWIM-Icon.icns
    }
}

# load SimCenter Common
include(../../SimCenterCommon/Common/Common.pri)

# load OpenSees
include(OPS_includes.pro)

#win32: INCLUDES += "./ops"
#win32: LIBS += -llapack -lblas
#unix:  QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-variable -std=c++11
#win32: QMAKE_CXXFLAGS += /Y-

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#win32: QMAKE_CXXFLAGS += -D_USRDLL
#win32: QMAKE_CXXFLAGS += -D_DLL

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    experiment.cpp \
    responsewidget.cpp \
    resp.cpp \
    historywidget.cpp \
    deformwidget.cpp \
    hysteresiswidget.cpp \
    qcustomplot/qcustomplot.cpp

HEADERS += \
    mainwindow.h \
    experiment.h \
    responsewidget.h \
    resp.h \
    historywidget.h \
    deformwidget.h \
    hysteresiswidget.h \
    qcustomplot/qcustomplot.h

#FORMS += \
#        mainwindow.ui

RESOURCES += \
    aiscshapedatabase.qrc \
    tests.qrc \
    braceImages.qrc
