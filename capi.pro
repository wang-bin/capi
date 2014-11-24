QT = core
TEMPLATE = subdirs
SUBDIRS = libcapi
libcapi.file = libcapi.pro

CONFIG += build_tests
build_tests: {
SUBDIRS += \
        test/zlib \
        test/sdl
}
