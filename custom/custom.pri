message("Adding Custom Plugin")

#-- Version control
#   Major and minor versions are defined here (manually)

CUSTOM_QGC_VER_MAJOR = 0
CUSTOM_QGC_VER_MINOR = 0
CUSTOM_QGC_VER_FIRST_BUILD = 0

# Build number is automatic
# Uses the current branch. This way it works on any branch including build-server's PR branches
CUSTOM_QGC_VER_BUILD = $$system(git --git-dir ../.git rev-list $$GIT_BRANCH --first-parent --count)
win32 {
    CUSTOM_QGC_VER_BUILD = $$system("set /a $$CUSTOM_QGC_VER_BUILD - $$CUSTOM_QGC_VER_FIRST_BUILD")
} else {
    CUSTOM_QGC_VER_BUILD = $$system("echo $(($$CUSTOM_QGC_VER_BUILD - $$CUSTOM_QGC_VER_FIRST_BUILD))")
}
CUSTOM_QGC_VERSION = $${CUSTOM_QGC_VER_MAJOR}.$${CUSTOM_QGC_VER_MINOR}.$${CUSTOM_QGC_VER_BUILD}

DEFINES -= GIT_VERSION=\"\\\"$$GIT_VERSION\\\"\"
DEFINES += GIT_VERSION=\"\\\"$$CUSTOM_QGC_VERSION\\\"\"

message(Custom QGC Version: $${CUSTOM_QGC_VERSION})

# Build a single flight stack by disabling APM support
#MAVLINK_CONF = common
#CONFIG  += QGC_DISABLE_APM_MAVLINK
#CONFIG  += QGC_DISABLE_APM_PLUGIN_FACTORY

# We implement our own PX4 plugin factory
CONFIG  += QGC_DISABLE_PX4_PLUGIN_FACTORY

# Branding

DEFINES += CUSTOMHEADER=\"\\\"CustomPlugin.h\\\"\"
DEFINES += CUSTOMCLASS=CustomPlugin

TARGET   = CustomQGroundControl
DEFINES += QGC_APPLICATION_NAME='"\\\"E38 Mission Control\\\""'

DEFINES += QGC_ORG_NAME=\"\\\"qgroundcontrol.org\\\"\"
DEFINES += QGC_ORG_DOMAIN=\"\\\"org.qgroundcontrol\\\"\"

QGC_APP_NAME        = "E38 Mission Control"
QGC_BINARY_NAME     = "E38 Mission Control"
QGC_ORG_NAME        = "Event38"
QGC_ORG_DOMAIN      = "event38.com"
QGC_ANDROID_PACKAGE = "org.custom.qgroundcontrol"
QGC_APP_DESCRIPTION = "Event38 Ground Control"
QGC_APP_COPYRIGHT   = "Copyright (C) 2022 Event38 Unmanned Systems Development Team. All rights reserved."



# Our own, custom resources
RESOURCES += \
    $$PWD/custom.qrc

QML_IMPORT_PATH += \
   $$PWD/res

# Our own, custom sources
SOURCES += \
    $$PWD/src/CustomPlugin.cc \

HEADERS += \
    $$PWD/src/CustomPlugin.h \

INCLUDEPATH += \
    $$PWD/src \

#-------------------------------------------------------------------------------------
# Custom Firmware/AutoPilot Plugin

INCLUDEPATH += \
    $$PWD/src/FirmwarePlugin \
    $$PWD/src/AutoPilotPlugin

HEADERS+= \
    $$PWD/src/AutoPilotPlugin/CustomAutoPilotPlugin.h \
    $$PWD/src/FirmwarePlugin/CustomFirmwarePlugin.h \
    $$PWD/src/FirmwarePlugin/CustomFirmwarePluginFactory.h \

SOURCES += \
    $$PWD/src/AutoPilotPlugin/CustomAutoPilotPlugin.cc \
    $$PWD/src/FirmwarePlugin/CustomFirmwarePlugin.cc \
    $$PWD/src/FirmwarePlugin/CustomFirmwarePluginFactory.cc \

DISTFILES += \
    $$PWD/res/Images/CustomAppIcon.png \
    $$PWD/res/Images/CustomVehicleIcon.svg \
    $$PWD/res/Images/Event38 Icon-2.png \
    $$PWD/res/Images/Event38 Icon-2.png \
    $$PWD/res/Images/Event38 Icon-6.png \
    $$PWD/res/Images/Event38 Icon-6.png \
    $$PWD/res/Images/Event38 Icon.ico \
    $$PWD/res/Images/Event38 Icon.ico \
    $$PWD/res/Images/altitude.svg \
    $$PWD/res/Images/attitude_crosshair.svg \
    $$PWD/res/Images/attitude_dial.svg \
    $$PWD/res/Images/attitude_pointer.svg \
    $$PWD/res/Images/chronometer.svg \
    $$PWD/res/Images/compass_needle.svg \
    $$PWD/res/Images/compass_pointer.svg \
    $$PWD/res/Images/distance.svg \
    $$PWD/res/Images/horizontal_speed.svg \
    $$PWD/res/Images/microSD.svg \
    $$PWD/res/Images/odometer.svg \
    $$PWD/res/Images/vertical_speed.svg \
    $$PWD/res/Images/void.png
