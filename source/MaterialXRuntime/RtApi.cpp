//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtBackdrop.h>
#include <MaterialXRuntime/RtStage.h>

#include <MaterialXRuntime/Private/PvtObject.h>

namespace MaterialX
{

class PvtApi
{
public:
    void registerCreateFunction(const RtToken& typeName, RtPrimCreateFunc creator)
    {
        if (getCreateFunction(typeName))
        {
            throw ExceptionRuntimeError("A create function for type name '" + typeName.str() + "' is already registered");
        }
        _createFunctions[typeName] = creator;
    }

    void unregisterCreateFunction(const RtToken& typeName)
    {
        _createFunctions.erase(typeName);
    }

    bool hasCreateFunction(const RtToken& typeName)
    {
        return _createFunctions.count(typeName) > 0;
    }

    RtPrimCreateFunc getCreateFunction(const RtToken& typeName)
    {
        auto it = _createFunctions.find(typeName);
        return it != _createFunctions.end() ? it->second : nullptr;
    }

    void registerMasterPrim(const RtPrim& prim)
    {
        if (getMasterPrim(prim.getName()))
        {
            throw ExceptionRuntimeError("A master prim with name '" + prim.getName().str() + "' is already registered");
        }
        _masterPrims[prim.getName()] = PvtObject::hnd(prim);
    }

    void unregisterMasterPrim(const RtToken& name)
    {
        _masterPrims.erase(name);
    }

    bool hasMasterPrim(const RtToken& typeName)
    {
        return _masterPrims.count(typeName) > 0;
    }

    RtPrim getMasterPrim(const RtToken& name)
    {
        auto it = _masterPrims.find(name);
        return it != _masterPrims.end() ? it->second : RtPrim();
    }

    RtStagePtr createStage(const RtToken& name)
    {
        if (getStage(name))
        {
            throw ExceptionRuntimeError("A stage with name '" + name.str() + "' already exists");
        }
        RtStagePtr stage = RtStage::createNew(name);
        _stages[name] = stage;
        return stage;
    }

    void deleteStage(const RtToken& name)
    {
        _stages.erase(name);
    }

    RtStagePtr getStage(const RtToken& name) const
    {
        auto it = _stages.find(name);
        return it != _stages.end() ? it->second : RtStagePtr();
    }

    void clear()
    {
        _createFunctions.clear();
        _masterPrims.clear();
        _stages.clear();
    }

private:
    RtTokenMap<RtPrimCreateFunc> _createFunctions;
    RtTokenMap<PvtDataHandle> _masterPrims;
    RtTokenMap<RtStagePtr> _stages;
};


namespace 
{
    // Syntactic sugar
    inline PvtApi* _cast(void* ptr)
    {
        return static_cast<PvtApi*>(ptr);
    }
}

RtApi::RtApi() :
    _ptr(new PvtApi())
{
}

RtApi::~RtApi()
{
    delete _cast(_ptr);
}

void RtApi::initialize()
{
    _cast(_ptr)->clear();

    // Register built in schemas
    registerTypedSchema<RtGeneric>();
    registerTypedSchema<RtNode>();
    registerTypedSchema<RtNodeDef>();
    registerTypedSchema<RtNodeGraph>();
    registerTypedSchema<RtBackdrop>();
}

void RtApi::shutdown()
{
    _cast(_ptr)->clear();
}


void RtApi::registerCreateFunction(const RtToken& typeName, RtPrimCreateFunc func)
{
    _cast(_ptr)->registerCreateFunction(typeName, func);
}

void RtApi::unregisterCreateFunction(const RtToken& typeName)
{
    _cast(_ptr)->unregisterCreateFunction(typeName);
}

bool RtApi::hasCreateFunction(const RtToken& typeName)
{
    return _cast(_ptr)->hasCreateFunction(typeName);
}

RtPrimCreateFunc RtApi::getCreateFunction(const RtToken& typeName)
{
    return _cast(_ptr)->getCreateFunction(typeName);
}

void RtApi::registerMasterPrim(const RtPrim& prim)
{
    _cast(_ptr)->registerMasterPrim(prim);
}

void RtApi::unregisterMasterPrim(const RtToken& name)
{
    _cast(_ptr)->unregisterMasterPrim(name);
}

bool RtApi::hasMasterPrim(const RtToken& name)
{
    return _cast(_ptr)->hasMasterPrim(name);
}

RtPrim RtApi::getMasterPrim(const RtToken& name)
{
    return _cast(_ptr)->getMasterPrim(name);
}

RtStagePtr RtApi::createStage(const RtToken& name)
{
    return _cast(_ptr)->createStage(name);
}

void RtApi::deleteStage(const RtToken& name)
{
    _cast(_ptr)->deleteStage(name);
}

RtStagePtr RtApi::getStage(const RtToken& name) const
{
    return _cast(_ptr)->getStage(name);
}


RtApi& RtApi::get()
{
    static RtApi _instance;
    return _instance;
}

}
