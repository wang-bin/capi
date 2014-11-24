# g++: ((T*)0)->member
*g++* {
  QMAKE_CXXFLAGS += -Wno-invalid-offsetof
}
INCLUDEPATH += $$PWD
SOURCES += $$PWD/capi.cpp
HEADERS += $$PWD/capi.h
