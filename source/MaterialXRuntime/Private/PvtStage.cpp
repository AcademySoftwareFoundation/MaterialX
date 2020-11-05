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
        // Second, try finding a registered nodedef.
        const RtPrim nodedef = RtApi::get().getNodeDef(typeName);
        if (nodedef)
        {
            // A nodedef exists with this name, so create a node instance.
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

    // Destroy this prim and all its children.
    prim->destroy();

    // Remove from its parent.
    PvtPrim* parent = prim->getParent();
    parent->removeChildPrim(prim);
}

void PvtStage::disposePrim(const PvtPath& path)
{
    PvtPrim* prim = getPrimAtPathLocal(path);
    if (!(prim && prim->getParent()))
    {
        throw ExceptionRuntimeError("Given path '" + path.asString() + " does not point to a prim in this stage");
    }

    // Make sure the prim has no connections.
    for (RtAttribute attr : prim->getAttributes())
    {
        if (attr.isA<RtInput>() && attr.asA<RtInput>().isConnected())
        {
            throw ExceptionRuntimeError("Found a connection to '" + attr.getName().str() + "'. Cannot dispose a prim with connections.");
        }
        else if (attr.isA<RtOutput>() && attr.asA<RtOutput>().isConnected())
        {
            throw ExceptionRuntimeError("Found a connection from '" + attr.getName().str() + "'. Cannot dispose a prim with connections.");
        }
    }

    // Dispose this prim and all its children.
    prim->dispose(true);

    // Remove from its parent.
    PvtPrim* parent = prim->getParent();
    parent->removeChildPrim(prim);
}

void PvtStage::restorePrim(const PvtPath& parentPath, const RtPrim& primH)
{
    // Use explicit cast since the asA() method will throw
    // in debug mode when the prim is disposed.
    PvtPrim* prim = static_cast<PvtPrim*>(primH._hnd.get());
    if (!prim->isDisposed())
    {
        throw ExceptionRuntimeError("Trying to revive prim '" + prim->getPath().asString() + "' that is already alive.");
    }

    PvtPrim* parent = getPrimAtPathLocal(parentPath);
    if (!parent)
    {
        throw ExceptionRuntimeError("Given parent path '" + parentPath.asString() + "' does not point to a prim in this stage.");
    }

    prim->dispose(false);
    parent->addChildPrim(prim);
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
    prim->setName(parent->makeUniqueChildName(newName));
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
        prim->setName(newParent->makeUniqueChildName(prim->getName()));

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
        for (const RtStagePtr& stage : _refStages)
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
    const RtStage* s = stage.get();
    if (_refStagesSet.count(s))
    {
        throw ExceptionRuntimeError("A reference to this stage already exists");
    }

    PvtStage::ptr(stage)->_selfRefCount++;
    _refStages.push_back(stage);
    _refStagesSet.insert(s);
}

void PvtStage::removeReference(const RtToken& name)
{
    for (auto it = _refStages.begin(); it != _refStages.end(); ++it)
    {
        const RtStagePtr& s = *it;
        if (s->getName() == name)
        {
            PvtStage::ptr(s)->_selfRefCount--;
            _refStages.erase(it);
            _refStagesSet.erase(s.get());
            break;
        }
    }
}

void PvtStage::removeReferences()
{
    // Decrease self ref count on all stages.
    for (const RtStagePtr& stage : _refStages)
    {
        PvtStage::ptr(stage)->_selfRefCount--;
    }
    // Removed them.
    _refStages.clear();
    _refStagesSet.clear();
}

RtStagePtr PvtStage::getReference(const RtToken& name) const
{
    for (const RtStagePtr& stage : _refStages)
    {
        if (stage->getName() == name)
        {
            return stage;
        }
    }
    return nullptr;
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
