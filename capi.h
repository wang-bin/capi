/******************************************************************************
    Use C API in C++ dynamically and no link
    Copyright (C) 2014 Wang Bin <wbsecg1@gmail.com>

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

#ifndef CAPI_H
#define CAPI_H

#include <QtDebug>
#include <QtCore/QLibrary>

#if defined(BUILD_CAPI_LIB)
#  undef CAPI_EXPORT
#  define CAPI_EXPORT Q_DECL_EXPORT
#else
#  undef CAPI_EXPORT
// __declspec(dllimport) is better for dll. but static link can not set __declspec(dllimport).
#  define CAPI_EXPORT //Q_DECL_IMPORT //only for vc?
#endif

namespace capi {
namespace version {
    enum {
        Major = 0,
        Minor = 0,
        Patch = 0,
        Value = ((Major&0xff)<<16) | ((Minor&0xff)<<8) | (Patch&0xff)
    };
    static const char name[] = { Major + '0', '.', Minor + '0', '.', Patch + '0', 0 };
    // DO NOT use macro here because it's included outside, the "build" will change
    CAPI_EXPORT const char* build();
} //namespace version

// set lib name with version
enum {
    NoVersion = -1,
    EndVersion = -2
};
} //namespace capi
/********************************** The following code is only used in .cpp **************************************************/
/*!
  * compiler may not support init list {a, b, c}
  * static const char* zlib[] = {
  * #ifdef __WIN32
  *   "zlib",
  * #else
  *   "z",
  * #endif
  *   NULL};
  * CAPI_BEGIN_DLL(zlib)
  * ...
  */
#define CAPI_BEGIN_DLL(names) \
    class api_dll : public capi::dll_helper { \
    public: \
        api_dll() : capi::dll_helper(names) \
        { CAPI_DBG_RESOLVE("dll symbols resolved...");}
/*!
  also defines possible library versions to be use. capi::NoVersion means no version suffix is used,
  e.g. libz.so. Versions array MUST be end with capi::EndVersion;
  below is an example to open libz.so, libz.so.1, libz.so.0 on unix
  static const int ver[] = { capi::NoVersion, 1, 0, capi::EndVersion };
  CAPI_BEGIN_DLL_VER(zlib, ver)
  ...
 */
#define CAPI_BEGIN_DLL_VER(names, versions) \
    class api_dll : public capi::dll_helper { \
    public: \
        api_dll() : capi::dll_helper(names, versions) \
        { CAPI_DBG_RESOLVE("dll symbols resolved...");}

#define CAPI_END_DLL() };

//e.g. CAPI_DEFINE(3, cl_int, clGetPlatformIDs, cl_uint, cl_platform_id*, cl_uint*)
/*!
 * N: number of arguments
 * R: return type
 * name: api name
 * ...: api arguments with only types
 * The symbol of the api is "name". Otherwise, use CAPI_DEFINEN instead.
 */
#define CAPI_DEFINE(N, R, name, ...) \
    EXPAND(CAPI_DEFINE##N(R, name, __VA_ARGS__))
#define CAPI_DEFINE_RESOLVER(N, R, name, ...) \
    EXPAND(CAPI_DEFINE_RESOLVER##N(R, name, name, __VA_ARGS__))
#define CAPI_DEFINE_M_RESOLVER(N, R, M, name, ...) \
    EXPAND(CAPI_DEFINE_M_RESOLVER##N(R, M, name, name, __VA_ARGS__))
//EXPAND(CAPI_DEFINE##N(R, name, #name, __VA_ARGS__))


/************The followings are used internally**********/
#define CAPI_DEFINE_T_V(R, name, ARG_T, ARG_T_V, ARG_V) \
    R api::name ARG_T_V { \
        CAPI_DBG_CALL(); \
        Q_ASSERT_X(dll, Q_FUNC_INFO, "class api_dll not initialized"); \
        return dll->name ARG_V; \
    }
// nested class can not call non-static members outside the class, so hack the address here
// need -Wno-invalid-offsetof
#define CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, ARG_T, ARG_T_V, ARG_V) \
    public: \
        typedef R (M *name##_t) ARG_T; \
        name##_t name; \
    private: \
        struct name##_resolver_t { \
            name##_resolver_t() { \
                const qptrdiff diff = qptrdiff(&((api_dll*)0)->name##_resolver) - qptrdiff(&((api_dll*)0)->name); \
                name##_t *p = (name##_t*)((quint8*)this - diff); \
                api_dll* dll = (api_dll*)((quint8*)this - ((qptrdiff)(&((api_dll*)0)->name##_resolver))); \
                if (!dll->isLoaded()) { \
                    CAPI_DBG_LOAD("dll not loaded"); \
                    *p = 0; \
                    return; \
                } \
                *p = (name##_t)dll->resolve(#sym); \
                CAPI_DBG_RESOLVE("dll::" #name ": %p", *p); \
            } \
        } name##_resolver;


namespace capi {
/*
 * base ctor dll_helper("name")=>derived members in decl order(resolvers)=>derived ctor
 */
class CAPI_EXPORT dll_helper {
public:
    dll_helper(const char* names[]);
    dll_helper(const char* names[], const int versions[]);
    virtual ~dll_helper() { m_lib.unload();}
    bool isLoaded() const { return m_lib.isLoaded(); }
    void* resolve(const char *symbol) { return (void*)m_lib.resolve(symbol);}
private:
    QLibrary m_lib;
};

template<typename T> struct IsVoid { enum { value = 0};};
template<> struct IsVoid<void> { enum { value = 1}; };
template<typename T> struct Default { static const T value = T();};
template<> struct Default<void> { static const int value = 0;};
template<typename T> struct Default<T*> { static T* value;};
template<typename T> T* Default<T*>::value = 0;
template<typename T> struct Default<T&> { static T& value;};
template<typename T> T& Default<T&>::value = Default<T>::value; //int*&, int&&
//static const int value = 0; //static const void* value = 0: invalid in-class initialization of static data member of non-integral type 'const void*'
template<> struct Default<void*> { enum { value = 0};};
} //namespace capi


#if defined(_MSC_VER)
#pragma warning(disable:4098) //vc return void
#endif //_MSC_VER

#ifndef Q_FUNC_INFO
#if defined(__GNUC__)
#  define Q_FUNC_INFO __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#  define Q_FUNC_INFO __FUNCSIG__
#else
#  define Q_FUNC_INFO __FUNCTION__
#endif
#endif //Q_FUNC_INFO

#ifdef CAPI_DBG
#undef CAPI_DBG
#endif //CAPI_DBG

#ifdef DEBUG
#define CAPI_DBG(fmt, ...) \
    qDebug("[%s] %s@%d: " fmt, __FILE__, Q_FUNC_INFO, __LINE__, ##__VA_ARGS__);
#define DEBUG_LOAD
#define DEBUG_RESOLVE
#define DEBUG_CALL
#else
#define CAPI_DBG(...)
#endif //DEBUG
#ifdef DEBUG_LOAD
#define CAPI_DBG_LOAD(fmt, ...) \
    qDebug("[%s] %s@%d: " fmt, __FILE__, Q_FUNC_INFO, __LINE__, ##__VA_ARGS__);
#else
#define CAPI_DBG_LOAD(...)
#endif //DEBUG_LOAD
#ifdef DEBUG_RESOLVE
#define CAPI_DBG_RESOLVE(fmt, ...) \
    qDebug("[%s] %s@%d: " fmt, __FILE__, Q_FUNC_INFO, __LINE__, ##__VA_ARGS__);
#else
#define CAPI_DBG_RESOLVE(...)
#endif //DEBUG_RESOLVE
#ifdef DEBUG_CALL
#define CAPI_DBG_CALL(fmt, ...) \
    qDebug("[%s] %s@%d: " fmt, __FILE__, Q_FUNC_INFO, __LINE__, ##__VA_ARGS__);
#else
#define CAPI_DBG_CALL(...)
#endif //DEBUG_CALL

/*!
 * used by .cpp to define the api
 *  e.g. CAPI_DEFINE3(cl_int, clGetPlatformIDs, "clGetPlatformIDs", cl_uint, cl_platform_id*, cl_uint*)
 * sym: symbol of the api in library.
 */
#define CAPI_DEFINE0(R, name, ...) CAPI_DEFINE_T_V(R, name, (), (), ())
#define CAPI_DEFINE1(R, name, P1) CAPI_DEFINE_T_V(R, name, (P1), (P1 p1), (p1))
#define CAPI_DEFINE2(R, name, P1, P2) CAPI_DEFINE_T_V(R, name, (P1, P2), (P1 p1, P2 p2), (p1, p2))
#define CAPI_DEFINE3(R, name, P1, P2, P3) CAPI_DEFINE_T_V(R, name, (P1, P2, P3), (P1 p1, P2 p2, P3 p3), (p1, p2, p3))
#define CAPI_DEFINE4(R, name, P1, P2, P3, P4) CAPI_DEFINE_T_V(R, name, (P1, P2, P3, P4), (P1 p1, P2 p2, P3 p3, P4 p4), (p1, p2, p3, p4))
#define CAPI_DEFINE5(R, name, P1, P2, P3, P4, P5) CAPI_DEFINE_T_V(R, name, (P1, P2, P3, P4, P5), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5), (p1, p2, p3, p4, p5))
#define CAPI_DEFINE6(R, name, P1, P2, P3, P4, P5, P6) CAPI_DEFINE_T_V(R, name, (P1, P2, P3, P4, P5, P6), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6), (p1, p2, p3, p4, p5, p6))
#define CAPI_DEFINE7(R, name, P1, P2, P3, P4, P5, P6, P7) CAPI_DEFINE_T_V(R, name, (P1, P2, P3, P4, P5, P6, P7), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7), (p1, p2, p3, p4, p5, p6, p7))
#define CAPI_DEFINE8(R, name, P1, P2, P3, P4, P5, P6, P7, P8) CAPI_DEFINE_T_V(R, name, (P1, P2, P3, P4, P5, P6, P7, P8), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8), (p1, p2, p3, p4, p5, p6, p7, p8))
#define CAPI_DEFINE9(R, name, P1, P2, P3, P4, P5, P6, P7, P8, P9) CAPI_DEFINE_T_V(R, name, (P1, P2, P3, P4, P5, P6, P7, P8, P9), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9), (p1, p2, p3, p4, p5, p6, p7, p8, p9))
#define CAPI_DEFINE10(R, name, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) CAPI_DEFINE_T_V(R, name, (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10))
#define CAPI_DEFINE11(R, name, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) CAPI_DEFINE_T_V(R, name, (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11))
#define CAPI_DEFINE12(R, name, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) CAPI_DEFINE_T_V(R, name, (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12))
#define CAPI_DEFINE13(R, name, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) CAPI_DEFINE_T_V(R, name, (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12, P13 p13), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13))
/* declare and define the symbol resolvers*/
#define EMPTY_LINKAGE
#define CAPI_DEFINE_RESOLVER0(R, name, sym, ...) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (), (), ())
#define CAPI_DEFINE_RESOLVER1(R, name, sym, P1) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1), (P1 p1), (p1))
#define CAPI_DEFINE_RESOLVER2(R, name, sym, P1, P2) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1, P2), (P1 p1, P2 p2), (p1, p2))
#define CAPI_DEFINE_RESOLVER3(R, name, sym, P1, P2, P3) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1, P2, P3), (P1 p1, P2 p2, P3 p3), (p1, p2, p3))
#define CAPI_DEFINE_RESOLVER4(R, name, sym, P1, P2, P3, P4) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1, P2, P3, P4), (P1 p1, P2 p2, P3 p3, P4 p4), (p1, p2, p3, p4))
#define CAPI_DEFINE_RESOLVER5(R, name, sym, P1, P2, P3, P4, P5) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1, P2, P3, P4, P5), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5), (p1, p2, p3, p4, p5))
#define CAPI_DEFINE_RESOLVER6(R, name, sym, P1, P2, P3, P4, P5, P6) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1, P2, P3, P4, P5, P6), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6), (p1, p2, p3, p4, p5, p6))
#define CAPI_DEFINE_RESOLVER7(R, name, sym, P1, P2, P3, P4, P5, P6, P7) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1, P2, P3, P4, P5, P6, P7), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7), (p1, p2, p3, p4, p5, p6, p7))
#define CAPI_DEFINE_RESOLVER8(R, name, sym, P1, P2, P3, P4, P5, P6, P7, P8) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1, P2, P3, P4, P5, P6, P7, P8), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8), (p1, p2, p3, p4, p5, p6, p7, p8))
#define CAPI_DEFINE_RESOLVER9(R, name, sym, P1, P2, P3, P4, P5, P6, P7, P8, P9) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1, P2, P3, P4, P5, P6, P7, P8, P9), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9), (p1, p2, p3, p4, p5, p6, p7, p8, p9))
#define CAPI_DEFINE_RESOLVER10(R, name, sym, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10))
#define CAPI_DEFINE_RESOLVER11(R, name, sym, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11))
#define CAPI_DEFINE_RESOLVER12(R, name, sym, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12))
#define CAPI_DEFINE_RESOLVER13(R, name, sym, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12, P13 p13), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13))
// api with linkage modifier
#define CAPI_DEFINE_M_RESOLVER0(R, M, name, sym, ...) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (), (), ())
#define CAPI_DEFINE_M_RESOLVER1(R, M, name, sym, P1) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1), (P1 p1), (p1))
#define CAPI_DEFINE_M_RESOLVER2(R, M, name, sym, P1, P2) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1, P2), (P1 p1, P2 p2), (p1, p2))
#define CAPI_DEFINE_M_RESOLVER3(R, M, name, sym, P1, P2, P3) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1, P2, P3), (P1 p1, P2 p2, P3 p3), (p1, p2, p3))
#define CAPI_DEFINE_M_RESOLVER4(R, M, name, sym, P1, P2, P3, P4) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1, P2, P3, P4), (P1 p1, P2 p2, P3 p3, P4 p4), (p1, p2, p3, p4))
#define CAPI_DEFINE_M_RESOLVER5(R, M, name, sym, P1, P2, P3, P4, P5) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1, P2, P3, P4, P5), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5), (p1, p2, p3, p4, p5))
#define CAPI_DEFINE_M_RESOLVER6(R, M, name, sym, P1, P2, P3, P4, P5, P6) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1, P2, P3, P4, P5, P6), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6), (p1, p2, p3, p4, p5, p6))
#define CAPI_DEFINE_M_RESOLVER7(R, M, name, sym, P1, P2, P3, P4, P5, P6, P7) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1, P2, P3, P4, P5, P6, P7), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7), (p1, p2, p3, p4, p5, p6, p7))
#define CAPI_DEFINE_M_RESOLVER8(R, M, name, sym, P1, P2, P3, P4, P5, P6, P7, P8) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1, P2, P3, P4, P5, P6, P7, P8), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8), (p1, p2, p3, p4, p5, p6, p7, p8))
#define CAPI_DEFINE_M_RESOLVER9(R, M, name, sym, P1, P2, P3, P4, P5, P6, P7, P8, P9) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1, P2, P3, P4, P5, P6, P7, P8, P9), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9), (p1, p2, p3, p4, p5, p6, p7, p8, p9))
#define CAPI_DEFINE_M_RESOLVER10(R, M, name, sym, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10))
#define CAPI_DEFINE_M_RESOLVER11(R, M, name, sym, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11))
#define CAPI_DEFINE_M_RESOLVER12(R, M, name, sym, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12))
#define CAPI_DEFINE_M_RESOLVER13(R, M, name, sym, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12, P13 p13), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13))

//fully expand. used by VC. VC will not expand __VA_ARGS__ but treats it as 1 parameter
#define EXPAND(expr) expr

#endif // CAPI_H
