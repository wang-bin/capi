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
//#define ZLIB_CAPI_NS
//#define CAPI_LINK_ZLIB // add -lz is required
#include <stdio.h>
#include "zlib_api.h"

class test_zlib_api
#ifndef ZLIB_CAPI_NS
        : protected zlib::api // will unload library in dtor
#endif
{
public:
    void test_version() {
        printf("loaded: %d\n", zlib::capi::loaded());
        printf("%d, zlib version: %s\n", zlib::capi::loaded(), zlibVersion());
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

