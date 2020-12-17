//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_RTAPI_H
#define MATERIALX_RTAPI_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/Library.h>
#include <MaterialXRuntime/RtLogger.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtConnectableApi.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtFileIo.h>

#include <MaterialXFormat/File.h>

#include <MaterialXCore/Unit.h>

namespace MaterialX
{

class RtApi
{
public:
    ~RtApi();

    /// Initialize the API session.
    void initialize();

    /// Shutdown the API session.
    void shutdown();


    /// Registers a logger with the API
    void registerLogger(RtLoggerPtr logger);

    /// Unregisters a logger with the API
    void unregisterLogger(RtLoggerPtr logger);

    /// Logs a message with the registered loggers
    void log(RtLogger::MessageType type, const string& msg);

    /// @name Create functions
    /// @{

    /// Register a create function for a typename.
    void registerCreateFunction(const RtToken& typeName, RtPrimCreateFunc func);

    /// Unregister a create function.
    void unregisterCreateFunction(const RtToken& typeName);

    /// Return true if the given typename has a create function registered.
    bool hasCreateFunction(const RtToken& typeName) const;

    /// Return the create function for given typename.
    /// Or nullptr if no such create function has been registered.
    RtPrimCreateFunc getCreateFunction(const RtToken& typeName) const;

    /// @}
    /// @name NodeDefs
    /// @{

    /// Register a nodedef prim to be used for creating node instances from.
    void registerNodeDef(const RtPrim& prim);

    /// Unregister a nodedef.
    void unregisterNodeDef(const RtToken& name);

    /// Return true if a nodedef prim with the given name has been registered.
    bool hasNodeDef(const RtToken& name) const;

    /// Return the number of registered nodedefs prims.
    size_t numNodeDefs() const;

    /// Return a registered nodedef prim by index.
    RtPrim getNodeDef(size_t index) const;

    /// Return a registered nodedef prim by name.
    RtPrim getNodeDef(const RtToken& name) const;

    /// @}
    /// @name NodeImpls
    /// @{

    /// Register a node implementation prim to be used as the implementation
    /// of a nodedef for a specific target.
    void registerNodeImpl(const RtPrim& prim);

    /// Unregister a node implementation prim.
    void unregisterNodeImpl(const RtToken& name);

    /// Return true if a node implementation prim with the given name has been registered.
    bool hasNodeImpl(const RtToken& name) const;

    /// Return the number of registered node implementation prims.
    size_t numNodeImpls() const;

    /// Return a registered node implementation prim by index.
    RtPrim getNodeImpl(size_t index) const;

    /// Return a registered noded implementation prim by name.
    RtPrim getNodeImpl(const RtToken& name) const;

    /// @}
    /// @name TargetDefs
    /// @{

    /// Register a targetdef prim specifying the name and inheritance of an implementation target.
    void registerTargetDef(const RtPrim& prim);

    /// Unregister a targetdef prim.
    void unregisterTargetDef(const RtToken& name);

    /// Return true if a targetdef prim with the given name has been registered.
    bool hasTargetDef(const RtToken& name) const;

    /// @}
    /// @name TypedSchema registration
    /// @{

    /// Register a typed prim schema.
    template<class T, class ConnectableApi = RtConnectableApi>
    void registerTypedSchema()
    {
        registerCreateFunction(T::typeName(), T::createPrim);
        RtConnectableApi::registerApi<T, ConnectableApi>();
    }

    /// Unregister a typed prim schema.
    template<class T>
    void unregisterTypedSchema()
    {
        unregisterCreateFunction(T::typeName());
        RtConnectableApi::unregisterApi<T>();
    }

    /// @}
    /// @name Search paths
    /// @{

    /// Clear the definition search path.
    void clearSearchPath();

    /// Clear the texture search path.
    void clearTextureSearchPath();

    /// Clear the implementation search path.
    void clearImplementationSearchPath();

    /// Set search path for definition libraries. Can be called multiple times
    /// to append to the current search path.
    void setSearchPath(const FileSearchPath& searchPath);

    /// Set search path for texture resources. Can be called multiple times
    /// to append to the current search path.
    void setTextureSearchPath(const FileSearchPath& searchPath);

    /// Set search path for implementations used by libraries. Can be called multiple times
    /// to append to the current search path.
    void setImplementationSearchPath(const FileSearchPath& searchPath);

    /// Get the search path for definition libraries.
    const FileSearchPath& getSearchPath() const;

    /// Get search path for texture resources.
    const FileSearchPath& getTextureSearchPath() const;

    /// Get search path for implementations used by libraries.
    const FileSearchPath& getImplementationSearchPath() const;

    /// Set location for non-library user definitions
    const FilePath& getUserDefinitionPath() const;

    /// Set location for non-library user definitions
    void setUserDefinitionPath(const FilePath& path);

    /// @}
    /// @name Library management
    /// @{

    /// Create a library.
    void createLibrary(const RtToken& name);

    /// Load a library.
    void loadLibrary(const RtToken& name, const RtReadOptions& options);

    /// Unload a library.
    void unloadLibrary(const RtToken& name);

    /// Return a list of all loaded libraries.
    RtTokenVec getLibraryNames() const;

    /// Return a particular library stage
    RtStagePtr getLibrary(const RtToken& name);

    /// Return the library stage containing all loaded libraries.
    RtStagePtr getLibrary();

    /// @}
    /// @name Stage management
    /// @{

    /// Create a new empty stage.
    RtStagePtr createStage(const RtToken& name);

    /// Delete a stage.
    void deleteStage(const RtToken& name);

    /// Return a stage by name.
    RtStagePtr getStage(const RtToken& name) const;

    /// Rename a stage.
    RtToken renameStage(const RtToken& name, const RtToken& newName);

    /// Return a list of all stages created.
    RtTokenVec getStageNames() const;

    /// @}

    /// Return a registry of unit definitions
    UnitConverterRegistryPtr getUnitDefinitions();

    /// Get the singleton API instance.
    static RtApi& get();

    RtApi(const RtApi&) = delete;
    const RtApi& operator=(const RtApi&) = delete;

protected:
    RtApi();

    void* _ptr;
    friend class PvtApi;
};


/// RAII class for scoped initialization and shutdown
/// of the API instance.
class RtScopedApiHandle
{
public:
    /// Constructor.
    RtScopedApiHandle()
    {
        RtApi::get().initialize();
    }

    /// Destructor.
    ~RtScopedApiHandle()
    {
        RtApi::get().shutdown();
    }

    /// Access a pointer to the api instance.
    RtApi* operator->()
    {
        return &RtApi::get();
    }

    /// Access a reference to the api instance.
    RtApi& operator*()
    {
        return RtApi::get();
    }
};

}

#endif
