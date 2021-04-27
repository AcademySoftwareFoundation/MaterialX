//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtPath.h>

#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXCore/Util.h>

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(PvtPrim, RtObjType::PRIM, "PvtPrim")

PvtPrim::PvtPrim(const RtTypeInfo* typeInfo, const RtIdentifier& name, PvtPrim* parent) :
    PvtObject(name, parent),
    _typeInfo(typeInfo)
{
    setTypeBit<PvtPrim>();
}

void PvtPrim::dispose(bool state)
{
    for (PvtObject* obj : _rel.vec())
    {
        obj->setDisposed(state);
    }
    for (PvtObject* obj : _inputs.vec())
    {
        obj->setDisposed(state);
    }
    for (PvtObject* obj : _outputs.vec())
    {
        obj->setDisposed(state);
    }
    for (PvtObject* obj : _prims.vec())
    {
        obj->asA<PvtPrim>()->dispose(state);
    }
    setDisposed(state);
}

void PvtPrim::destroy()
{
    // Disconnect and delete all relationships.
    for (PvtObject* obj : _rel.vec())
    {
        obj->asA<PvtRelationship>()->clearConnections();
    }
    _rel.clear();

    // Disconnect and delete all inputs.
    for (PvtObject* obj : _inputs.vec())
    {
        obj->asA<PvtInput>()->clearConnection();
    }
    _inputs.clear();

    // Disconnect and delete all outputs.
    for (PvtObject* obj : _outputs.vec())
    {
        obj->asA<PvtOutput>()->clearConnections();
    }
    _outputs.clear();

    // Destroy all child prims reqursively.
    for (PvtObject* obj : _prims.vec())
    {
        obj->asA<PvtPrim>()->destroy();
    }
    _prims.clear();

    // Tag as disposed.
    dispose(true);
}

PvtRelationship* PvtPrim::createRelationship(const RtIdentifier& name)
{
    if (getRelationship(name))
    {
        throw ExceptionRuntimeError("A relationship named '" + name.str() + "' already exists in prim '" + getName().str() + "'");
    }

    PvtRelationship* rel = new PvtRelationship(name, this);
    _rel.add(rel);

    return rel;
}

void PvtPrim::removeRelationship(const RtIdentifier& name)
{
    PvtRelationship* rel = getRelationship(name);
    if (rel)
    {
        rel->setDisposed(true);
        _rel.remove(name);
    }
}

PvtInput* PvtPrim::createInput(const RtIdentifier& name, const RtIdentifier& type, uint32_t flags)
{
    // Tokens as well as inputs with type filename, identifier or string must always be uniform.
    if (((flags & RtPortFlag::TOKEN) != 0) || type == RtType::FILENAME || type == RtType::IDENTIFIER || type == RtType::STRING)
    {
        flags |= RtPortFlag::UNIFORM;
    }

    RtIdentifier uniqueName = makeUniqueChildName(name);
    PvtInput* port = new PvtInput(uniqueName, type, flags, this);
    _inputs.add(port);

    return port;
}

void PvtPrim::removeInput(const RtIdentifier& name)
{
    PvtPort* port = getInput(name);
    if (!port)
    {
        throw ExceptionRuntimeError("No input found with name '" + name.str() + "'");
    }
    port->setDisposed(true);
    _inputs.remove(name);
}

PvtOutput* PvtPrim::createOutput(const RtIdentifier& name, const RtIdentifier& type, uint32_t flags)
{
    RtIdentifier uniqueName = makeUniqueChildName(name);

    PvtOutput* port = new PvtOutput(uniqueName, type, flags, this);
    _outputs.add(port);

    return port;
}

void PvtPrim::removeOutput(const RtIdentifier& name)
{
    PvtPort* port = getOutput(name);
    if (!port)
    {
        throw ExceptionRuntimeError("No output found with name '" + name.str() + "'");
    }
    port->setDisposed(true);
    _outputs.remove(name);
}

RtPrimIterator PvtPrim::getChildren(RtObjectPredicate predicate) const
{
    return RtPrimIterator(hnd(), predicate);
}

RtIdentifier PvtPrim::makeUniqueChildName(const RtIdentifier& name) const
{
    RtIdentifier newName = name;

    // Check if there is another child with this name.
    // We must check both prims, inputs and outputs since in
    // MaterialX core these all stored in the same map and 
    // cannot have name conflicts among them.
    if (_prims.count(newName) || _inputs.count(newName) || _outputs.count(newName))
    {
        // Find a number to append to the name, incrementing
        // the counter until a unique name is found.
        string baseName = name.str();
        int i = 1;
        const size_t n = name.str().find_last_not_of("0123456789") + 1;
        if (n < name.str().size())
        {
            const string number = name.str().substr(n);
            i = std::stoi(number) + 1;
            baseName = baseName.substr(0, n);
        }
        // Iterate until there is no other child with the resulting name.
        do {
            newName = RtIdentifier(baseName + std::to_string(i++));
        } while (_prims.count(newName) || _inputs.count(newName) || _outputs.count(newName));
    }

    return newName;
}

void PvtPrim::addChildPrim(PvtPrim* prim)
{
    _prims.add(prim);
}

void PvtPrim::removeChildPrim(PvtPrim* prim)
{
    _prims.remove(prim->getName());
}


const RtAttributeSpec* PvtPrimSpec::getPortAttribute(const RtPort& port, const RtIdentifier& name) const
{
    const bool input = port.isA<RtInput>();

    // First search the input/output lists by port name.
    const RtIdentifierMap<RtAttributeSpecList>& mapByName = (input ? _inputAttrByName : _outputAttrByName);
    auto it = mapByName.find(port.getName());
    if (it != mapByName.end())
    {
        const RtAttributeSpec* spec = it->second.find(name);
        if (spec)
        {
            return spec;
        }
    }

    // Second search the input/output lists by port type.
    const RtIdentifierMap<RtAttributeSpecList>& mapByType = (input ? _inputAttrByType : _outputAttrByType);
    it = mapByType.find(port.getType());
    if (it != mapByType.end())
    {
        const RtAttributeSpec* spec = it->second.find(name);
        if (spec)
        {
            return spec;
        }
    }

    // Finally search the general input/output list.
    const RtAttributeSpecList& list = (input ? _inputAttr : _outputAttr);
    return list.find(name);
}

RtAttributeSpecVec PvtPrimSpec::getPortAttributes(const RtPort& port) const
{
    RtAttributeSpecVec result;

    const bool input = port.isA<RtInput>();

    // First search the input/output lists by port name.
    const RtIdentifierMap<RtAttributeSpecList>& mapByName = (input ? _inputAttrByName : _outputAttrByName);
    auto it = mapByName.find(port.getName());
    if (it != mapByName.end())
    {
        result.insert(result.begin(), it->second._vec.begin(), it->second._vec.end());
    }

    // Second search the input/output lists by port type.
    const RtIdentifierMap<RtAttributeSpecList>& mapByType = (input ? _inputAttrByType : _outputAttrByType);
    it = mapByType.find(port.getType());
    if (it != mapByType.end())
    {
        result.insert(result.begin(), it->second._vec.begin(), it->second._vec.end());
    }

    // Finally search the general input/output list.
    const RtAttributeSpecList& list = (input ? _inputAttr : _outputAttr);
    result.insert(result.begin(), list._vec.begin(), list._vec.end());

    return result;
}

RtAttributeSpec* PvtPrimSpec::create(const RtIdentifier& name, const RtIdentifier& type, const string& value,
                                     bool exportable, bool custom)
{
    RtAttributeSpec* spec = new RtAttributeSpec();
    PvtAttributeSpec* ptr = static_cast<PvtAttributeSpec*>(spec->_ptr);
    ptr->name = name;
    ptr->type = type;
    ptr->value = value;
    ptr->exportable = exportable;
    ptr->custom = custom;
    return spec;
}

}
