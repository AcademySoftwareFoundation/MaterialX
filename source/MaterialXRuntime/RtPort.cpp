//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtPort.h>

#include <MaterialXRuntime/Private/PvtPort.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(RtPort, RtObjType::PORT, "RtPort")

RtPort::RtPort(PvtObjHandle hnd) :
    RtObject(hnd)
{
}

const RtString& RtPort::getType() const
{
    return hnd()->asA<PvtPort>()->getType();
}

const RtValue& RtPort::getValue() const
{
    return hnd()->asA<PvtPort>()->getValue();
}

RtValue& RtPort::getValue()
{
    return hnd()->asA<PvtPort>()->getValue();
}

void RtPort::setValue(const RtValue& v)
{
    return hnd()->asA<PvtPort>()->setValue(v);
}

string RtPort::getValueString() const
{
    return hnd()->asA<PvtPort>()->getValueString();
}

void RtPort::setValueString(const string& v)
{
    hnd()->asA<PvtPort>()->setValueString(v);
}

const RtString& RtPort::getColorSpace() const
{
    return hnd()->asA<PvtPort>()->getColorSpace();
}

void RtPort::setColorSpace(const RtString& colorspace)
{
    return hnd()->asA<PvtPort>()->setColorSpace(colorspace);
}


RT_DEFINE_RUNTIME_OBJECT(RtInput, RtObjType::INPUT, "RtInput")

RtInput::RtInput(PvtObjHandle hnd) :
    RtPort(hnd)
{
}

bool RtInput::isUniform() const
{
    return hnd()->asA<PvtInput>()->isUniform();
}

bool RtInput::isToken() const
{
    return hnd()->asA<PvtInput>()->isToken();
}

bool RtInput::isUIVisible() const
{
    return hnd()->asA<PvtInput>()->isUIVisible();
}

void RtInput::setIsUIVisible(bool val)
{
    hnd()->asA<PvtInput>()->setIsUIVisible(val);
}

const RtString& RtInput::getUnit() const
{
    return hnd()->asA<PvtInput>()->getUnit();
}

void RtInput::setUnit(const RtString& unit)
{
    return hnd()->asA<PvtInput>()->setUnit(unit);
}

const RtString& RtInput::getUnitType() const
{
    return hnd()->asA<PvtInput>()->getUnitType();
}

void RtInput::setUnitType(const RtString& unit)
{
    return hnd()->asA<PvtInput>()->setUnitType(unit);
}

bool RtInput::isConnected() const
{
    return hnd()->asA<PvtInput>()->isConnected();
}

bool RtInput::isSocket() const
{
    return hnd()->asA<PvtInput>()->isSocket();
}

bool RtInput::isConnectable(const RtOutput& output) const
{
    return output.hnd()->asA<PvtOutput>()->isConnectable(hnd()->asA<PvtInput>());
}

void RtInput::connect(const RtOutput& output)
{
    output.hnd()->asA<PvtOutput>()->connect(hnd()->asA<PvtInput>());
}

void RtInput::disconnect(const RtOutput& output)
{
    output.hnd()->asA<PvtOutput>()->disconnect(hnd()->asA<PvtInput>());
}

void RtInput::clearConnection()
{
    return hnd()->asA<PvtInput>()->clearConnection();
}

RtOutput RtInput::getConnection() const
{
    PvtOutput* output = hnd()->asA<PvtInput>()->getConnection();
    return output ? RtOutput(output->hnd()) : RtOutput();
}


RT_DEFINE_RUNTIME_OBJECT(RtOutput, RtObjType::OUTPUT, "RtOutput")

RtOutput::RtOutput(PvtObjHandle hnd) :
    RtPort(hnd)
{
}

bool RtOutput::isConnected() const
{
    return hnd()->asA<PvtOutput>()->isConnected();
}

bool RtOutput::isSocket() const
{
    return hnd()->asA<PvtOutput>()->isSocket();
}

bool RtOutput::isConnectable(const RtInput& input) const
{
    return hnd()->asA<PvtOutput>()->isConnectable(input.hnd()->asA<PvtInput>());
}

void RtOutput::connect(const RtInput& input)
{
    return hnd()->asA<PvtOutput>()->connect(input.hnd()->asA<PvtInput>());
}

void RtOutput::disconnect(const RtInput& input)
{
    return hnd()->asA<PvtOutput>()->disconnect(input.hnd()->asA<PvtInput>());
}

void RtOutput::clearConnections()
{
    return hnd()->asA<PvtOutput>()->clearConnections();
}

RtConnectionIterator RtOutput::getConnections() const
{
    return hnd()->asA<PvtOutput>()->getConnections();
}

}
