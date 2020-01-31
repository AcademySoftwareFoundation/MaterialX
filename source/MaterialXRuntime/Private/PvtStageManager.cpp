//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtStageManager.h>

#include <MaterialXRuntime/RtStage.h>

namespace MaterialX
{

PvtStageManager::PvtStageManager()
{
}

RtObject PvtStageManager::createStage(const string& stageName)
{
    if (_stages.count(stageName) == 0)
    {
        RtObject stageObject = RtStage::createNew(RtToken(stageName));
        _stages.emplace(stageName, stageObject);
        return stageObject;
    }
    else
    {
        // TODO: In the future increment the stageName instead of throwing an exception
        throw ExceptionRuntimeError("Stage name: " + stageName + " already exists");
    }
}

void PvtStageManager::deleteStage(const string& stageName)
{
    if (hasStage(stageName))
    {
        _stages.erase(stageName);
    }
    else
    {
        throw ExceptionRuntimeError("Stage name: " + stageName + " doesn't exist");
    }
}

void PvtStageManager::renameStage(const string& oldName, const string& newName)
{
    if (hasStage(oldName))
    {
        RtObject stageObject = _stages.at(oldName);
        _stages.erase(oldName);
        _stages.emplace(newName, stageObject);
    }
    else
    {
        throw ExceptionRuntimeError("Stage name: " + oldName + " doesn't exist");
    }
}

bool PvtStageManager::hasStage(const string& stageName) const
{
    return _stages.find(stageName) != _stages.end();
}

RtObject PvtStageManager::getStageObject(const string& stageName) const
{
    return _stages.at(stageName);
}

const std::unordered_map<string, RtObject>& PvtStageManager::getStageObjects() const
{
    return _stages;
}

}
