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
#include <MaterialXRuntime/RtGeneric.h>
#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtFileIo.h>
#include <MaterialXRuntime/RtLook.h>
#include <MaterialXRuntime/RtCollection.h>
#include <MaterialXRuntime/RtConnectableApi.h>
#include <MaterialXRuntime/Codegen/RtSourceCodeImpl.h>
#include <MaterialXRuntime/Codegen/RtSubGraphImpl.h>

#include <MaterialXRuntime/Private/PvtApi.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

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
    _cast(_ptr)->reset();

    // Register built in schemas
    registerTypedSchema<RtGeneric>();
    registerTypedSchema<RtNode>();
    registerTypedSchema<RtNodeDef>();
    registerTypedSchema<RtNodeGraph>();
    registerTypedSchema<RtNodeImpl>();
    registerTypedSchema<RtTargetDef>();
    registerTypedSchema<RtSourceCodeImpl>();
    registerTypedSchema<RtSubGraphImpl>();
    registerTypedSchema<RtBackdrop>();
    registerTypedSchema<RtBindElement>();
    registerTypedSchema<RtLookGroup, RtLookGroupConnectableApi>();
    registerTypedSchema<RtLook, RtLookConnectableApi>();
    registerTypedSchema<RtMaterialAssign, RtMaterialAssignConnectableApi>();
    registerTypedSchema<RtCollection, RtCollectionConnectableApi>();
}

void RtApi::shutdown()
{
    _cast(_ptr)->reset();

    // Unregister built in schemas
    unregisterTypedSchema<RtGeneric>();
    unregisterTypedSchema<RtNode>();
    unregisterTypedSchema<RtNodeDef>();
    unregisterTypedSchema<RtNodeGraph>();
    unregisterTypedSchema<RtNodeImpl>();
    unregisterTypedSchema<RtTargetDef>();
    unregisterTypedSchema<RtSourceCodeImpl>();
    unregisterTypedSchema<RtSubGraphImpl>();
    unregisterTypedSchema<RtBackdrop>();
    unregisterTypedSchema<RtBindElement>();
    unregisterTypedSchema<RtLookGroup>();
    unregisterTypedSchema<RtLook>();
    unregisterTypedSchema<RtMaterialAssign>();
    unregisterTypedSchema<RtCollection>();
}

void RtApi::registerLogger(RtLoggerPtr logger)
{
    _cast(_ptr)->registerLogger(logger);
}

void RtApi::unregisterLogger(RtLoggerPtr logger)
{
    _cast(_ptr)->unregisterLogger(logger);
}

void RtApi::log(RtLogger::MessageType type, const string& msg)
{
    _cast(_ptr)->log(type, msg);
}

void RtApi::registerCreateFunction(const RtIdentifier& typeName, RtPrimCreateFunc func)
{
    _cast(_ptr)->registerCreateFunction(typeName, func);
}

void RtApi::unregisterCreateFunction(const RtIdentifier& typeName)
{
    _cast(_ptr)->unregisterCreateFunction(typeName);
}

bool RtApi::hasCreateFunction(const RtIdentifier& typeName) const
{
    return _cast(_ptr)->hasCreateFunction(typeName);
}

RtPrimCreateFunc RtApi::getCreateFunction(const RtIdentifier& typeName) const
{
    return _cast(_ptr)->getCreateFunction(typeName);
}

template<> void RtApi::registerDefinition<RtNodeDef>(const RtPrim& prim)
{
    return _cast(_ptr)->registerNodeDef(prim);
}

template<> void RtApi::unregisterDefinition<RtNodeDef>(const RtIdentifier& name)
{
    return _cast(_ptr)->unregisterNodeDef(name);
}

template<> bool RtApi::hasDefinition<RtNodeDef>(const RtIdentifier& name) const
{
    return _cast(_ptr)->hasNodeDef(name);
}

template<> RtPrim RtApi::getDefinition<RtNodeDef>(const RtIdentifier& name) const
{
    PvtObject* obj = _cast(_ptr)->getNodeDef(name);
    return obj ? obj->hnd() : RtPrim();
}

template<> size_t RtApi::numDefinitions<RtNodeDef>() const
{
    return _cast(_ptr)->numNodeDefs();
}

template<> RtPrim RtApi::getDefinition<RtNodeDef>(size_t index) const
{
    PvtObject* obj = _cast(_ptr)->getNodeDef(index);
    return obj ? obj->hnd() : RtPrim();
}

template<> void RtApi::registerImplementation<RtNodeGraph>(const RtPrim& prim)
{
    return _cast(_ptr)->registerNodeGraph(prim);
}

template<> void RtApi::unregisterImplementation<RtNodeGraph>(const RtIdentifier& name)
{
    return _cast(_ptr)->unregisterNodeGraph(name);
}

template<> bool RtApi::hasImplementation<RtNodeGraph>(const RtIdentifier& name) const
{
    return _cast(_ptr)->hasNodeGraph(name);
}

template<> RtPrim RtApi::getImplementation<RtNodeGraph>(const RtIdentifier& name) const
{
    PvtObject* obj = _cast(_ptr)->getNodeGraph(name);
    return obj ? obj->hnd() : RtPrim();
}

template<> size_t RtApi::numImplementations<RtNodeGraph>() const
{
    return _cast(_ptr)->numNodeGraphs();
}

template<> RtPrim RtApi::getImplementation<RtNodeGraph>(size_t index) const
{
    PvtObject* obj = _cast(_ptr)->getNodeGraph(index);
    return obj ? obj->hnd() : RtPrim();
}

void RtApi::clearSearchPath()
{
    _cast(_ptr)->clearSearchPath();
}

void RtApi::clearTextureSearchPath()
{
    _cast(_ptr)->clearTextureSearchPath();
}

void RtApi::clearImplementationSearchPath()
{
    _cast(_ptr)->clearImplementationSearchPath();
}

void RtApi::setSearchPath(const FileSearchPath& searchPath)
{
    _cast(_ptr)->setSearchPath(searchPath);
}

void RtApi::setTextureSearchPath(const FileSearchPath& searchPath)
{
    _cast(_ptr)->setTextureSearchPath(searchPath);
}

void RtApi::setImplementationSearchPath(const FileSearchPath& searchPath)
{
    _cast(_ptr)->setImplementationSearchPath(searchPath);
}

const FileSearchPath& RtApi::getSearchPath() const
{
    return _cast(_ptr)->getSearchPath();
}

const FileSearchPath& RtApi::getTextureSearchPath() const
{
    return _cast(_ptr)->getTextureSearchPath();
}

const FileSearchPath& RtApi::getImplementationSearchPath() const
{
    return _cast(_ptr)->getImplementationSearchPath();
}

RtStagePtr RtApi::loadLibrary(const RtIdentifier& name, const FilePath& path, const RtReadOptions* options, bool forceReload)
{
    return _cast(_ptr)->loadLibrary(name, path, options, forceReload);
}

void RtApi::unloadLibrary(const RtIdentifier& name)
{
    return _cast(_ptr)->unloadLibrary(name);
}

void RtApi::unloadLibraries()
{
    return _cast(_ptr)->unloadLibraries();
}

size_t RtApi::numLibraries() const
{
    return _cast(_ptr)->numLibraries();
}

RtStagePtr RtApi::getLibrary(const RtIdentifier& name) const
{
    return _cast(_ptr)->getLibrary(name);
}

RtStagePtr RtApi::getLibrary(size_t index) const
{
    return _cast(_ptr)->getLibrary(index);
}

RtStagePtr RtApi::createStage(const RtIdentifier& name)
{
    return _cast(_ptr)->createStage(name);
}

void RtApi::deleteStage(const RtIdentifier& name)
{
    _cast(_ptr)->deleteStage(name);
}

void RtApi::deleteStages()
{
    _cast(_ptr)->deleteStages();
}

size_t RtApi::numStages() const
{
    return _cast(_ptr)->numStages();
}

RtStagePtr RtApi::getStage(const RtIdentifier& name) const
{
    return _cast(_ptr)->getStage(name);
}

RtStagePtr RtApi::getStage(size_t index) const
{
    return _cast(_ptr)->getStage(index);
}

RtIdentifier RtApi::renameStage(const RtIdentifier& name, const RtIdentifier& newName)
{
    return _cast(_ptr)->renameStage(name, newName);
}

UnitConverterRegistryPtr RtApi::getUnitDefinitions()
{
    return _cast(_ptr)->getUnitDefinitions();
}

RtApi& RtApi::get()
{
    static RtApi _instance;
    return _instance;
}

}
