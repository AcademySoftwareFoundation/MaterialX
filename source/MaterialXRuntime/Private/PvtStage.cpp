//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtStage.h>
#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtNode.h>
#include <MaterialXRuntime/Private/PvtNodeGraph.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

const RtObjType PvtStage::_typeId = RtObjType::STAGE;
const RtToken PvtStage::_typeName = RtToken("stage");

PvtStage::PvtStage(const RtToken& name) :
    _name(name),
    _root(nullptr),
    _selfRefCount(0)
{
    _root = PvtDataHandle(new RootPrim(this));
}

PvtDataHandle PvtStage::createNew(const RtToken& name)
{
    return PvtDataHandle(new PvtStage(name));
}

PvtPrim* PvtStage::createPrim(const PvtPath& path, const RtToken& typeName, PvtObject* def)
{
    PvtPath parentPath(path);
    parentPath.pop();
    return createPrim(parentPath, path.getName(), typeName, def);
}

PvtPrim* PvtStage::createPrim(const PvtPath& parentPath, const RtToken& name, const RtToken& typeName, PvtObject* def)
{
    PvtPrim* parent = getPrimAtPathLocal(parentPath);
    if (!parent)
    {
        throw ExceptionRuntimeError("Given parent path '" + parentPath.asString() + "' does not point to a prim in this stage");
    }

    // TODO: Consider using a prim factory instead.
    PvtDataHandle hnd;
    if (typeName == PvtNode::typeName())
    {
        if (!(def && def->hasApi(RtApiType::NODEDEF)))
        {
            throw ExceptionRuntimeError("No valid nodedef given for creating node '" + name.str() +
                "' at path '" + parentPath.asString() + "'");
        }
        hnd = PvtNode::createNew(name, def->hnd(), parent);
    }
    else if (typeName == PvtNodeGraph::typeName())
    {
        hnd = PvtNodeGraph::createNew(name, parent);
    }
    else if (typeName == PvtNodeDef::typeName())
    {
        hnd = PvtNodeDef::createNew(name, parent);
    }
    else
    {
        // Create a generic prim.
        hnd = PvtPrim::createNew(name, parent);
    }

    PvtPrim* prim = hnd->asA<PvtPrim>();
    parent->addChildPrim(prim);

    return prim;
}

void PvtStage::removePrim(const PvtPath& path)
{
    PvtPrim* prim = getPrimAtPathLocal(path);
    if (!(prim && prim->getParent()))
    {
        throw ExceptionRuntimeError("Given path '" + path.asString() + " does not point to a prim in this stage");
    }

    PvtPrim* parent = prim->getParent();
    parent->removeChildPrim(prim);
}

RtToken PvtStage::renamePrim(const PvtPath& path, const RtToken& newName)
{
    PvtPrim* prim = getPrimAtPathLocal(path);
    if (!(prim && prim->getParent()))
    {
        throw ExceptionRuntimeError("Given path '" + path.asString() + " does not point to a prim in this stage");
    }

    // Remove the old name from the name map.
    PvtPrim* parent = prim->getParent();
    parent->_primMap.erase(prim->getName());

    // Make sure the new name is unique and insert it to the name map.
    prim->setName(parent->makeUniqueName(newName));
    parent->_primMap[prim->getName()] = prim->shared_from_this();

    return prim->getName();
}

RtToken PvtStage::reparentPrim(const PvtPath& path, const PvtPath& newParentPath)
{
    PvtPrim* prim = getPrimAtPathLocal(path);
    if (!(prim && prim->getParent()))
    {
        throw ExceptionRuntimeError("Given path '" + path.asString() + " does not point to a prim in this stage");
    }

    PvtPrim* newParent = getPrimAtPathLocal(newParentPath);
    if (!newParent)
    {
        throw ExceptionRuntimeError("Given parent path '" + path.asString() + " does not point to a prim in this stage");
    }

    PvtPrim* oldParent = prim->getParent();
    if (newParent != oldParent)
    {
        // Remove from old parent.
        oldParent->removeChildPrim(prim);

        // Make sure the name is unique in the new parent.
        prim->setName(newParent->makeUniqueName(prim->getName()));

        // Add to new parent.
        newParent->addChildPrim(prim);
        prim->setParent(newParent);
    }

    return prim->getName();
}

PvtPrim* PvtStage::getPrimAtPath(const PvtPath& path)
{
    // First search this local stage.
    PvtPrim* prim = getPrimAtPathLocal(path);
    if (!prim)
    {
        // Then search any referenced stages as well.
        for (auto it : _refStages)
        {
            PvtStage* refStage = it->asA<PvtStage>();
            prim = refStage->getPrimAtPath(path);
            if (prim)
            {
                break;
            }
        }
    }
    return prim;
}

PvtPrim* PvtStage::getPrimAtPathLocal(const PvtPath& path)
{
    if (path.empty())
    {
        return nullptr;
    }
    if (path.size() == 1)
    {
        return _root->asA<PvtPrim>();
    }

    // TODO: Use a single map of prims keyed by path
    // instead of looping over the hierarchy
    PvtPrim* parent = _root->asA<PvtPrim>();
    PvtPrim* prim = nullptr;
    size_t i = 1;
    while (parent)
    {
        prim = parent->getChild(path[i++]);
        parent = prim && (i < path.size()) ? prim : nullptr;
    }

    return prim;
}

void PvtStage::addReference(PvtDataHandle stage)
{
    if (!stage->hasApi(RtApiType::STAGE))
    {
        throw ExceptionRuntimeError("Given object is not a valid stage");
    }
    if (_refStagesSet.count(stage))
    {
        throw ExceptionRuntimeError("Given object is not a valid stage");
    }

    stage->asA<PvtStage>()->_selfRefCount++;
    _refStages.push_back(stage);
}

void PvtStage::removeReference(const RtToken& name)
{
    for (auto it = _refStages.begin(); it != _refStages.end(); ++it)
    {
        PvtStage* stage = (*it)->asA<PvtStage>();
        if (stage->getName() == name)
        {
            stage->_selfRefCount--;
            _refStagesSet.erase(*it);
            _refStages.erase(it);
            break;
        }
    }
}

void PvtStage::removeReferences()
{
    _selfRefCount = 0;
    _refStages.clear();
    _refStagesSet.clear();
}

size_t PvtStage::numReferences() const
{
    return _refStages.size();
}

PvtStage* PvtStage::getReference(size_t index) const
{
    return index < _refStages.size() ? _refStages[index]->asA<PvtStage>() : nullptr;
}

PvtStage* PvtStage::findReference(const RtToken& name) const
{
    for (auto it = _refStages.begin(); it != _refStages.end(); ++it)
    {
        PvtStage* stage = (*it)->asA<PvtStage>();
        if (stage->getName() == name)
        {
            return (*it)->asA<PvtStage>();
        }
    }
    return nullptr;
}

}
