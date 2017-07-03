QT += core network
QT -= gui

CONFIG += c++11

TARGET = HelloXpiderCenter
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    hdlc_qt.cpp \
    xpiderhdlcencoder.cpp \
    xpider_ctl/xpider_info.cpp \
    xpider_ctl/xpider_protocol.cpp \
    xpiderpool.cpp \
    optiserver.cpp \
    optiprotocol.cpp \
    xpiderinstance.cpp \
    xpidersocket.cpp \
    trajectoryplanner.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
    hdlc_qt.h \
    xpiderhdlcencoder.h \
    xpider_ctl/xpider_info.h \
    xpider_ctl/xpider_protocol.h \
    xpider_ctl/xpider_linked_list.h \
    xpiderpool.h \
    optiserver.h \
    optiprotocol.h \
    global_xpier.h \
    xpiderinstance.h \
    xpidersocket.h \
    trajectoryplanner.h
