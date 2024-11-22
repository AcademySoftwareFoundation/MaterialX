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

#include <mi/mdl_sdk.h>
#include "io.h"
#include "mdl.h"
#include "os.h"
#include "strings.h"
#include "log.h"


/// called to abort the execution of an example in case of failure.
/// \param  file        current file determined using the `__FILE__` macro
/// \param  line        current line in file determined using the `__LINE__` macro
/// \param  message     description of the error that caused the failure
inline void exit_failure_(
    const char* file, int line,
    std::string message)
{
    // print message
    if (message.empty())
        fprintf(stderr, "Fatal error in file: %s line: %d\n\nClosing the example.\n", file, line);
    else
        fprintf(stderr, "Fatal error in file: %s line: %d\n  %s\n\nClosing the example.\n", 
            file, line, message.c_str());

    // kill the application
    exit(EXIT_FAILURE);
}

/// called to abort the execution of an example in case of failure.
/// \param  file        current file determined using the `__FILE__` macro
/// \param  line        current line in file determined using the `__LINE__` macro
inline void exit_failure_(const char* file, int line)
{
    exit_failure_(file, line, "");
}
#define exit_failure(...) \
    exit_failure_(__FILE__, __LINE__, mi::examples::strings::format(__VA_ARGS__))


// ------------------------------------------------------------------------------------------------

/// called to end execution of an example in case of success.
/// use like this: 'return exit_success()'
inline void exit_success_()
{
    exit(EXIT_SUCCESS);
}

#define exit_success() exit_success_(); return EXIT_SUCCESS; // no warning about missing return

// ------------------------------------------------------------------------------------------------

// Helper macro. Checks whether the expression is true and if not prints a message and exits.
#define check_success( expr) \
    do { \
        if( !(expr)) \
            exit_failure( "%s", #expr); \
    } while( false)

/// Prints a message.
inline void print_message(
    mi::base::details::Message_severity severity,
    mi::neuraylib::IMessage::Kind kind,
    const char* msg)
{
    std::string s_kind = mi::examples::strings::to_string(kind);
    std::string s_severity = mi::examples::strings::to_string(severity);
    fprintf(stderr, "%s: %s %s\n", s_severity.c_str(), s_kind.c_str(), msg);
}

/// Prints the messages of the given context.
/// Returns true, if the context does not contain any error messages, false otherwise.
inline bool print_messages(mi::neuraylib::IMdl_execution_context* context)
{
    for (mi::Size i = 0, n = context->get_messages_count(); i < n; ++i) {
        mi::base::Handle<const mi::neuraylib::IMessage> message(context->get_message(i));
        print_message(message->get_severity(), message->get_kind(), message->get_string());
    }
    return context->get_error_messages_count() == 0;
}

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
                check_success(size > 0); \
                argv_utf8[i] = new char[size]; \
                DWORD result = WideCharToMultiByte(CP_UTF8, 0, warg, -1, argv_utf8[i], size, NULL, NULL); \
                check_success(result > 0); \
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
