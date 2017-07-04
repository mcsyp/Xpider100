QT += qml quick

CONFIG += c++11

SOURCES += main.cpp \
    ../HelloXpiderCenter/hdlc_qt.cpp \
    ../HelloXpiderCenter/optiprotocol.cpp \
    ../HelloXpiderCenter/optiserver.cpp \
    ../HelloXpiderCenter/xpiderhdlcencoder.cpp \
    ../HelloXpiderCenter/xpiderpool.cpp \
    ../HelloXpiderCenter/xpider_ctl/xpider_info.cpp \
    ../HelloXpiderCenter/xpider_ctl/xpider_protocol.cpp \
    ../HelloXpiderCenter/xpidersocket.cpp \
    ../HelloXpiderCenter/xpiderinstance.cpp \
    ../HelloXpiderCenter/xpider_location.cpp \
    ../HelloXpiderCenter/trajectoryplanner.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH+="../HelloXpdierCenter"
INCLUDEPATH+="../HelloXpdierCenter/xpider_ctl"

HEADERS += \
    ../HelloXpiderCenter/global_xpier.h \
    ../HelloXpiderCenter/hdlc_qt.h \
    ../HelloXpiderCenter/optiprotocol.h \
    ../HelloXpiderCenter/optiserver.h \
    ../HelloXpiderCenter/xpiderhdlcencoder.h \
    ../HelloXpiderCenter/xpiderpool.h \
    ../HelloXpiderCenter/xpider_ctl/xpider_info.h \
    ../HelloXpiderCenter/xpider_ctl/xpider_protocol.h \
    ../HelloXpiderCenter/xpidersocket.h \
    ../HelloXpiderCenter/xpiderinstance.h \
    ../HelloXpiderCenter/xpider_location.h \
    ../HelloXpiderCenter/xpider_ctl/arduino_log.h \
    ../HelloXpiderCenter/xpider_ctl/linked_list.h \
    ../HelloXpiderCenter/trajectoryplanner.h

DISTFILES += \
    main.qml
