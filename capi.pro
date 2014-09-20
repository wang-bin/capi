QT = core gui
# g++: ((T*)0)->member
QMAKE_CXXFLAGS += -Wno-invalid-offsetof

SOURCES += \
    testz.cpp
