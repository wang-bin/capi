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

#include "capi.h"

namespace capi {
namespace version {
const char* build() { return "(" __DATE__ ", " __TIME__ ")";}
} //namespace version

dll_helper::dll_helper(const char* names[])
{
    for (int i = 0; names[i]; ++i) {
        m_lib.setFileName(names[i]);
        if (!m_lib.load()) {
            qDebug() << "can not load [" << m_lib.fileName() << "], error: " << m_lib.errorString();
            continue;
        }
        qDebug() << " loaded" << m_lib.fileName();
        break;
    }
}

dll_helper::dll_helper(const char* names[], const int versions[])
{
    for (int i = 0; names[i]; ++i) {
        for (int j = 0; versions[j] != capi::EndVersion; ++j) {
            if (versions[j] == capi::NoVersion)
                m_lib.setFileName(names[i]);
            else
                m_lib.setFileNameAndVersion(names[i], versions[j]);
            if (!m_lib.load()) {
                qDebug() << "can not load [" << m_lib.fileName() << "], error: " << m_lib.errorString();
                continue;
            }
            qDebug() << " loaded" << m_lib.fileName();
            break;
        }
    }
}

} //namespace capi
