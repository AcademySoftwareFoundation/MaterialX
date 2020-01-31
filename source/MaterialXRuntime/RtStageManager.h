//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTSTAGEMANAGER_H
#define MATERIALX_RTSTAGEMANAGER_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtObject.h>

#include <memory>

namespace MaterialX
{

class PvtStageManager;

using RtStageManagerPtr = std::shared_ptr<class RtStageManager>;

/// @class RtStageManager
/// API for creating and accessing stages.
class RtStageManager
{
public:
    /// Creates a stage manager
    static RtStageManagerPtr createNew();
    
    /// Destructor
    virtual ~RtStageManager();

    /// Creates a stage
    virtual RtObject createStage(const string& stageName);
    
    /// Deletes a stage known to the stage manager
    virtual void deleteStage(const string& stageName);
    
    /// Renames a stage known to the stage manager
    void renameStage(const string& oldName, const string& newName);

    /// Whether the stage manager contains a stage with the given name
    bool hasStage(const string& stageName) const;

    /// Retrieves a stage from the stage manager
    RtObject getStageObject(const string& stageName) const;

    /// Retrieve the stage objects from the stage manager
    const std::unordered_map<string, RtObject>& getStageObjects() const;

protected:
    RtStageManager();

    friend class PvtStageManager;
    std::unique_ptr<PvtStageManager> _stageManager;
};

}

#endif
