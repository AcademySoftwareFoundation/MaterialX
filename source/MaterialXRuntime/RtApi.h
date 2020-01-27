//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTAPI_H
#define MATERIALX_RTAPI_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtPrim.h>

namespace MaterialX
{

/// Function type for creating prims for a typed schema.
using RtPrimCreateFunc = std::function<RtPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)>;

class RtApi
{
public:
    ~RtApi();

    void initialize();

    void shutdown();

    void registerCreateFunction(const RtToken& typeName, RtPrimCreateFunc func);

    void unregisterCreateFunction(const RtToken& typeName);

    bool hasCreateFunction(const RtToken& typeName);

    RtPrimCreateFunc getCreateFunction(const RtToken& typeName);

    void registerMasterPrim(const RtPrim& prim);

    void unregisterMasterPrim(const RtToken& name);

    bool hasMasterPrim(const RtToken& name);

    RtPrim getMasterPrim(const RtToken& name);

    template<class T>
    void registerTypedSchema()
    {
        registerCreateFunction(T::typeName(), T::createPrim);
    }

    template<class T>
    void unregisterTypedSchema()
    {
        unregisterCreateFunction(T::typeName());
    }

    /// Create a new empty stage.
    RtStagePtr createStage(const RtToken& name);

    /// Delete a stage.
    void deleteStage(const RtToken& name);

    /// Return a stage by name.
    RtStagePtr getStage(const RtToken& name) const;

    /// Get the singleton API instance.
    static RtApi& get();

    RtApi(const RtApi&) = delete;
    const RtApi& operator=(const RtApi&) = delete;

protected:
    RtApi();
    void* _ptr;
};

}

#endif
