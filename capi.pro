QT = core gui
# g++: ((T*)0)->member
*g++* {
QMAKE_CXXFLAGS += -Wno-invalid-offsetof
}
SOURCES += \
    testz.cpp
