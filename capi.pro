TEMPLATE = subdirs
SUBDIRS = libcapi

#CONFIG += build_tests
build_tests: {
SUBDIRS = \
        test/zlib \
        test/sdl
}
libcapi.file = libcapi.pro
