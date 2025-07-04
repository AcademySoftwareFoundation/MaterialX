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

#include <chrono>
#include <functional>
#include <iostream>

#include <mi/mdl_sdk.h>

#include "strings.h"

namespace mi { namespace examples { namespace log
{

enum class Level : char
{
    Error,
    Warning,
    Info,
    Verbose,
    Debug
};

struct LogFile
{
    LogFile(const std::string& filepath) :
        m_log_file(filepath, std::ios_base::out),
        m_log_stream(m_log_file)
    {
    }

    virtual ~LogFile()
    {
        m_log_file.close();
    }

    std::ostream& stream()
    {
        return m_log_stream;
    }

    private:
    std::ofstream m_log_file;
    std::ostream& m_log_stream;
};


// current log levels need to be defined in the one link unit (one cpp-file)
extern Level g_level_console;
extern Level g_level_file;

// log file needs to be specified in one link unit (one cpp-file). Can be nullptr.
extern LogFile* g_file;

// ------------------------------------------------------------------------------------------------

inline void print(
    Level level,
    const std::string& message)
{

    bool log_to_console = static_cast<char>(g_level_console) >= static_cast<char>(level);
    bool log_to_file = g_file && static_cast<char>(g_level_file) >= static_cast<char>(level);

    if (!log_to_console && !log_to_file)
        return; // early out if the message is not written.

    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::vector<char> date(12);
    size_t num = std::strftime(date.data(), date.size(), "[%H:%M:%S] ", std::localtime(&now));
    std::string m(num > 0 ? date.data() : "");
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
        case Level::Debug:
            m.append("[DEBUG]   ");
            break;
        default:
            break;
    }
    m.append(message);

    // print the message to the console depending on the log level
    if (log_to_console)
    {
        std::cerr << m.c_str() << std::endl;
    }
    // print the message to the log file depending on the log level
    if (log_to_file)
    {
        g_file->stream() << m.c_str() << std::endl;
    }
}

// ------------------------------------------------------------------------------------------------

inline void debug(const std::string& message)
{
    print(Level::Debug, message);
}

template <typename... Args>
void debug(const char* format_string, Args&&... args)
{
    print(Level::Debug, mi::examples::strings::format(format_string, std::forward<Args>(args)...));
}

// ------------------------------------------------------------------------------------------------

inline void verbose(const std::string& message)
{
    print(Level::Verbose, message);
}

template <typename... Args>
void verbose(const char* format_string, Args&&... args)
{
    print(Level::Verbose, mi::examples::strings::format(format_string, std::forward<Args>(args)...));
}

// ------------------------------------------------------------------------------------------------

inline void info(const std::string& message)
{
    print(Level::Info, message);
}

template <typename... Args>
void info(const char* format_string, Args&&... args)
{
    print(Level::Info, mi::examples::strings::format(format_string, std::forward<Args>(args)...));
}

// ------------------------------------------------------------------------------------------------

inline void warning(const std::string& message)
{
    print(Level::Warning, message);
}

template <typename... Args>
void warning(const char* format_string, Args&&... args)
{
    print(Level::Warning, mi::examples::strings::format(format_string, std::forward<Args>(args)...));
}

// ------------------------------------------------------------------------------------------------

inline void error(const std::string& message)
{
    print(Level::Error, message);
}

template <typename... Args>
void error(const char* format_string, Args&&... args)
{
    print(Level::Error, mi::examples::strings::format(format_string, std::forward<Args>(args)...));
}

// ------------------------------------------------------------------------------------------------

inline void exception(
    const std::string& message,
    const std::exception& exception)
{
    std::function<std::string(const std::exception&)> print_nested_exception = [&](const std::exception& e)
    {
        std::string what = e.what();
        try {
            std::rethrow_if_nested(e);
        }
        catch (const std::exception& nested) {
            what += "\n  nested: " + print_nested_exception(nested);
        }
        return what;
    };
    error(message + " " + print_nested_exception(exception));
}

// ------------------------------------------------------------------------------------------------

inline void print_context_messages(const mi::neuraylib::IMdl_execution_context* context)
{
    if (context->get_messages_count() == 0)
        return;

    for (mi::Size i = 0, n = context->get_messages_count(); i < n; ++i)
    {
        mi::base::Handle<const mi::neuraylib::IMessage> message(context->get_message(i));
        switch (message->get_severity())
        {
            case mi::base::MESSAGE_SEVERITY_FATAL:
                error(message->get_string());
                break;
            case mi::base::MESSAGE_SEVERITY_ERROR:
                error(message->get_string());
                break;
            case mi::base::MESSAGE_SEVERITY_WARNING:
                warning(message->get_string());
                break;
            case mi::base::MESSAGE_SEVERITY_INFO:
                info(message->get_string());
                break;
            case mi::base::MESSAGE_SEVERITY_VERBOSE:
                verbose(message->get_string());
                break;
            case mi::base::MESSAGE_SEVERITY_DEBUG:
                debug(message->get_string());
                break;
            case mi::base::MESSAGE_SEVERITY_FORCE_32_BIT:
                break;
        }
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
            case mi::base::MESSAGE_SEVERITY_VERBOSE:
                verbose(message + 7 + 3); // "verbose : "
                break;
            case mi::base::MESSAGE_SEVERITY_DEBUG:
                debug(message + 5 + 3); // "debug : "
                break;
            case mi::base::MESSAGE_SEVERITY_FORCE_32_BIT:
                break;
        }
    }

};

}}}
#endif
