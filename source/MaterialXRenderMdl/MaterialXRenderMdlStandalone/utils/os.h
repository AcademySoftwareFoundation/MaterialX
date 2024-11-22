/******************************************************************************
 * Copyright (c) 2021-2024, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

 // examples/mdl_sdk/shared/utils/os.h
 //
 // Code shared by all examples

#ifndef EXAMPLE_SHARED_UTILS_OS_H
#define EXAMPLE_SHARED_UTILS_OS_H

#include <fstream>
#include <algorithm>
#include <sstream>

#include <mi/base/config.h>
#include "strings.h"

#ifdef MI_PLATFORM_WINDOWS
    #include <windows.h>
    #include <commdlg.h>
    #include <direct.h>
    #include <Shlobj.h>
#else
    #include <dlfcn.h>
    #include <unistd.h>
    #include <dirent.h>
#endif

namespace mi { namespace examples { namespace os
{
    // Returns the value of the given environment variable.
    //
    // \param env_var   environment variable name
    // \return          the value of the environment variable or an empty string
    //                  if that variable does not exist or does not have a value.
    inline std::string get_environment(const char* env_var)
    {
        std::string value = "";
        #ifdef MI_PLATFORM_WINDOWS
            char* buf = nullptr;
            size_t sz = 0;
            if (_dupenv_s(&buf, &sz, env_var) == 0 && buf != nullptr) {
                value = buf;
                free(buf);
            }
        #else
            const char* v = getenv(env_var);
            if (v)
                value = v;
        #endif
            return value;
    }

    //---------------------------------------------------------------------------------------------

    // Sets the value of the given environment variable.
    //
    // \param env_var   environment variable name
    // \param value     the new value to set.
    inline bool set_environment(const char* env_var, const char* value)
    {
        #ifdef MI_PLATFORM_WINDOWS
            return 0 == _putenv_s(env_var, value);
        #else
            return 0 == setenv(env_var, value, 1);
        #endif
    }

}}}
#endif
