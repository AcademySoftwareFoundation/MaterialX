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

    /// @name Prim create functions
    /// @{

    /// Register a create function for a typename.
    void registerCreateFunction(const RtToken& typeName, RtPrimCreateFunc func);

    /// Unregister a create function.
    void unregisterCreateFunction(const RtToken& typeName);

    /// Return true if the given typename has a create function registered.
    bool hasCreateFunction(const RtToken& typeName) const;

    /// Return the create function for given typename,
    /// or nullptr if no such create function has been registered.
    RtPrimCreateFunc getCreateFunction(const RtToken& typeName) const;

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
    /// @name Definitions
    /// @{

     /// Register a definition of templated types.
    template<class T>
    void registerDefinition(const RtPrim& prim);

    /// Unregister a definition of templated types.
    template<class T>
    void unregisterDefinition(const RtToken& name);

    /// Return true if a definition of templates type
    /// with the given name has been registered.
    template<class T>
    bool hasDefinition(const RtToken& name) const;

    /// Return a registered definition prim by name.
    template<class T>
    RtPrim getDefinition(const RtToken& name) const;

    /// Return a number of registered definitions.
    template<class T>
    size_t numDefinitions() const;

    /// Return a registered definition prim by index.
    template<class T>
    RtPrim getDefinition(size_t index) const;

    /// @}
    /// @name Implementations
    /// @{

     /// Register an implementation of templated types.
    template<class T>
    void registerImplementation(const RtPrim& prim);

    /// Unregister an implementation of templated types.
    template<class T>
    void unregisterImplementation(const RtToken& name);

    /// Return true if an implementation of templates type
    /// with the given name has been registered.
    template<class T>
    bool hasImplementation(const RtToken& name) const;

    /// Return a registered implementation prim by name.
    template<class T>
    RtPrim getImplementation(const RtToken& name) const;

    /// Return a number of registered implementations.
    template<class T>
    size_t numImplementations() const;

    /// Return a registered implementation prim by index.
    template<class T>
    RtPrim getImplementation(size_t index) const;

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

    /// @}
    /// @name Library management
    /// @{

    /// Load a file or folder of files into a new library. The search paths previously set
    /// will be used to find the files if the given path is a relative path.
    /// All definitions and implementations found will be registered and available to use
    /// for content creation as long as the library remains loaded.
    RtStagePtr loadLibrary(const RtToken& name, const FilePath& path, const RtReadOptions* options = nullptr, bool forceReload = false);

    /// Unload a previously loaded library.
    /// All definitions and implementations in the library will be
    /// unregistered and no longer available for content creation.
    void unloadLibrary(const RtToken& name);

    /// Unload all previously loaded libraries.
    /// All definitions and implementations will be unregistered
    /// and no longer available for content creation.
    void unloadLibraries();

    /// Return the number of loaded libraries.
    size_t numLibraries() const;

    /// Return a library stage by name.
    RtStagePtr getLibrary(const RtToken& name) const;

    /// Return a library stage by index.
    RtStagePtr getLibrary(size_t index) const;

    /// @}
    /// @name Stage management
    /// @{

    /// Create a new empty stage.
    RtStagePtr createStage(const RtToken& name);

    /// Delete a stage.
    void deleteStage(const RtToken& name);

    /// Delete all stages.
    void deleteStages();

    /// Return the number of stages.
    size_t numStages() const;

    /// Return a stage by name.
    RtStagePtr getStage(const RtToken& name) const;

    /// Return a stage by index.
    RtStagePtr getStage(size_t index) const;

    /// Rename a stage.
    RtToken renameStage(const RtToken& name, const RtToken& newName);

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
