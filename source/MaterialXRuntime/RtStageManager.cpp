//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtStageManager.h>

#include <MaterialXRuntime/Private/PvtStageManager.h>

namespace MaterialX
{


RtStageManagerPtr RtStageManager::createNew()
{
    RtStageManagerPtr result(new RtStageManager());
    return result;
}

RtStageManager::RtStageManager()
    : _stageManager(new PvtStageManager())
{
}

RtStageManager::~RtStageManager()
{
}

RtObject RtStageManager::createStage(const string& stageName)
{
    return _stageManager->createStage(stageName);
}

void RtStageManager::deleteStage(const string& stageName)
{
    _stageManager->deleteStage(stageName);
}

bool RtStageManager::hasStage(const string& stageName) const
{
    return _stageManager->hasStage(stageName);
}

RtObject RtStageManager::getStageObject(const string& stageName) const
{
    return _stageManager->getStageObject(stageName);
}

}
