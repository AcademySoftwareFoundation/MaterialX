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

#include <MaterialXFormat/File.h>

namespace MaterialX
{

/// Function type for creating prims for a typed schema.
using RtPrimCreateFunc = std::function<RtPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)>;

class RtApi
{
public:
    ~RtApi();

    /// Initialize the API session.
    void initialize();

    /// Shutdown the API session.
    void shutdown();

    /// Register a create function for a typename.
    void registerCreateFunction(const RtToken& typeName, RtPrimCreateFunc func);

    /// Unregister a create function.
    void unregisterCreateFunction(const RtToken& typeName);

    /// Return true if the given typename has a create function registered.
    bool hasCreateFunction(const RtToken& typeName);

    /// Return the create function for given typename.
    /// Or nullptr if no such create function has been registered.
    RtPrimCreateFunc getCreateFunction(const RtToken& typeName);

    /// Register a master prim to be used for creating instances from.
    /// A typical usecase is for registering a nodedef prim to be used for
    /// creating node instances.
    void registerMasterPrim(const RtPrim& prim);

    /// Unregister a master prim.
    void unregisterMasterPrim(const RtToken& name);

    /// Return true if a master prim with the given name has been registered.
    bool hasMasterPrim(const RtToken& name);

    /// Return the master prim with given name.
    /// Or a null object if no such prim has been registered.
    RtPrim getMasterPrim(const RtToken& name);

    /// Return and iterator over all registered master prims,
    /// optionally filtered using a predicate.
    RtPrimIterator getMasterPrims(RtObjectPredicate predicate = nullptr);

    /// Register a typed prim schema.
    template<class T>
    void registerTypedSchema()
    {
        registerCreateFunction(T::typeName(), T::createPrim);
    }

    /// Unregister a typed prim schema.
    template<class T>
    void unregisterTypedSchema()
    {
        unregisterCreateFunction(T::typeName());
    }

    /// Set search path for libraries. Can be called multiple times
    /// to append to the current search path.
    void setSearchPath(const FileSearchPath& searchPath);

    /// Load a library.
    void loadLibrary(const RtToken& name);

    /// Unload a library.
    void unloadLibrary(const RtToken& name);

    /// Return a list of all loaded libraries.
    RtTokenVec getLibraryNames() const;

    /// Return the library stage containing all loaded libraries.
    RtStagePtr getLibrary();

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

    /// Get the singleton API instance.
    static RtApi& get();

    RtApi(const RtApi&) = delete;
    const RtApi& operator=(const RtApi&) = delete;

protected:
    RtApi();
    void* _ptr;
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
};

}

#endif
