QT = #core
TEMPLATE = app
CONFIG -= app_bundle
SOURCES += \
    sdl_api.cpp \
    sdl_api_test.cpp
HEADERS += sdl_api.h
INCLUDEPATH += /usr/local/include
unix: LIBS *= -ldl
include(../../capi.pri)
