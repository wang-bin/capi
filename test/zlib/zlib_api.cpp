/******************************************************************************
    An example to show how to use CAPI
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
#define DEBUG

#include "zlib_api.h"
#include "capi.h"

namespace zlib {
static const char* zlib[] = {
#ifdef __WIN32
    "zlib",
#else
    "z",
#endif
    NULL
};

CAPI_BEGIN_DLL(zlib)
CAPI_DEFINE_RESOLVER(0, const char*, zlibVersion)
CAPI_DEFINE_RESOLVER(0, uLong, zlibCompileFlags)
CAPI_END_DLL()

api::api() : dll(new api_dll()) {
    qDebug("capi::version: %s build %s", capi::version::name, capi::version::build());
}
api::~api() { delete dll;}
bool api::loaded() const { return dll->isLoaded();}

CAPI_DEFINE(0, const char*, zlibVersion)
CAPI_DEFINE(0, uLong, zlibCompileFlags)

} //namespace zlib
