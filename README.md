# capi

This header only tool helps you use C APIs in a shared library by dynamically loading instead of linking against it with minimal efforts.

Only depends on std C++

Here is a simple zlib example, if you want use zlib functions, inherits class zlib::api.

>define a class with zlib functions in zlib_api.h

    #ifndef CAPI_LINK_ZLIB
    namespace zlib { //need a unique namespace
    namespace capi {
    #else
    extern "C" {
    #endif
    #include "zlib.h" //// we need some types define there. otherwise we can remove this
    #ifndef CAPI_LINK_ZLIB
    }
    #endif
    }

    namespace zlib { //need a unique namespace
    #ifndef CAPI_LINK_ZLIB
    using namespace capi;
    #endif
    class api_dll; //must use this name
    class api //must use this name
    {
        api_dll *dll;
    public:
        api();
        virtual ~api();
        virtual bool loaded() const;
    #if !defined(CAPI_LINK_ZLIB) && !defined(ZLIB_CAPI_NS)
        const char* zlibVersion();
        const char* zError(int);
    #endif
    };
    } //namespace zlib
    #ifdef ZLIB_CAPI_NS
    using namespace zlib::capi;
    #else
    using namespace zlib;
    #endif

`zlib_api.h` is the header you will use

>zlib_api.cpp (some code can be generated from  tools/mkapi)

    #define DEBUG //log dll load and symbol resolve
    //#define CAPI_IS_LAZY_RESOLVE 0 //define it will resolve all symbols in constructor
    #include "capi.h"
    #include "zlib_api.h" //include last because zlib.h was in namespace capi to avoid covering types later

    namespace zlib {
    static const char* zlib[] = {
    #ifdef CAPI_TARGET_OS_WIN
        "zlib",
    #else
        "z",
    #endif
        NULL
    };
    static const int versions[] = { 0, ::capi::NoVersion, 1, ::capi::EndVersion };
    CAPI_BEGIN_DLL_VER(zlib, versions, ::capi::dso) // you can also use QLibrary or your custom library resolver instead of ::capi::dso
    CAPI_DEFINE_ENTRY(const char*, zlibVersion, CAPI_ARG0())
    CAPI_DEFINE_ENTRY(const char*, zError, CAPI_ARG1(int))
    CAPI_END_DLL()
    CAPI_DEFINE_DLL
    CAPI_DEFINE(const char*, zlibVersion, CAPI_ARG0())
    CAPI_DEFINE(const char*, zError, CAPI_ARG1(int))
    } //namespace zlib

>test.cpp (dynamically loaded symbols, not link to zlib):

    //#define ZLIB_CAPI_NS // namespace style
    //#define CAPI_LINK_ZLIB // direct linkt to zlib. add -lz is required
    #include <stdio.h>
    #include "zlib_api.h"

    class test_zlib_api
    #ifndef ZLIB_CAPI_NS
                : protected zlib::api // will unload library in dtor
    #endif
    {
    public:
        void test_version() {
            printf("zlib version: %s\n", zlibVersion());
        }
        void test_zError(int e) {
            printf("zlib error: %d => %s\n", e, zError(e));
        }
    };

    int main(int, char **)
    {
        test_zlib_api tt;
        printf("capi zlib test\n");
        tt.test_version();
        tt.test_version();
        tt.test_zError(1);
        return 0;
    }

### 3 Styles

The same code support 3 styles with only 1 line change in you code (test.cpp above)! You can switch class style and namespace without rebuild api implementation object(zlib_api.cpp).

- **Class style(the default)**

  All the functions you call are from class `zlib:api` and your class must be a subclass of it.

- **Namespace style**

  All the functions you call are from namesoace `zlib:capi`. Must add `#define ZLIB_CAPI_NS` before `#include "zlib_api.h"`.

  It's easier to use than class style. It is not the default style because if the library is loaded and unloaded multiple times, symbol addresses from first load(wrong!) are used in current implementation.

- **Direct link style**

  The original functions are called. Must add `#define CAPI_LINK_ZLIB` before `#include "zlib_api.h"`, add `-DCAPI_LINK_ZLIB` to rebuild zlib_api.cpp add `-lz` flags to the compiler

### Lazy Resolve

The symbol is resolved at the first call. You can add `#define CAPI_IS_LAZY_RESOLVE 0` in zlib_api.cpp before `#include "capi.h"` to resolve all symbols as soon as the library is loaded.

### Auto Code Generation

There is a tool to help you generate header and source: https://github.com/wang-bin/mkapi

The tool is based on clang 3.4.

All you need to do is simply run the tool and use the generated files in your project, maybe with a few modifications.

Run `make` to build the tool then run `./mkapi.sh -name zlib zlib.h -I` to generate zlib_api.h and zlib_api.cpp.
