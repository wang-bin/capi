/******************************************************************************
    An example to show how to use CAPI
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

#ifndef ZLIB_API_H
#define ZLIB_API_H
// no need to include the C header if only functions declared there
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
#ifndef CAPI_LINK_ZLIB // avoid ambiguous in zlib_api.cpp
using namespace capi;
#endif
namespace capi { bool loaded();} // For link or NS style. Or load test for class style. api.loaded for class style.
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
#ifndef ZLIB_CAPI_BUILD
#ifdef ZLIB_CAPI_NS
using namespace zlib::capi;
#else
using namespace zlib;
#endif
#endif //ZLIB_CAPI_BUILD
#endif // ZLIB_API_H
