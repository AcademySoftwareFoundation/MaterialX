//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTAPI_H
#define MATERIALX_PVTAPI_H

/// @file
/// TODO: Docs

#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtStage.h>

#include <MaterialXRuntime/Private/PvtObject.h>
#include <MaterialXRuntime/Private/PvtPrim.h>
#include <MaterialXRuntime/Private/PvtCommand.h>
#include <MaterialXRuntime/Private/PvtMessage.h>

namespace MaterialX
{

class PvtApi
{
public:
    PvtApi()
    {
        reset();
    }

    void reset();

    PvtCommandEngine& getCommandEngine()
    {
        return _commandEngine;
    }

    PvtMessageHandler& getMessageHandler()
    {
        return _messageHandler;
    }

    void registerLogger(RtLoggerPtr logger)
    {
        _loggers.push_back(logger);
    }

    void unregisterLogger(RtLoggerPtr logger)
    {
        static_cast<void>(std::remove(_loggers.begin(), _loggers.end(), logger));
    }

    void log(RtLogger::MessageType type, const string& msg)
    {
        for (RtLoggerPtr logger : _loggers)
        {
            if (logger)
            {
                logger->log(type, msg);
            }
        }
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

    void createLibrary(const RtToken& name);

    void loadLibrary(const RtToken& name, const RtReadOptions& options);

    void unloadLibrary(const RtToken& name);

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

   const FilePath& getUserDefinitionPath()
    {
        return _userDefinitionPath;
    }

    void setUserDefinitionPath(const FilePath& path)
    {
        _userDefinitionPath = path;
    }

    RtToken makeUniqueName(const RtToken& name) const;

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

    UnitConverterRegistryPtr& getUnitDefinitions()
    {
        return _unitDefinitions;
    }

    static PvtApi* cast(RtApi& api)
    {
        return reinterpret_cast<PvtApi*>(api._ptr);
    }

    vector<RtLoggerPtr> _loggers;

    PvtCommandEngine _commandEngine;
    PvtMessageHandler _messageHandler;

    FileSearchPath _searchPaths;
    FileSearchPath _implementationSearchPaths;
    FileSearchPath _textureSearchPaths;
    FilePath _userDefinitionPath;
    RtStagePtr _libraryRoot;
    RtTokenMap<RtStagePtr> _libraries;
    UnitConverterRegistryPtr  _unitDefinitions;

    PvtDataHandle _masterPrimRoot;
    RtTokenMap<RtPrimCreateFunc> _createFunctions;
    RtTokenMap<RtStagePtr> _stages;
};

}

#endif
