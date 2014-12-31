/******************************************************************************
    An example to show how to use C API in C++ dynamically and no link
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

#include <QtCore/QLibrary>
#include <cassert>
// no need to include the C header if only functions declared there
extern "C" {
#include "zlib.h"
}
#ifndef Q_FUNC_INFO
#define Q_FUNC_INFO __FUNCTION__
#endif

/*
 * base ctor dll_helper("name")=>derived members in decl order(resolvers)=>derived ctor
 */
class dll_helper {
public:
    dll_helper(const QString& soname) {
        m_lib.setFileName(soname);
        if (m_lib.load())
            qDebug("%s loaded", m_lib.fileName().toUtf8().constData());
        else
            qDebug("can not load %s: %s", m_lib.fileName().toUtf8().constData(), m_lib.errorString().toUtf8().constData());
    }
    virtual ~dll_helper() { m_lib.unload();}
    bool isLoaded() const { return m_lib.isLoaded(); }
    void* resolve(const char *symbol) { return (void*)m_lib.resolve(symbol);}
private:
    QLibrary m_lib;
};

/*!
 * \brief The zlib_dll class
 * dll loader and symbol resolver.
 * It's designed to easily to use macros to simplify the code
 */
class zlib_dll : public dll_helper {
public:
    zlib_dll()
#ifdef Q_OS_WIN
        : dll_helper("zlib")
#else
        : dll_helper("z")
#endif
    { qDebug("dll symbols resolved...");}
    // can use 1 macro here for each zlib function
public:
    typedef const char* zlibVersion_t();
    zlibVersion_t* zlibVersion;
private:
    class zlibVersion_resolver_t {
    public:
        zlibVersion_resolver_t() {
            // nested class can not call non-static members outside the class, so hack the address here
            // need -Wno-invalid-offsetof
            const qptrdiff diff = qptrdiff(&((zlib_dll*)0)->zlibVersion_resolver) - qptrdiff(&((zlib_dll*)0)->zlibVersion);
            zlibVersion_t **p = (zlibVersion_t**)((quint8*)this - diff);
            zlib_dll* dll = (zlib_dll*)((quint8*)this - ((qptrdiff)(&((zlib_dll*)0)->zlibVersion_resolver)));
            if (!dll->isLoaded()) {
                qWarning("dll not loaded %s @%d", __FUNCTION__, __LINE__);
                *p = 0;
                return;
            }
            *p = (zlibVersion_t*)dll->resolve("zlibVersion");
            qDebug("zlib_dll::zlibVersion: %p", *p);
        }
    } zlibVersion_resolver;
public:
    typedef const char* zError_t(int);
    zError_t* zError;
private:
    class zError_resolver_t {
    public:
        zError_resolver_t() {
            const qptrdiff diff = qptrdiff(&((zlib_dll*)0)->zError_resolver) - qptrdiff(&((zlib_dll*)0)->zError);
            zError_t **p = (zError_t**)((quint8*)this - diff);
            zlib_dll* dll = (zlib_dll*)((quint8*)this - ((qptrdiff)(&((zlib_dll*)0)->zError_resolver)));
            if (!dll->isLoaded()) {
                qWarning("dll not loaded %s @%d", __FUNCTION__, __LINE__);
                *p = 0;
                return;
            }
            *p = (zError_t*)dll->resolve("zError");
            qDebug("zlib_dll::zError: %p", *p);
        }
    } zError_resolver;
};

class zlib_dll2 : public dll_helper {
public:
    zlib_dll2()
#ifdef Q_OS_WIN
        : dll_helper("zlib")
#else
        : dll_helper("z")
#endif
    {memset(&api, 0, sizeof(api));}
    typedef struct {
        typedef const char* zlibVersion_t();
        zlibVersion_t *zlibVersion;
        typedef const char* zError_t(int);
        zError_t *zError;
    } api_t;
    api_t api;
};
class zlib_api2 {
public:
    zlib_api2() : dll(new zlib_dll2()) {}
    const char* zlibVersion() {
        assert(dll);
        if (!dll->api.zlibVersion) {
            qDebug("resolving '%s' ...", __FUNCTION__);
            dll->api.zlibVersion = (zlib_dll2::api_t::zlibVersion_t*)dll->resolve("zlibVersion");
        }
        assert(dll->api.zlibVersion);
        return dll->api.zlibVersion();
    }
    const char* zError(int e) {
        assert(dll);
        if (!dll->api.zError) {
            qDebug("resolving '%s' ...", __FUNCTION__);
            dll->api.zError = (zlib_dll2::api_t::zError_t*)dll->resolve("zError");
        }
        assert(dll->api.zError);
        return dll->api.zError(e);
    }
private:
    zlib_dll2 *dll;
};

/*!
 * \brief The zlib_api class
 * Any class calling zlib functions inherits this. The dynamically loaded symbols will be called
 */
class zlib_api {
public:
    zlib_api() : dll(new zlib_dll()) {}
    const char* zlibVersion() {
        qDebug("%s @%d", Q_FUNC_INFO, __LINE__);
        assert(dll);
        return dll->zlibVersion();
    }
    const char* zError(int e) {
        qDebug("%s @%d", Q_FUNC_INFO, __LINE__);
        assert(dll);
        return dll->zError(e);
    }
private:
    zlib_dll *dll;
};

class test_zlib_api : public zlib_api2 {
public:
    void test_version() {
        qDebug("zlib version: %s", zlibVersion());
    }
    void test_error(int e) {
        qDebug("zlib error: %d %s", e, zError(e));
    }
};

int main(int argc, char *argv[])
{
    test_zlib_api tt;
    tt.test_version();
    tt.test_version();
    tt.test_version();
    tt.test_version();
    tt.test_error(1);
    return 0;
}

