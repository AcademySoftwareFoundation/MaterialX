//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTLOGGER_H
#define MATERIALX_RTLOGGER_H

#include <MaterialXRuntime/Library.h>

/// @file RtLogger.h
/// Base class for logging messages. Derived classes can be used to route messages to various locations.

namespace MaterialX
{

using RtLoggerPtr = std::shared_ptr<class RtLogger>;

/// @class RtLogger
/// Base class for message logger.
class RtLogger
{
public:
    enum class MessageType
    {
        ERROR_MESSAGE = 0,
        WARNING_MESSAGE = 1,
        INFO_MESSAGE = 2
    };

    // Destructor.
    virtual ~RtLogger();

    /// Enable or disable logging of a given message type.
    void enable(MessageType type, bool value);

    /// Whether logging of a given message type is enabled.
    bool isEnabled(MessageType type);

    /// Logs a message of the provided type if the provided message type is enabled.
    void log(MessageType type, const string& msg);

protected:
    /// Logs a message of the provided type.
    virtual void logImpl(MessageType type, const string& msg) = 0;

    RtLogger();

    void* _ptr;
    friend class PvtLogger;
};

}

#endif
