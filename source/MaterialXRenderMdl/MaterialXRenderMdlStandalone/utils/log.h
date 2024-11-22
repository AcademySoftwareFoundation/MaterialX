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

 // examples/mdl_sdk/shared/utils/log.h
 //
 // Code shared by all examples

#ifndef EXAMPLE_SHARED_UTILS_LOG_H
#define EXAMPLE_SHARED_UTILS_LOG_H

#include "strings.h"

#include <mi/base/config.h>
#include <chrono>
#include <functional>
#include <iostream>


namespace mi { namespace examples { namespace log
{
    enum class Level : char
    {
        None = 0,
        Error,
        Warning,
        Info,
        Verbose
    };

    // current log level needs to be defined in the one link unit (one cpp-file)
    extern Level s_Level;

// ------------------------------------------------------------------------------------------------

inline void print(
    Level level,
    const std::string& message)
{
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char text[12] = { '\0' };
    auto res = std::strftime(text, sizeof(text), "[%H:%M:%S] ", std::localtime(&now));

    std::string m(text);
    switch(level)
    {
        case Level::Error:
            m.append("[ERROR]   ");
            break;
        case Level::Warning:
            m.append("[WARNING] ");
            break;
        case Level::Info:
            m.append("[INFO]    ");
            break;
        case Level::Verbose:
            m.append("[VERBOSE] ");
            break;
        default:
            return;
    }
    m.append(message);
    std::cerr << m.c_str() << std::endl;
}

// ------------------------------------------------------------------------------------------------

inline void verbose(const std::string& message)
{
    if (static_cast<char>(s_Level) >= static_cast<char>(Level::Verbose))
        print(Level::Verbose, message);
}

template <typename... Args>
void verbose(const char* format_string, Args... args)
{
    if (static_cast<char>(s_Level) >= static_cast<char>(Level::Verbose))
        print(Level::Verbose, mi::examples::strings::format(format_string, std::forward<Args>(args)...));
}

// ------------------------------------------------------------------------------------------------

inline void info(const std::string& message)
{
    if (static_cast<char>(s_Level) >= static_cast<char>(Level::Info))
        print(Level::Info, message);
}

template <typename... Args>
void info(const char* format_string, Args... args)
{
    if (static_cast<char>(s_Level) >= static_cast<char>(Level::Info))
        print(Level::Info, mi::examples::strings::format(format_string, std::forward<Args>(args)...));
}

// ------------------------------------------------------------------------------------------------

inline void warning(const std::string& message)
{
    if (static_cast<char>(s_Level) >= static_cast<char>(Level::Warning))
        print(Level::Warning, message);
}

template <typename... Args>
void warning(const char* format_string, Args... args)
{
    if (static_cast<char>(s_Level) >= static_cast<char>(Level::Warning))
        print(Level::Warning, mi::examples::strings::format(format_string, std::forward<Args>(args)...));
}

// ------------------------------------------------------------------------------------------------

inline void error(const std::string& message)
{
    if (static_cast<char>(s_Level) >= static_cast<char>(Level::Error))
        print(Level::Error, message);
}

template <typename... Args>
void error(const char* format_string, Args... args)
{
    if (static_cast<char>(s_Level) >= static_cast<char>(Level::Error))
        print(Level::Error, mi::examples::strings::format(format_string, std::forward<Args>(args)...));
}

// ------------------------------------------------------------------------------------------------

inline void exception(
    const std::string& message,
    const std::exception& exception)
{
    std::function<std::string(const std::exception&)> print_nested_exception = [&](const std::exception& e)
    {
        std::string message = e.what();
        try {
            std::rethrow_if_nested(e);
        }
        catch (const std::exception& nested) {
            message += "\n  nested: " + print_nested_exception(nested);
        }
        return message;
    };
    error(message + " " + print_nested_exception(exception));
}

// ------------------------------------------------------------------------------------------------

inline void context_messages(const mi::neuraylib::IMdl_execution_context* context)
{
    if (context->get_messages_count() == 0)
        return;

    std::string log;
    mi::base::Message_severity most_severe = mi::base::MESSAGE_SEVERITY_DEBUG;
    for (mi::Size i = 0, n = context->get_messages_count(); i < n; ++i)
    {
        log += "\n";
        mi::base::Handle<const mi::neuraylib::IMessage> message(context->get_message(i));
        switch (message->get_severity())
        {
            case mi::base::MESSAGE_SEVERITY_FATAL:
                log += "  fatal:   ";
                break;
            case mi::base::MESSAGE_SEVERITY_ERROR:
                log += "  error:   ";
                break;
            case mi::base::MESSAGE_SEVERITY_WARNING:
                log += "  warning: ";
                break;
            case mi::base::MESSAGE_SEVERITY_INFO:
                log += "  info:    ";
                break;
            case mi::base::MESSAGE_SEVERITY_VERBOSE:
                log += "  verbose: ";
                break;
            case mi::base::MESSAGE_SEVERITY_DEBUG:
                log += "  debug:   ";
                break;
            default:
                log += "           ";
                break;
        }
        most_severe = std::min(most_severe, message->get_severity());
        const std::string kind = std::to_string(message->get_kind());
        if (!kind.empty())
            log += kind + ": ";
        log += message->get_string();
    }

    switch (most_severe)
    {
        case mi::base::MESSAGE_SEVERITY_FATAL:
        case mi::base::MESSAGE_SEVERITY_ERROR:
            error(log);
            break;
        case mi::base::MESSAGE_SEVERITY_WARNING:
            warning(log);
            break;
        case mi::base::MESSAGE_SEVERITY_INFO:
            info(log);
            break;
        case mi::base::MESSAGE_SEVERITY_VERBOSE:
        case mi::base::MESSAGE_SEVERITY_DEBUG:
        default:
            verbose(log);
            break;
    }
}

// ------------------------------------------------------------------------------------------------

class ExampleLogger : public mi::base::Interface_implement<mi::base::ILogger>
{
  public:
    /// Emits a message to the application's log.
    void message(mi::base::Message_severity level,
                 const char* /*module_category*/,
                 const mi::base::Message_details& /*details*/,
                 const char* message) 
    { 
        switch (level)
        {
            case mi::base::MESSAGE_SEVERITY_FATAL:
                error(message + 5 + 3); // "fatal : "
                break;
            case mi::base::MESSAGE_SEVERITY_ERROR:
                error(message + 5 + 3); // "error : "
                break;
            case mi::base::MESSAGE_SEVERITY_WARNING:
                warning(message + 4 + 3); // "warn : "
                break;
            case mi::base::MESSAGE_SEVERITY_INFO:
                info(message + 4 + 3); // "info : "
                break;
            case mi::base::MESSAGE_SEVERITY_DEBUG:
                verbose(message + 5 + 3); // "debug : "
                break;
            case mi::base::MESSAGE_SEVERITY_VERBOSE:
                error(message);
                break;
            default:
                verbose(message);
                break;
        }
    }

};

}}}
#endif
