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

// examples/mdl_sdk/shared/example_shared.h
//
// Code shared by all examples

#ifndef EXAMPLE_SHARED_H
#define EXAMPLE_SHARED_H

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <tuple>
#include <vector>
#include <cassert>

#include <mi/mdl_sdk.h>
#include "io.h"
#include "mdl.h"
#include "os.h"
#include "strings.h"
#include "log.h"

// ------------------------------------------------------------------------------------------------

// wrap the example entry point in order to support UTF8 arguments
#ifdef MI_PLATFORM_WINDOWS

    #define MAIN_UTF8 main_utf8
    #define COMMANDLINE_TO_UTF8 \
        int wmain(int argc, wchar_t* argv[]) { \
            char** argv_utf8 = new char*[argc]; \
            for (int i = 0; i < argc; i++) { \
                LPWSTR warg = argv[i]; \
                DWORD size = WideCharToMultiByte(CP_UTF8, 0, warg, -1, NULL, 0, NULL, NULL); \
                assert(size > 0); \
                if (size == 0)                                                                            \
                {                                                                                         \
                    fprintf(stderr, "Failed to convert command line argument %d to UTF-8.\n", i);         \
                    return -1;                                                                            \
                } \
                argv_utf8[i] = new char[size]; \
                DWORD result = WideCharToMultiByte(CP_UTF8, 0, warg, -1, argv_utf8[i], size, NULL, NULL); \
                assert(result > 0); \
                if (result == 0)                                                                          \
                {                                                                                         \
                    fprintf(stderr, "Failed to convert command line argument %d to UTF-8.\n", i);         \
                    return -1;                                                                            \
                } \
            } \
            SetConsoleOutputCP(CP_UTF8); \
            int result = main_utf8(argc, argv_utf8); \
            delete[] argv_utf8; \
            return result; \
        }

#else // MI_PLATFORM_WINDOWS

    #define MAIN_UTF8 main
    #define COMMANDLINE_TO_UTF8

#endif // MI_PLATFORM_WINDOWS

#endif // MI_EXAMPLE_SHARED_H
