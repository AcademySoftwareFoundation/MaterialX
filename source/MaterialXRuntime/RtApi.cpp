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

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

class PvtApi
{
public:
    PvtApi()
    {
        reset();
    }

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
        _masterPrimRoot->asA<PvtPrim>()->addChildPrim(PvtObject::ptr<PvtPrim>(prim));
    }

    void unregisterMasterPrim(const RtToken& name)
    {
        PvtPrim* prim = _masterPrimRoot->asA<PvtPrim>()->getChild(name);
        if (prim)
        {
            _masterPrimRoot->asA<PvtPrim>()->removeChildPrim(prim);
        }
    }

    bool hasMasterPrim(const RtToken& name)
    {
        PvtPrim* prim = _masterPrimRoot->asA<PvtPrim>()->getChild(name);
        return prim != nullptr;
    }

    RtPrim getMasterPrim(const RtToken& name)
    {
        PvtPrim* prim = _masterPrimRoot->asA<PvtPrim>()->getChild(name);
        return prim ? prim->hnd() : RtPrim();
    }

    RtPrimIterator getMasterPrims(RtObjectPredicate predicate = nullptr)
    {
        return RtPrimIterator(_masterPrimRoot, predicate);
    }

    void clearSearchPath()
    {
        _searchPaths.clear();
    }

    void clearTextureSearchPath()
    {
        _textureSearchPaths.clear();
    }

    void clearImplementationSearchPath()
    {
        _implementationSearchPaths.clear();
    }


    void setSearchPath(const FileSearchPath& searchPath)
    {
        _searchPaths.append(searchPath);
    }

    void setTextureSearchPath(const FileSearchPath& searchPath)
    {
        _textureSearchPaths.append(searchPath);
    }

    void setImplementationSearchPath(const FileSearchPath& searchPath)
    {
        _implementationSearchPaths.append(searchPath);
    }

    const FileSearchPath& getSearchPath() const
    {
        return _searchPaths;
    }

    const FileSearchPath& getTextureSearchPath() const
    {
        return _textureSearchPaths;
    }

    const FileSearchPath& getImplementationSearchPath() const
    {
        return _implementationSearchPaths;
    }

    void loadLibrary(const RtToken& name)
    {
        // If already loaded unload the old first,
        // to support reloading of updated libraries.
        if (getLibrary(name))
        {
            unloadLibrary(name);
        }

        RtStagePtr lib = RtStage::createNew(name);
        _libraries[name] = lib;

        RtFileIo file(lib);
        file.readLibraries({ name }, _searchPaths);

        _libraryRoot->addReference(lib);
    }

    void unloadLibrary(const RtToken& name)
    {
        RtStagePtr lib = getLibrary(name);
        if (lib)
        {
            // Unregister any nodedefs from this library.
            RtSchemaPredicate<RtNodeDef> nodedefFilter;
            for (RtPrim nodedef : lib->getRootPrim().getChildren(nodedefFilter))
            {
                unregisterMasterPrim(nodedef.getName());
            }

            // Delete the library.
            _libraries.erase(name);
        }
    }

    RtStagePtr getLibrary(const RtToken& name)
    {
        auto it = _libraries.find(name);
        return it != _libraries.end() ? it->second : nullptr;
    }

    RtStagePtr getLibraryRoot()
    {
        return _libraryRoot;
    }

    RtTokenVec getLibraryNames() const
    {
        RtTokenVec names;
        for (auto it : _libraries)
        {
            names.push_back(it.first);
        }
        return names;
    }

    RtToken makeUniqueName(const RtToken& name) const
    {
        RtToken newName = name;

        // Check if there is another stage with this name.
        RtStagePtr otherStage = getStage(name);
        if (otherStage)
        {
            // Find a number to append to the name, incrementing
            // the counter until a unique name is found.
            string baseName = name.str();
            int i = 1;
            const size_t n = name.str().find_last_not_of("0123456789") + 1;
            if (n < name.str().size())
            {
                const string number = name.str().substr(n);
                i = std::stoi(number) + 1;
                baseName = baseName.substr(0, n);
            }
            // Iterate until there is no other stage with the resulting name.
            do {
                newName = baseName + std::to_string(i++);
                otherStage = getStage(newName);
            } while (otherStage);
        }

        return newName;
    }

    RtStagePtr createStage(const RtToken& name)
    {
        const RtToken newName = makeUniqueName(name);
        RtStagePtr stage = RtStage::createNew(newName);
        _stages[newName] = stage;
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

    RtToken renameStage(const RtToken& name, const RtToken& newName)
    {
        RtStagePtr stage = getStage(name);
        if (!stage)
        {
            throw ExceptionRuntimeError("Can't find a stage named '" + name.str() + "' to rename");
        }
        const RtToken uniqueName = makeUniqueName(newName);
        stage->setName(uniqueName);
        _stages[uniqueName] = stage;
        _stages.erase(name);
        return uniqueName;
    }

    RtTokenVec getStageNames() const
    {
        RtTokenVec names;
        for (auto it : _stages)
        {
            names.push_back(it.first);
        }
        return names;
    }

    void reset()
    {
        static const RtTypeInfo masterPrimRootType("api_masterprimroot");
        static const RtToken libRootName("api_libroot");

        _masterPrimRoot.reset(new PvtPrim(&masterPrimRootType, masterPrimRootType.getShortTypeName(), nullptr));
        _createFunctions.clear();
        _stages.clear();

        _libraryRoot.reset();
        _libraries.clear();
        _libraryRoot = RtStage::createNew(libRootName);
    }

    FileSearchPath _searchPaths;
    FileSearchPath _implementationSearchPaths;
    FileSearchPath _textureSearchPaths;
    RtStagePtr _libraryRoot;
    RtTokenMap<RtStagePtr> _libraries;

    PvtDataHandle _masterPrimRoot;
    RtTokenMap<RtPrimCreateFunc> _createFunctions;
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
    _cast(_ptr)->reset();

    // Register built in schemas
    registerTypedSchema<RtGeneric>();
    registerTypedSchema<RtNode>();
    registerTypedSchema<RtNodeDef>();
    registerTypedSchema<RtNodeGraph>();
    registerTypedSchema<RtBackdrop>();
    registerTypedSchema<RtLookGroup>();
    registerTypedSchema<RtLook>();
    registerTypedSchema<RtMaterialAssign>();
    registerTypedSchema<RtCollection>();
}

void RtApi::shutdown()
{
    _cast(_ptr)->reset();
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

RtPrimIterator RtApi::getMasterPrims(RtObjectPredicate predicate)
{
    return RtPrimIterator(_cast(_ptr)->_masterPrimRoot, predicate);
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

void RtApi::loadLibrary(const RtToken& name)
{
    _cast(_ptr)->loadLibrary(name);
}

void RtApi::unloadLibrary(const RtToken& name)
{
    _cast(_ptr)->unloadLibrary(name);
}

RtTokenVec RtApi::getLibraryNames() const
{
    return _cast(_ptr)->getLibraryNames();
}

RtStagePtr RtApi::getLibrary()
{
    return _cast(_ptr)->getLibraryRoot();
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

RtToken RtApi::renameStage(const RtToken& name, const RtToken& newName)
{
    return _cast(_ptr)->renameStage(name, newName);
}

RtTokenVec RtApi::getStageNames() const
{
    return _cast(_ptr)->getStageNames();
}

RtApi& RtApi::get()
{
    static RtApi _instance;
    return _instance;
}

}
