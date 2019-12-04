//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtStage.h>
#include <MaterialXRuntime/Private/PvtNodeDef.h>
#include <MaterialXRuntime/Private/PvtNode.h>
#include <MaterialXRuntime/Private/PvtNodeGraph.h>
#include <MaterialXRuntime/Private/PvtPath.h>

#include <MaterialXRuntime/RtObject.h>

#include <MaterialXCore/Util.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

PvtStage::PvtStage(const RtToken& name) :
    PvtAllocatingElement(RtObjType::STAGE, name),
    _selfRefCount(0)
{
}

PvtDataHandle PvtStage::createNew(const RtToken& name)
{
    return std::make_shared<PvtStage>(name);
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

PvtDataHandle PvtStage::getReference(size_t index) const
{
    return index < _refStages.size() ? _refStages[index] : nullptr;
}

PvtDataHandle PvtStage::findReference(const RtToken& name) const
{
    for (auto it = _refStages.begin(); it != _refStages.end(); ++it)
    {
        PvtStage* stage = (*it)->asA<PvtStage>();
        if (stage->getName() == name)
        {
            return *it;
        }
    }
    return nullptr;
}

PvtDataHandle PvtStage::findChildByName(const RtToken& name) const
{
    auto it = _childrenByName.find(name);
    if (it != _childrenByName.end())
    {
        return it->second;
    }
    for (auto rs : _refStages)
    {
        PvtStage* refStage = rs->asA<PvtStage>();
        PvtDataHandle elem = refStage->findChildByName(name);
        if (elem)
        {
            return elem;
        }
    }
    return nullptr;
}

PvtDataHandle PvtStage::findChildByPath(const string& path) const
{
    const StringVec elementNames = splitString(path, PvtPath::SEPARATOR);
    if (elementNames.empty())
    {
        return nullptr;
    }

    size_t i = 0;
    RtToken name(elementNames[i++]);
    PvtDataHandle elem = findChildByName(name);

    while (elem && i < elementNames.size())
    {
        name = elementNames[i++];
        if (elem->getObjType() == RtObjType::NODE)
        {
            // For nodes find the portdef on the corresponding nodedef
            PvtNode* node = elem->asA<PvtNode>();
            PvtNodeDef* nodedef = node->getNodeDef()->asA<PvtNodeDef>();
            elem = nodedef->findChildByName(name);
        }
        else
        {
            elem = elem->asA<PvtElement>()->findChildByName(name);
        }
    }

    if (!elem || i < elementNames.size())
    {
        // The full path was not found so search
        // any referenced stages as well.
        for (auto it : _refStages)
        {
            PvtStage* refStage = it->asA<PvtStage>();
            elem = refStage->findChildByPath(path);
            if (elem)
            {
                break;
            }
        }
    }

    return elem;
}

}
