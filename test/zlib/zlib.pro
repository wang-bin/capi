QT = #core
TEMPLATE = app
CONFIG -= app_bundle
#CONFIG += link_zlib
SOURCES += \
    zlib_api.cpp \
    zlib_api_test.cpp
HEADERS += zlib_api.h
DEFINES += QT_NO_CAST_FROM_ASCII
unix: LIBS *= -ldl
link_zlib {
  LIBS += -lz
  DEFINES += CAPI_LINK_ZLIB
}
include(../../capi.pri)
