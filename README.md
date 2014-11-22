# capi

This library helps you use C api in a shared library by dynamically loading instead of linking to it with minimal efforts.

Here is a simple zlib example, if you want use zlib functions, inherits class zlib::api.

>define a class with zlib functions in zlib_api.h

    extern "C" {
    #include "zlib.h" // we need some types define there. otherwise we can remove this
    }
    namespace zlib { //need a unique namespace
    class api_dll; //must use this name
    class api //must use this name
    {
    public:
        api();
        virtual ~api();
        const char* zlibVersion();
        uLong zlibCompileFlags();
    private:
        api_dll *dll;
    };
    } //namespace zlib

`zlib_api.h` is the header you will use

>zlib_api.cpp (some code can be generated from  tools/mkapi)

    #include "zlib_api.h"
    #include "capi.h"
    namespace zlib {
    static const char* zlib[] = { "zlib", "z", NULL}; //zlib.dll, libz.so, libz.dylib
    CAPI_BEGIN_DLL(zlib)
    CAPI_DEFINE_RESOLVER(0, const char*, zlibVersion)
    CAPI_DEFINE_RESOLVER(0, uLong, zlibCompileFlags)
    CAPI_END_DLL()
    api::api() : dll(new api_dll()) {}
    api::~api() { delete dll;}
    CAPI_DEFINE(0, const char*, zlibVersion)
    CAPI_DEFINE(0, uLong, zlibCompileFlags)
    } //namespace zlib

>test.cpp (dynamically loaded symbols):

    #include "zlib_api.h"
    class test_zlib_api : public zlib::api {
    public:
        void test_version() {
            qDebug("START %s", __FUNCTION__);
            qDebug("zlib version: %s", zlibVersion());
            qDebug("STOP %s", __FUNCTION__);
        }
    };
    int main(int, char **)
    {
        test_zlib_api tt;
        tt.test_version();
        return 0;
    }

### Auto Code Generation

There is a tool to help you generate header and source: https://github.com/wang-bin/mkapi

The tool is based on clang 3.4.

Run `make` to build the tool then run `./mkapi.sh -name zlib zlib.h` to generate zlib_api.h and zlib_api.cpp.
