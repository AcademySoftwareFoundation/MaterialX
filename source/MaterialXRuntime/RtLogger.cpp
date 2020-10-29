//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtLogger.h>

#include <MaterialXRuntime/Private/PvtLogger.h>

namespace MaterialX {

namespace
{
    inline PvtLogger* _cast(void* ptr)
    {
        return static_cast<PvtLogger*>(ptr);
    }
}

RtLogger::RtLogger() :
    _ptr(new PvtLogger(this))
{
}

RtLogger::~RtLogger()
{
    delete _cast(_ptr);
}

void RtLogger::enable(MessageType type, bool value) {
    _cast(_ptr)->enable(type, value);
}

bool RtLogger::isEnabled(MessageType type) {
    return _cast(_ptr)->isEnabled(type);
}

void RtLogger::log(MessageType type, const string& msg) {
    _cast(_ptr)->log(type, msg);
}

}

