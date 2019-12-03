//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvNodeDef.h>
#include <MaterialXRuntime/Private/PrvPortDef.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

PrvNodeDef::PrvNodeDef(const RtToken& name, const RtToken& nodeName) :
    PrvAllocatingElement(RtObjType::NODEDEF, name),
    _nodeName(nodeName),
    _numOutputs(0)
{
}

PrvObjectHandle PrvNodeDef::createNew(PrvElement* parent, const RtToken& name, const RtToken& category)
{
    if (parent && !parent->hasApi(RtApiType::STAGE))
    {
        throw ExceptionRuntimeError("Given parent object is not a stage");
    }

    PrvObjectHandle nodedef(new PrvNodeDef(name, category));
    if (parent)
    {
        parent->addChild(nodedef);
    }

    return nodedef;
}

void PrvNodeDef::addPort(PrvObjectHandle portdef)
{
    if (!portdef->hasApi(RtApiType::PORTDEF))
    {
        throw ExceptionRuntimeError("Given object is not a valid portdef");
    }

    PrvPortDef* p = portdef->asA<PrvPortDef>();
    if (_childrenByName.count(p->getName()))
    {
        throw ExceptionRuntimeError("A port named '" + p->getName().str() + "' already exists for nodedef '" + getName().str() + "'");
    }
    if (p->getParent())
    {
        throw ExceptionRuntimeError("Port '" + p->getName().str() + "' already has a parent");
    }

    // We want to preserve the ordering of having all outputs stored before any inputs.
    // So if inputs are already stored we need to handled inserting the new output in
    // the right place.
    if (p->isOutput() && numPorts() > numOutputs())
    {
        // Insert the new output after the last output.
        auto it = _children.begin() + numOutputs();
        _children.insert(it, 1, portdef);
    }
    else
    {
        _children.push_back(portdef);
    }

    p->setParent(this);
    _childrenByName[p->getName()] = portdef;
    _numOutputs += p->isOutput();
    rebuildPortIndex();
}

void PrvNodeDef::removePort(const RtToken& name)
{
    PrvPortDef* p = findPort(name);
    if (p)
    {
        _numOutputs -= p->isOutput();
        removeChild(name);
        rebuildPortIndex();
    }
}

void PrvNodeDef::rebuildPortIndex()
{
    for (size_t i = 0; i < numPorts(); ++i)
    {
        _portIndex[getPort(i)->getName()] = i;
    }
}

}
