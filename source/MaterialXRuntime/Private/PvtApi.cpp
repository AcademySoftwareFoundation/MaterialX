//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtApi.h>

#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtSchema.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtFileIo.h>

namespace MaterialX
{

namespace
{
    static const RtToken libRootName("api_lib_root");
}

void PvtApi::reset()
{
    _createFunctions.clear();
    _stages.clear();
    _nodedefs.clear();
    _nodeimpls.clear();

    _libraryRootStage.reset();
    _libraries.clear();
    _libraryRootStage = RtStage::createNew(libRootName);

    _unitDefinitions = UnitConverterRegistry::create();
}

void PvtApi::createLibrary(const RtToken& name)
{
    // If already loaded unload the old first,
    // to support reloading of updated libraries.
    if (getLibrary(name))
    {
        unloadLibrary(name);
    }

    RtStagePtr lib = RtStage::createNew(name);
    _libraries[name] = lib;

    _libraryRootStage->addReference(lib);
}

void PvtApi::loadLibrary(const RtToken& name, const RtReadOptions& options)
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
    file.readLibraries({ name.str() }, _searchPaths, options);

    _libraryRootStage->addReference(lib);
}

void PvtApi::unloadLibrary(const RtToken& name)
{
    RtStagePtr lib = getLibrary(name);
    if (lib)
    {
        // Unregister any nodedefs from this library.
        RtSchemaPredicate<RtNodeDef> nodedefFilter;
        for (RtPrim nodedef : lib->getRootPrim().getChildren(nodedefFilter))
        {
            unregisterNodeDef(nodedef.getName());
        }

        // Delete the library.
        _libraries.erase(name);
    }
}

RtToken PvtApi::makeUniqueStageName(const RtToken& name) const
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
            newName = RtToken(baseName + std::to_string(i++));
            otherStage = getStage(newName);
        } while (otherStage);
    }

    return newName;
}


}
