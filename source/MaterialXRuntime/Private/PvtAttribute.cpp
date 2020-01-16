//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtAttribute.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

const RtObjType PvtAttribute::typeId = RtObjType::ATTRIBUTE;
const RtToken PvtAttribute::typeName = "attribute";

const RtToken PvtAttribute::DEFAULT_OUTPUT_NAME("out");
const RtToken PvtAttribute::COLOR_SPACE("colorspace");
const RtToken PvtAttribute::UNIT("unit");

PvtAttribute::PvtAttribute(const RtToken& name, const RtToken& type, uint32_t flags, PvtPrim* parent) :
    PvtPathItem(name, parent),
    _value(type, RtValue::createNew(type, parent->obj())),
    _flags(flags)
{
}

bool PvtAttribute::isConnectable(const PvtAttribute* source) const
{
    // TODO: Check if the data types are compatible.
    return isConnectable() && source->isConnectable() &&
           isInput() && source->isOutput();
}

void PvtAttribute::connect(PvtAttribute* source, PvtAttribute* dest)
{
    if (dest->isConnected())
    {
        throw ExceptionRuntimeError("Destination is already connected");
    }
    if (!(source->isOutput() && source->isConnectable()))
    {
        throw ExceptionRuntimeError("Source is not a connectable output");
    }
    if (!(dest->isInput() && dest->isConnectable()))
    {
        throw ExceptionRuntimeError("Destination is not a connectable input");
    }
    source->_connections.push_back(dest->hnd());
    dest->_connections.push_back(source->hnd());
}

void PvtAttribute::disconnect(PvtAttribute* source, PvtAttribute* dest)
{
    if (dest->_connections.size() != 1 || dest->_connections.front().get() != source)
    {
        throw ExceptionRuntimeError("Given source and destination is not connected");
    }

    dest->_connections.clear();
    for (auto it = source->_connections.begin(); it != source->_connections.end(); ++it)
    {
        if (it->get() == dest)
        {
            source->_connections.erase(it);
            break;
        }
    }
}

void PvtAttribute::clearConnections()
{
    if (isOutput())
    {
        // Break connections to all destination inputs.
        for (PvtDataHandle destH : _connections)
        {
            destH->asA<PvtAttribute>()->_connections.clear();
        }
    }
    else if (isConnected())
    {
        // Break connection to our single source output.
        PvtAttribute* source = _connections.front()->asA<PvtAttribute>();
        for (auto it = source->_connections.begin(); it != source->_connections.end(); ++it)
        {
            if (it->get() == this)
            {
                source->_connections.erase(it);
                break;
            }
        }
    }
    _connections.clear();
}

}
