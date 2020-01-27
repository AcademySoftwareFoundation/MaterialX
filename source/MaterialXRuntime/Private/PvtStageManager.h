//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTSTAGEMANAGER_H
#define MATERIALX_PVTSTAGEMANAGER_H

#include <MaterialXRuntime/RtObject.h>

#include <unordered_map>

namespace MaterialX
{

class PvtStageManager
{
  public:
    /// Constructor
    PvtStageManager();

    /// Creates a stage
    RtObject createStage(const string& stageName);

    /// Deletes a stage known to the stage manager
    void deleteStage(const string& stageName);

    /// Whether the stage manager contains a stage with the given name
    bool hasStage(const string& stageName) const;

    /// Retrieves a stage from the stage manager
    RtObject getStageObject(const string& stageName) const;

  private:
    std::unordered_map<string, RtObject> _stages;
};

}

#endif
