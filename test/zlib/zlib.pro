QT = core
TEMPLATE = app
CONFIG -= app_bundle
SOURCES += \
    zlib_api.cpp \
    zlib_api_test.cpp
HEADERS += zlib_api.h
#LIBS += -lz
#DEFINES += CAPI_LINK_ZLIB
include(../../capi.pri)
