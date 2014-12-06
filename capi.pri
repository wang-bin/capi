CONFIG *= capi
# g++: ((T*)0)->member
*g++* {
  QMAKE_CXXFLAGS += -Wno-invalid-offsetof
}
INCLUDEPATH += $$PWD
HEADERS += $$PWD/capi.h
