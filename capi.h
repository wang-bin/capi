/******************************************************************************
    Use C API in C++ dynamically and no link. Header only.
    Use it with a code generation tool: https://github.com/wang-bin/mkapi
    Copyright (C) 2014-2016 Wang Bin <wbsecg1@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/
// no class based implementation: https://github.com/wang-bin/dllapi . limitation: can not reload library
#ifndef CAPI_H
#define CAPI_H

/*!
    How To Use: (see test/zlib)
    use the header and source from code gerenrated from: https://github.com/wang-bin/mkapi
 */

#include <cstddef> //ptrdiff_t
#include <cstdio>
#include <cassert>
#include <string.h>

#define CAPI_IS(X) (defined CAPI_IS_##X && CAPI_IS_##X)
/*!
 * you can define CAPI_IS_LAZY_RESOLVE 0 before including capi.h. then all symbols will be resolved in constructor.
 * default resolving a symbol at it's first call
 */
#ifndef CAPI_IS_LAZY_RESOLVE
#define CAPI_IS_LAZY_RESOLVE 1
#endif
namespace capi {
namespace version {
    enum {
        Major = 0, Minor = 6, Patch = 0,
        Value = ((Major&0xff)<<16) | ((Minor&0xff)<<8) | (Patch&0xff)
    };
    static const char name[] = { Major + '0', '.', Minor + '0', '.', Patch + '0', 0 };
} //namespace version
// set lib name with version
enum {
    NoVersion = -1, /// library name without major version, for example libz.so
    EndVersion = -2
};
/********************************** The following code is only used in .cpp **************************************************/
/*!
  * -Library names:
    static const char* zlib[] = {
    #ifdef CAPI_TARGET_OS_WIN
      "zlib",
    #else
      "z",
    #endif
      /usr/local/lib/libmyz.so, // absolute path is ok
      NULL};
    CAPI_BEGIN_DLL(zlib, ::capi::dso) // the 2nd parameter is a dynamic shared object loader class. \sa dll_helper class
    ...
  * -Multiple library versions
    An example to open libz.so, libz.so.1, libz.so.0 on unix
    static const int ver[] = { ::capi::NoVersion, 1, 0, ::capi::EndVersion };
    CAPI_BEGIN_DLL_VER(zlib, ver, ::capi::dso)
    ...
  */
class dso {
    void *handle;
    char full_name[256];
    dso(const dso&);
    dso& operator=(const dso&);
public:
    dso(): handle(0) {}
    virtual ~dso() { unload();}
    inline void setFileName(const char* name);
    inline void setFileNameAndVersion(const char* name, int ver);
    inline bool load();
    inline bool unload();
    bool isLoaded() const { return !!handle;}
    virtual void* resolve(const char* symbol) { return resolve(symbol, true);}
protected:
    inline void* resolve(const char* sym, bool try_);
};
} //namespace capi
/// DLL_CLASS is a library loader and symbols resolver class. Must implement api like capi::dso (the same function name and return type, but string parameter type can be different):
/// unload() must support ref count. i.e. do unload if no one is using the real library
/// Currently you can use ::capi::dso and QLibrary for DLL_CLASS. You can also use your own library resolver
#define CAPI_BEGIN_DLL(names, DLL_CLASS) \
    class api_dll : public ::capi::internal::dll_helper<DLL_CLASS> { \
    public: api_dll() : ::capi::internal::dll_helper<DLL_CLASS>(names) CAPI_DLL_BODY_DEFINE
#define CAPI_BEGIN_DLL_VER(names, versions, DLL_CLASS) \
    class api_dll : public ::capi::internal::dll_helper<DLL_CLASS> { \
    public: api_dll() : ::capi::internal::dll_helper<DLL_CLASS>(names, versions) CAPI_DLL_BODY_DEFINE
#if CAPI_IS(LAZY_RESOLVE)
#define CAPI_END_DLL() } api_t; api_t api; };
#else
#define CAPI_END_DLL() };
#endif
#define CAPI_DEFINE_DLL api::api():dll(new api_dll()){} \
    api::~api(){delete dll;} \
    bool api::loaded() const { return dll->isLoaded();} \
    namespace capi { \
        static api_dll* dll = NULL; \
        bool loaded() { \
            if (!dll) dll = new api_dll(); \
            return dll->isLoaded(); \
        } \
    }

/*!
 * N: number of arguments
 * R: return type
 * name: api name
 * ...: api arguments with only types, wrapped by CAPI_ARGn, n=0,1,2,...13
 * The symbol of the api is "name". Otherwise, use CAPI_DEFINE instead.
 * example:
 * 1. const char* zlibVersion()
 *    CAPI_DEFINE(const char* zlibVersion, CAPI_ARG0())
 * 2. const char* zError(int) // get error string from zlib error code
 *    CAPI_DEFINE(const char*, zError, CAPI_ARG1(int))
 */
#if CAPI_IS(LAZY_RESOLVE)
#define CAPI_DEFINE(R, name, ...) EXPAND(CAPI_DEFINE2_X(R, name, name, __VA_ARGS__)) /* not ##__VA_ARGS__ !*/
#define CAPI_DEFINE_ENTRY(R, name, ...) EXPAND(CAPI_DEFINE_ENTRY_X(R, name, name, __VA_ARGS__))
#define CAPI_DEFINE_M_ENTRY(R, M, name, ...) EXPAND(CAPI_DEFINE_M_ENTRY_X(R, M, name, name, __VA_ARGS__))
#else
#define CAPI_DEFINE(R, name, ...) EXPAND(CAPI_DEFINE_X(R, name, __VA_ARGS__)) /* not ##__VA_ARGS__ !*/
#define CAPI_DEFINE_ENTRY(R, name, ...) EXPAND(CAPI_DEFINE_RESOLVER_X(R, name, name, __VA_ARGS__))
#define CAPI_DEFINE_M_ENTRY(R, M, name, ...) EXPAND(CAPI_DEFINE_M_RESOLVER_X(R, M, name, name, __VA_ARGS__))
#endif
//EXPAND(CAPI_DEFINE##N(R, name, #name, __VA_ARGS__))

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/************The followings are used internally**********/
#if CAPI_IS(LAZY_RESOLVE)
#define CAPI_DLL_BODY_DEFINE { memset(&api, 0, sizeof(api));} typedef struct {
#else
#define CAPI_DLL_BODY_DEFINE { CAPI_DBG_RESOLVE("capi resolved dll symbols...");}
#endif
#define CAPI_DEFINE_T_V(R, name, ARG_T, ARG_T_V, ARG_V) \
    R api::name ARG_T_V { \
        CAPI_DBG_CALL(" "); \
        assert(dll && dll->isLoaded() && "dll is not loaded"); \
        return dll->name ARG_V; \
    }
#define CAPI_DEFINE2_T_V(R, name, sym, ARG_T, ARG_T_V, ARG_V) \
    R api::name ARG_T_V { \
        CAPI_DBG_CALL(" "); \
        assert(dll && dll->isLoaded() && "dll is not loaded"); \
        if (!dll->api.name) { \
            dll->api.name = (api_dll::api_t::name##_t)dll->resolve(#sym); \
            CAPI_DBG_RESOLVE("dll::api_t::" #name ": @%p", dll->api.name); \
        } \
        assert(dll->api.name && "failed to resolve " #R #sym #ARG_T_V); \
        return dll->api.name ARG_V; \
    }
/*
 * TODO: choose 1 of below
 * - use CAPI_LINKAGE and remove CAPI_DEFINE_M_ENTRY_X & CAPI_DEFINE_M_RESOLVER_X
 * - also pass a linkage parameter to CAPI_NS_DEFINE_T_V & CAPI_NS_DEFINE2_T_V
 */
#ifndef CAPI_LINKAGE
#define CAPI_LINKAGE
#endif //CAPI_LINKAGE
#define CAPI_NS_DEFINE_T_V(R, name, ARG_T, ARG_T_V, ARG_V) \
    namespace capi { \
    using capi::dll; \
    R CAPI_LINKAGE name ARG_T_V { \
        CAPI_DBG_CALL(" "); \
        if (!dll) dll = new api_dll(); \
        assert(dll && dll->isLoaded() && "dll is not loaded"); \
        return dll->name ARG_V; \
    } }
#define CAPI_NS_DEFINE2_T_V(R, name, sym, ARG_T, ARG_T_V, ARG_V) \
    namespace capi { \
    using capi::dll; \
    R CAPI_LINKAGE name ARG_T_V { \
        CAPI_DBG_CALL(" "); \
        if (!dll) dll = new api_dll(); \
        assert(dll && dll->isLoaded() && "dll is not loaded"); \
        if (!dll->api.name) { \
            dll->api.name = (api_dll::api_t::name##_t)dll->resolve(#sym); \
            CAPI_DBG_RESOLVE("dll::api_t::" #name ": @%p", dll->api.name); \
        } \
        assert(dll->api.name && "failed to resolve " #R #sym #ARG_T_V); \
        return dll->api.name ARG_V; \
    } }

// nested class can not call non-static members outside the class, so hack the address here
// need -Wno-invalid-offsetof
#define CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, ARG_T, ARG_T_V, ARG_V) \
    public: \
        typedef R (M *name##_t) ARG_T; \
        name##_t name; \
    private: \
        struct name##_resolver_t { \
            name##_resolver_t() { \
                const ptrdiff_t diff = ptrdiff_t(&((api_dll*)0)->name##_resolver) - ptrdiff_t(&((api_dll*)0)->name); \
                name##_t *p = (name##_t*)((char*)this - diff); \
                api_dll* dll = (api_dll*)((char*)this - ((ptrdiff_t)(&((api_dll*)0)->name##_resolver))); \
                if (!dll->isLoaded()) { \
                    CAPI_WARN_LOAD("dll not loaded"); \
                    *p = NULL; \
                    return; \
                } \
                *p = (name##_t)dll->resolve(#sym); \
                if (*p) { CAPI_DBG_RESOLVE("dll::" #name ": @%p", *p); } \
                else { CAPI_WARN_RESOLVE("capi resolve error '" #name "'"); } \
            } \
        } name##_resolver;

#if defined(__GNUC__)
#  define CAPI_FUNC_INFO __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#  define CAPI_FUNC_INFO __FUNCSIG__
#else
#  define CAPI_FUNC_INFO __FUNCTION__
#endif
#ifdef DEBUG
#define DEBUG_LOAD
#define DEBUG_RESOLVE
#define DEBUG_CALL
#endif //DEBUG
#if defined(DEBUG) || defined(DEBUG_LOAD) || defined(DEBUG_RESOLVE) || defined(DEBUG_CALL)
#define CAPI_LOG(STDWHERE, fmt, ...) do {fprintf(STDWHERE, "[%s] %s@%d: " fmt "\n", __FILE__, CAPI_FUNC_INFO, __LINE__, ##__VA_ARGS__); fflush(0);} while(0);
#else
#define CAPI_LOG(...)
#endif //DEBUG
#ifdef DEBUG_LOAD
#define CAPI_DBG_LOAD(...) EXPAND(CAPI_LOG(stdout, ##__VA_ARGS__))
#define CAPI_WARN_LOAD(...) EXPAND(CAPI_LOG(stderr, ##__VA_ARGS__))
#else
#define CAPI_DBG_LOAD(...)
#define CAPI_WARN_LOAD(...)
#endif //DEBUG_LOAD
#ifdef DEBUG_RESOLVE
#define CAPI_DBG_RESOLVE(...) EXPAND(CAPI_LOG(stdout, ##__VA_ARGS__))
#define CAPI_WARN_RESOLVE(...) EXPAND(CAPI_LOG(stderr, ##__VA_ARGS__))
#else
#define CAPI_DBG_RESOLVE(...)
#define CAPI_WARN_RESOLVE(...)
#endif //DEBUG_RESOLVE
#ifdef DEBUG_CALL
#define CAPI_DBG_CALL(...) EXPAND(CAPI_LOG(stdout, ##__VA_ARGS__))
#define CAPI_WARN_CALL(...) EXPAND(CAPI_LOG(stderr, ##__VA_ARGS__))
#else
#define CAPI_DBG_CALL(...)
#define CAPI_WARN_CALL(...)
#endif //DEBUG_CALL
//fully expand. used by VC. VC will not expand __VA_ARGS__ but treats it as 1 parameter
#define EXPAND(expr) expr //TODO: rename CAPI_EXPAND
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) \
    || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) \
    || defined(WINCE) || defined(_WIN32_WCE)
#define CAPI_TARGET_OS_WIN 1
#include <windows.h>
#ifdef WINAPI_FAMILY
#include <winapifamily.h>
#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define CAPI_TARGET_OS_WINRT 1
#endif
#endif //WINAPI_FAMILY
#endif
#if defined(__APPLE__)
#define CAPI_TARGET_OS_MAC 1
#endif
#ifndef CAPI_TARGET_OS_WIN
#include <dlfcn.h>
#endif
namespace capi {
namespace internal {
// the following code is for the case DLL=QLibrary + QT_NO_CAST_FROM_ASCII
// you can add a new qstr_wrap like class and a specialization of dso_trait to support a new string type before/after include "capi.h"
struct qstr_wrap {
    static const char* fromLatin1(const char* s) {return s;}
};
template<class T> struct dso_trait {
    typedef const char* str_t;
    typedef qstr_wrap qstr_t;
};
//can not explicit specialization of 'trait' in class scope
#if defined(QLIBRARY_H) && defined(QT_CORE_LIB)
template<> struct dso_trait<QLibrary> {
    typedef QString str_t;
    typedef QString qstr_t;
};
#endif
// base ctor dll_helper("name")=>derived members in decl order(resolvers)=>derived ctor
static const int kDefaultVersions[] = {::capi::NoVersion, ::capi::EndVersion};
template <class DLL> class dll_helper { //no CAPI_EXPORT required
    DLL m_lib;
    typename dso_trait<DLL>::str_t strType(const char* s) {
        return dso_trait<DLL>::qstr_t::fromLatin1(s);
    }
public:
    dll_helper(const char* names[], const int versions[] = kDefaultVersions) {
        static bool is_1st = true;
        if (is_1st) {
            is_1st = false;
            fprintf(stderr, "capi::version: %s\n", ::capi::version::name); fflush(0);
        }
        for (int i = 0; names[i]; ++i) {
            for (int j = 0; versions[j] != ::capi::EndVersion; ++j) {
                if (versions[j] == ::capi::NoVersion)
                    m_lib.setFileName(strType(names[i]));
                else
                    m_lib.setFileNameAndVersion(strType(names[i]), versions[j]);
                if (m_lib.load()) {
                    CAPI_DBG_LOAD("capi loaded {library name: %s, version: %d}", names[i], versions[j]);
                    return;
                }
                CAPI_WARN_LOAD("capi can not load {library name: %s, version %d}", names[i], versions[j]);
            }
        }
    }
    virtual ~dll_helper() { m_lib.unload();}
    bool isLoaded() const { return m_lib.isLoaded(); }
    void* resolve(const char *symbol) { return (void*)m_lib.resolve(symbol);}
};
#ifdef CAPI_TARGET_OS_WIN
    static const char kPre[] = "";
    static const char kExt[] = ".dll";
#else
    static const char kPre[] = "lib";
#ifdef CAPI_TARGET_OS_MAC
    static const char kExt[] = ".dylib";
#else
    static const char kExt[] = ".so";
#endif
#endif
} //namespace internal
#ifdef CAPI_TARGET_OS_WIN
#define CAPI_SNPRINTF _snprintf
#define CAPI_SNWPRINTF _snwprintf
#else
#define CAPI_SNPRINTF snprintf
#define CAPI_SNWPRINTF snwprintf
#endif
void dso::setFileName(const char* name) {
    CAPI_DBG_LOAD("dso.setFileName(\"%s\")", name);
    if (name[0] == '/')
        CAPI_SNPRINTF(full_name, sizeof(full_name), "%s", name);
    else
        CAPI_SNPRINTF(full_name, sizeof(full_name), "%s%s%s", internal::kPre, name, internal::kExt);
}
void dso::setFileNameAndVersion(const char* name, int ver) {
    CAPI_DBG_LOAD("dso.setFileNameAndVersion(\"%s\", %d)", name, ver);
    if (ver >= 0) {
#if defined(CAPI_TARGET_OS_WIN) // ignore version on win. xxx-V.dll?
        CAPI_SNPRINTF(full_name, sizeof(full_name), "%s%s%s", internal::kPre, name, internal::kExt);
#elif defined(CAPI_TARGET_OS_MAC)
        CAPI_SNPRINTF(full_name, sizeof(full_name), "%s%s.%d%s", internal::kPre, name, ver, internal::kExt);
#else
        CAPI_SNPRINTF(full_name, sizeof(full_name), "%s%s%s.%d", internal::kPre, name, internal::kExt, ver);
#endif
    } else {
        setFileName(name);
    }
}
bool dso::load() {
    CAPI_DBG_LOAD("dso.load: %s", full_name);
#ifdef CAPI_TARGET_OS_WIN
#ifdef CAPI_TARGET_OS_WINRT
    wchar_t wname[sizeof(full_name)];
    CAPI_SNWPRINTF(wname, sizeof(wname), L"%s", full_name);
    handle = (void*)::LoadPackagedLibrary(wname, 0);
#else
    handle = (void*)::LoadLibraryExA(full_name, NULL, 0); //DONT_RESOLVE_DLL_REFERENCES
#endif
#else
    handle = ::dlopen(full_name, RTLD_LAZY|RTLD_LOCAL); // try no prefix name if error?
#endif
    return !!handle;
}
bool dso::unload() {
    if (!isLoaded())
        return true;
#ifdef CAPI_TARGET_OS_WIN
    if (!::FreeLibrary(static_cast<HMODULE>(handle))) //return 0 if error. ref counted
        return false;
#else
    if (::dlclose(handle) != 0) //ref counted
        return false;
#endif
    handle = NULL; //TODO: check ref?
    return true;
}
void* dso::resolve(const char* sym, bool try_) {
    const char* s = sym;
    char _s[512]; // old a.out systems add an underscore in front of symbols
    if (!try_) {//previous has no '_', now has '_'
        CAPI_SNPRINTF(_s, sizeof(_s), "_%s", sym);
        s = _s;
    }
    CAPI_DBG_RESOLVE("dso.resolve(\"%s\", %d)", s, try_);
#ifdef CAPI_TARGET_OS_WIN
    void *ptr = (void*)::GetProcAddress((HMODULE)handle, s);
#else
    void *ptr = ::dlsym(handle, s);
#endif
    if (!ptr && try_)
        return resolve(sym, false);
    return ptr;
}
} //namespace capi

#if defined(_MSC_VER)
#pragma warning(disable:4098) //vc return void
#endif //_MSC_VER
#ifdef __GNUC__
//gcc: ((T*)0)->member
//no #pragma GCC diagnostic push/pop around because code is defined as a macro
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
/*!
 * used by .cpp to define the api
 *  e.g. CAPI_DEFINE(cl_int, clGetPlatformIDs, "clGetPlatformIDs", CAPI_ARG3(cl_uint, cl_platform_id*, cl_uint*))
 * sym: symbol of the api in library.
 * Defines both namespace style and class style. User can choose which one to use at runtime by adding a macro before including the header or not: #define SOMELIB_CAPI_NS
 * See test/zlib/zlib_api.h
 */
#define CAPI_DEFINE_X(R, name, ARG_T, ARG_T_V, ARG_V) \
    CAPI_DEFINE_T_V(R, name, ARG_T, ARG_T_V, ARG_V) \
    CAPI_NS_DEFINE_T_V(R, name, ARG_T, ARG_T_V, ARG_V)
/* declare and define the symbol resolvers*/
#define EMPTY_LINKAGE
#define CAPI_DEFINE_RESOLVER_X(R, name, sym, ARG_T, ARG_T_V, ARG_V) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, ARG_T, ARG_T_V, ARG_V)
// api with linkage modifier
#define CAPI_DEFINE_M_RESOLVER_X(R, M, name, sym, ARG_T, ARG_T_V, ARG_V) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, ARG_T, ARG_T_V, ARG_V)

#define CAPI_DEFINE2_X(R, name, sym, ARG_T, ARG_T_V, ARG_V) \
    CAPI_DEFINE2_T_V(R, name, sym, ARG_T, ARG_T_V, ARG_V) \
    CAPI_NS_DEFINE2_T_V(R, name, sym, ARG_T, ARG_T_V, ARG_V)
#define CAPI_DEFINE_ENTRY_X(R, name, sym, ARG_T, ARG_T_V, ARG_V) CAPI_DEFINE_M_ENTRY_X(R, EMPTY_LINKAGE, name, sym, ARG_T, ARG_T_V, ARG_V)
#define CAPI_DEFINE_M_ENTRY_X(R, M, sym, name, ARG_T, ARG_T_V, ARG_V) typedef R (M *name##_t) ARG_T; name##_t name;

#define CAPI_ARG0() (), (), ()
#define CAPI_ARG1(P1) (P1), (P1 p1), (p1)
#define CAPI_ARG2(P1, P2) (P1, P2), (P1 p1, P2 p2), (p1, p2)
#define CAPI_ARG3(P1, P2, P3) (P1, P2, P3), (P1 p1, P2 p2, P3 p3), (p1, p2, p3)
#define CAPI_ARG4(P1, P2, P3, P4) (P1, P2, P3, P4), (P1 p1, P2 p2, P3 p3, P4 p4), (p1, p2, p3, p4)
#define CAPI_ARG5(P1, P2, P3, P4, P5) (P1, P2, P3, P4, P5), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5), (p1, p2, p3, p4, p5)
#define CAPI_ARG6(P1, P2, P3, P4, P5, P6) (P1, P2, P3, P4, P5, P6), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6), (p1, p2, p3, p4, p5, p6)
#define CAPI_ARG7(P1, P2, P3, P4, P5, P6, P7) (P1, P2, P3, P4, P5, P6, P7), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7), (p1, p2, p3, p4, p5, p6, p7)
#define CAPI_ARG8(P1, P2, P3, P4, P5, P6, P7, P8) (P1, P2, P3, P4, P5, P6, P7, P8), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8), (p1, p2, p3, p4, p5, p6, p7, p8)
#define CAPI_ARG9(P1, P2, P3, P4, P5, P6, P7, P8, P9) (P1, P2, P3, P4, P5, P6, P7, P8, P9), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9), (p1, p2, p3, p4, p5, p6, p7, p8, p9)
#define CAPI_ARG10(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
#define CAPI_ARG11(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11)
#define CAPI_ARG12(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12)
#define CAPI_ARG13(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12, P13 p13), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13)

#endif // CAPI_H
