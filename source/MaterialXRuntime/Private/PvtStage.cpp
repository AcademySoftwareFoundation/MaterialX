//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtStage.h>
#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtTraversal.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

const RtTypeInfo PvtStage::RootPrim::_typeInfo("stagerootprim");

PvtStage::RootPrim::RootPrim(RtStageWeakPtr stage) :
    PvtPrim(&_typeInfo, PvtPath::ROOT_NAME, nullptr),
    _stage(stage)
{
}

PvtStage::PvtStage(const RtToken& name, RtStageWeakPtr owner) :
    _name(name),
    _root(nullptr),
    _selfRefCount(0)
{
    _root = PvtDataHandle(new RootPrim(owner));
}

PvtPrim* PvtStage::createPrim(const PvtPath& path, const RtToken& typeName)
{
    PvtPath parentPath(path);
    parentPath.pop();
    return createPrim(parentPath, path.getName(), typeName);
}

PvtPrim* PvtStage::createPrim(const PvtPath& parentPath, const RtToken& name, const RtToken& typeName)
{
    PvtPrim* parent = getPrimAtPathLocal(parentPath);
    if (!parent)
    {
        throw ExceptionRuntimeError("Given parent path '" + parentPath.asString() + "' does not point to a prim in this stage");
    }

    PvtDataHandle primH;

    // First, try finding a registered creator function for this typename.
    RtPrimCreateFunc creator = RtApi::get().getCreateFunction(typeName);
    if (creator)
    {
        primH = PvtObject::hnd(creator(typeName, name, parent->hnd()));
    }
    else
    {
        // Second, try finding a registered master prim.
        const RtPrim master = RtApi::get().getMasterPrim(typeName);
        if (master && master.getTypeInfo()->getShortTypeName() == RtNodeDef::typeName())
        {
            // This is a nodedef, so create a node instance from it.
            RtPrimCreateFunc nodeCreator = RtApi::get().getCreateFunction(RtNode::typeName());
            primH = PvtObject::hnd(nodeCreator(typeName, name, parent->hnd()));
        }
        else
        {
            throw ExceptionRuntimeError("Don't know how to create a prim with typename '" + typeName.str() + "'");
        }
    }

    PvtPrim* prim = primH->asA<PvtPrim>();
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

    // Dispose this prim and all its children.
    prim->dispose();

    // Remove from its parent.
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
    parent->_primMap[prim->getName()] = prim->hnd();

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
        for (const RtStagePtr& stage : _refStagesOrder)
        {
            PvtStage* refStage = PvtStage::ptr(stage);
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

RtPrimIterator PvtStage::getPrims(RtObjectPredicate predicate)
{
    return RtPrimIterator(_root, predicate);
}

void PvtStage::addReference(RtStagePtr stage)
{
    if (_refStagesMap.count(stage->getName()))
    {
        throw ExceptionRuntimeError("A reference to this stage already exists");
    }

    PvtStage::ptr(stage)->_selfRefCount++;
    _refStagesMap[stage->getName()] = stage;
    _refStagesOrder.push_back(stage);
}

void PvtStage::removeReference(const RtToken& name)
{
    auto it = _refStagesMap.find(name);
    if (it != _refStagesMap.end())
    {
        PvtStage::ptr(it->second)->_selfRefCount--;

        _refStagesMap.erase(it);

        for (auto it2 = _refStagesOrder.begin(); it2 != _refStagesOrder.end(); ++it2)
        {
            if ((*it2)->getName() == name)
            {
                _refStagesOrder.erase(it2);
            }
        }
    }
}

void PvtStage::removeReferences()
{
    // Decrease self ref count on all stages.
    for (const RtStagePtr& stage : _refStagesOrder)
    {
        PvtStage::ptr(stage)->_selfRefCount--;
    }
    // Removed them.
    _refStagesMap.clear();
    _refStagesOrder.clear();
}

PvtStage* PvtStage::findReference(const RtToken& name) const
{
    auto it = _refStagesMap.find(name);
    return it != _refStagesMap.end() ? PvtStage::ptr(it->second) : nullptr;
}


PvtStageIterator& PvtStageIterator::operator++()
{
    while (true)
    {
        if (_stack.empty())
        {
            // Traversal is complete.
            abort();
            return *this;
        }

        StackFrame& frame = _stack.back();
        PvtStage* stage = std::get<0>(frame);
        int& primIndex = std::get<1>(frame);
        int& stageIndex = std::get<2>(frame);

        bool pop = true;

        if (primIndex + 1 < int(stage->getRootPrim()->getAllChildren().size()))
        {
            _current = stage->getRootPrim()->getAllChildren()[++primIndex];
            if (!_predicate || _predicate(_current->obj()))
            {
                return *this;
            }
            pop = false;
        }
        else if (stageIndex + 1 < int(stage->getAllReferences().size()))
        {
            PvtStage* refStage = PvtStage::ptr(stage->getAllReferences()[++stageIndex]);
            if (!refStage->getRootPrim()->getAllChildren().empty())
            {
                _stack.push_back(std::make_tuple(refStage, 0, stageIndex));
                _current = refStage->getRootPrim()->getAllChildren()[0];
                if (!_predicate || _predicate(_current->obj()))
                {
                    return *this;
                }
                pop = false;
            }
        }

        if (pop)
        {
            _stack.pop_back();
        }
    }
    return *this;
}

}
