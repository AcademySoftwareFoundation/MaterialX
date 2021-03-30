//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtApi.h>
#include <MaterialXRuntime/Private/PvtObject.h>

#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtSchema.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtTargetDef.h>
#include <MaterialXRuntime/RtFileIo.h>

namespace MaterialX
{

namespace
{
    const RtIdentifier DEFAULT_LIBRARY_NAME("default");
    const string NUMBERS("0123456789");
}

void PvtApi::reset()
{
    _createFunctions.clear();
    _libraries.clear();
    _librariesOrder.clear();
    _stages.clear();
    _stagesOrder.clear();

    _unitDefinitions = UnitConverterRegistry::create();
}

RtStagePtr PvtApi::loadLibrary(const RtIdentifier& name, const FilePath& path, const RtReadOptions* options, bool forceReload)
{
    auto it = _libraries.find(name);
    if (it != _libraries.end())
    {
        if (forceReload)
        {
            unloadLibrary(name);
        }
        else
        {
            throw ExceptionRuntimeError("A library named '" + name.str() + "' has already been loaded");
        }
    }

    RtStagePtr stage = PvtStage::createNew(name);
    _libraries[name] = stage;
    _librariesOrder.push_back(stage);

    // Load in the files(s).
    RtFileIo fileApi(stage);
    StringSet loadedFiles = fileApi.readLibrary(path, _searchPaths, options);
    for (const string& file : loadedFiles)
    {
        stage->addSourceUri(file);
    }

    // Register any definitions and implementations.
    registerPrims(stage);

    // Reset nodeimpl relationsships since the registry of
    // definitions and implementations have changed.
    setupNodeImplRelationships();

    return stage;
}

void PvtApi::unloadLibrary(const RtIdentifier& name)
{
    auto it = _libraries.find(name);
    if (it != _libraries.end())
    {
        RtStagePtr stage = it->second;
        unregisterPrims(stage);

        _libraries.erase(it);
        _librariesOrder.erase(std::find(_librariesOrder.begin(), _librariesOrder.end(), stage));

        // Reset nodeimpl relationsships since the registry of
        // definitions and implementations have changed.
        setupNodeImplRelationships();
    }
}

void PvtApi::unloadLibraries()
{
    for (auto stage : _librariesOrder)
    {
        unregisterPrims(stage);
    }
    _libraries.clear();
    _librariesOrder.clear();
}

void PvtApi::registerPrims(RtStagePtr stage)
{
    for (RtPrim prim : stage->traverse())
    {
        if (prim.hasApi<RtNodeDef>())
        {
            registerNodeDef(prim);
        }
        else if (prim.hasApi<RtNodeGraph>())
        {
            RtNodeGraph nodegraph(prim);
            if (nodegraph.getDefinition() != EMPTY_IDENTIFIER)
            {
                registerNodeGraph(prim);
            }
        }
        else if (prim.hasApi<RtNodeImpl>())
        {
            registerNodeImpl(prim);
        }
        else if (prim.hasApi<RtTargetDef>())
        {
            registerTargetDef(prim);
        }
    }
}

void PvtApi::unregisterPrims(RtStagePtr stage)
{
    for (RtPrim prim : stage->traverse())
    {
        if (prim.hasApi<RtNodeDef>())
        {
            unregisterNodeDef(prim.getName());
        }
        else if (prim.hasApi<RtNodeGraph>())
        {
            RtNodeGraph nodegraph(prim);
            if (nodegraph.getDefinition() != EMPTY_IDENTIFIER)
            {
                unregisterNodeGraph(prim.getName());
            }
        }
        else if (prim.hasApi<RtNodeImpl>())
        {
            unregisterNodeImpl(prim.getName());
        }
        else if (prim.hasApi<RtTargetDef>())
        {
            unregisterTargetDef(prim.getName());
        }
    }
}

void PvtApi::setupNodeImplRelationships()
{
    // Break old relationsships.
    for (PvtObject* obj : _nodedefs.vec())
    {
        RtNodeDef nodedef(obj->hnd());
        nodedef.getNodeImpls().clearConnections();
    }

    // Make new relationsships.
    for (PvtObject* obj : _nodeimpls.vec())
    {
        RtNodeImpl nodeimpl(obj->hnd());
        PvtObject* nodedefObj  = _nodedefs.find(nodeimpl.getNodeDef());
        if (nodedefObj)
        {
            RtNodeDef nodedef(nodedefObj->hnd());
            nodedef.getNodeImpls().connect(nodeimpl.getPrim());
        }
    }
    for (PvtObject* obj : _nodegraphs.vec())
    {
        RtNodeGraph nodegraph(obj->hnd());
        PvtObject* nodedefObj = _nodedefs.find(nodegraph.getDefinition());
        if (nodedefObj)
        {
            RtNodeDef nodedef(nodedefObj->hnd());
            nodedef.getNodeImpls().connect(nodegraph.getPrim());
        }
    }
}

RtIdentifier PvtApi::makeUniqueStageName(const RtIdentifier& name) const
{
    RtIdentifier newName = name;

    // Check if there is another stage with this name.
    RtStagePtr otherStage = getStage(name);
    if (otherStage)
    {
        // Find a number to append to the name, incrementing
        // the counter until a unique name is found.
        string baseName = name.str();
        int i = 1;
        const size_t n = name.str().find_last_not_of(NUMBERS) + 1;
        if (n < name.str().size())
        {
            const string number = name.str().substr(n);
            i = std::stoi(number) + 1;
            baseName = baseName.substr(0, n);
        }
        // Iterate until there is no other stage with the resulting name.
        do {
            newName = RtIdentifier(baseName + std::to_string(i++));
            otherStage = getStage(newName);
        } while (otherStage);
    }

    return newName;
}

}
