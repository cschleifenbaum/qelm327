TEMPLATE = lib
TARGET = elm327canbus

QT = core serialbus serialport

HEADERS += \
    elm327canbusdevice.h

SOURCES += \
    elm327canbusdevice.cpp \
    main.cpp

DISTFILES = plugin.json
