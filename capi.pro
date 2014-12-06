TEMPLATE = subdirs
include(capi.pri)
CONFIG += build_tests
build_tests: {
SUBDIRS += \
        test/zlib \
        test/sdl
}
