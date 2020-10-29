//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTLOGGER_H
#define MATERIALX_PVTLOGGER_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtLogger.h>

#include <bitset>

namespace MaterialX
{

class PvtLogger
{
public:
    PvtLogger(RtLogger* logger) :
        _logger(logger)
    {
        _enabled.set();
    }

    void enable(RtLogger::MessageType type, bool value) {
        size_t bit = static_cast<size_t>(type);
        if (value) {
            _enabled.set(bit);
        } else {
            _enabled.reset(bit);
        }
    }

    bool isEnabled(RtLogger::MessageType type) {
        size_t bit = static_cast<size_t>(type);
        return _enabled.test(bit);
    }

    void log(RtLogger::MessageType type, const string& msg) {
        if (isEnabled(type)) {
            _logger->logImpl(type, msg);
        }
    }

private:
    std::bitset<32> _enabled;
    RtLogger*       _logger;
};

}

#endif
