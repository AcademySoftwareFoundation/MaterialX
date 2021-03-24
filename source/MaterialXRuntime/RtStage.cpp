//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtApi.h>

#include <MaterialXRuntime/Private/PvtStage.h>
#include <MaterialXRuntime/Private/PvtApi.h>

namespace MaterialX
{

namespace
{
    // Syntactic sugar
    inline PvtStage* _cast(void* ptr)
    {
        return static_cast<PvtStage*>(ptr);
    }
}

RtStage::RtStage() :
    _ptr(nullptr)
{
}

RtStage::~RtStage()
{
    delete _cast(_ptr);
}

const RtToken& RtStage::getName() const
{
    return _cast(_ptr)->getName();
}

void RtStage::addSourceUri(const FilePath& uri)
{
    return _cast(_ptr)->addSourceUri(uri);
}

const FilePathVec& RtStage::getSourceUri() const
{
    return _cast(_ptr)->getSourceUri();
}

RtPrim RtStage::createPrim(const RtToken& typeName)
{
    return createPrim(RtPath("/"), EMPTY_TOKEN, typeName);
}

RtPrim RtStage::createPrim(const RtPath& path, const RtToken& typeName)
{
    PvtPrim* prim = _cast(_ptr)->createPrim(*static_cast<PvtPath*>(path._ptr), typeName);
    return prim->hnd();
}

RtPrim RtStage::createPrim(const RtPath& parentPath, const RtToken& name, const RtToken& typeName)
{
    PvtPrim* prim = _cast(_ptr)->createPrim(*static_cast<PvtPath*>(parentPath._ptr), name, typeName);
    return prim->hnd();
}

void RtStage::removePrim(const RtPath& path)
{
    _cast(_ptr)->removePrim(*static_cast<PvtPath*>(path._ptr));
}

RtToken RtStage::renamePrim(const RtPath& path, const RtToken& newName)
{
    return _cast(_ptr)->renamePrim(*static_cast<PvtPath*>(path._ptr), newName);
}

RtToken RtStage::reparentPrim(const RtPath& path, const RtPath& newParentPath)
{
    return _cast(_ptr)->reparentPrim(
        *static_cast<PvtPath*>(path._ptr),
        *static_cast<PvtPath*>(newParentPath._ptr)
    );
}

RtPrim RtStage::getPrimAtPath(const RtPath& path)
{
    PvtPrim* prim = _cast(_ptr)->getPrimAtPath(*static_cast<PvtPath*>(path._ptr));
    return prim ? prim->hnd() : RtPrim();
}

RtPrim RtStage::getRootPrim()
{
    return _cast(_ptr)->getRootPrim()->hnd();
}

RtStageIterator RtStage::traverse(RtObjectPredicate predicate)
{
    return RtStageIterator(shared_from_this(), predicate);
}

void RtStage::addReference(RtStagePtr stage)
{
    _cast(_ptr)->addReference(stage);
}

RtStagePtr RtStage::getReference(const RtToken& name) const
{
    return _cast(_ptr)->getReference(name);
}

void RtStage::removeReference(const RtToken& name)
{
    _cast(_ptr)->removeReference(name);
}

void RtStage::removeReferences()
{
    _cast(_ptr)->removeReferences();
}

void RtStage::setName(const RtToken& name)
{
    _cast(_ptr)->setName(name);
}

void RtStage::disposePrim(const RtPath& path)
{
    _cast(_ptr)->disposePrim(*static_cast<PvtPath*>(path._ptr));
}

void RtStage::restorePrim(const RtPath& parentPath, const RtPrim& prim)
{
    _cast(_ptr)->restorePrim(*static_cast<PvtPath*>(parentPath._ptr), prim);
}

RtPrim RtStage::createNodeDef(RtPrim nodegraphPrim,
                              const RtToken& nodeDefName, 
                              const RtToken& nodeName, 
                              const RtToken& version,
                              bool isDefaultVersion,
                              const RtToken& nodeGroup,
                              const RtToken& namespaceString)
{
    // Must have a nodedef name and a node name
    if (nodeDefName == EMPTY_TOKEN || nodeName == EMPTY_TOKEN)
    {
        throw ExceptionRuntimeError("Cannot create nodedef '" + nodeDefName.str() + "', with node name: '" + nodeName.str() + "'");
    }

    // Always used qualified namespace
    const bool isNameSpaced = namespaceString != EMPTY_TOKEN;
    const RtToken qualifiedNodeDefName = isNameSpaced ? RtToken(namespaceString.str() + MaterialX::NAME_PREFIX_SEPARATOR + nodeDefName.str()) : nodeDefName;

    PvtStage* stage = PvtStage::cast(this);

    // Make sure the nodedef name is unique among all prims in the stage.
    PvtPath path(nodeDefName);
    if (stage->getPrimAtPath(path))
    {
        throw ExceptionRuntimeError("The nodedef name '" + qualifiedNodeDefName.str() + "' is not unique");
    }

    PvtPrim* nodedefPrim = stage->createPrim(stage->getPath(), qualifiedNodeDefName, RtNodeDef::typeName());
    RtNodeDef nodedef(nodedefPrim->hnd());

    // Set node, version and optional node group
    nodedef.setNode(nodeName);
    if (version != EMPTY_TOKEN)
    {
        nodedef.setVersion(version);

        // If a version is specified, set if it is the default version
        if (isDefaultVersion)
        {
            nodedef.setIsDefaultVersion(true);
        }
    }
    if (nodeGroup != EMPTY_TOKEN)
    {
        nodedef.setNodeGroup(nodeGroup);
    }

    RtNodeGraph nodegraph(nodegraphPrim);

    // Add an input per nodegraph input
    for (RtInput input : nodegraph.getInputs())
    {
        RtInput nodedefInput = nodedef.createInput(input.getName(), input.getType());
        nodedefInput.setUniform(input.isUniform());
        RtValue::copy(input.getType(), input.getValue(), nodedefInput.getValue());
    }

    // TODO : Add support for tokens

    // Add an output per nodegraph output
    for (RtOutput output : nodegraph.getOutputs())
    {
        RtOutput nodedefOutput = nodedef.createOutput(output.getName(), output.getType());
        RtValue::copy(output.getType(), output.getValue(), nodedefOutput.getValue());
    }

    // Set namespace for the nodegraph and nodedef
    if (isNameSpaced)
    {
        nodedef.setNamespace(namespaceString);
        nodegraph.setNamespace(namespaceString);
    }

    // Set the definition on the nodegraph
    // turning this into a functional graph
    nodegraph.setDefinition(qualifiedNodeDefName);

    // Create the relationship between nodedef and it's implementation.
    nodedef.getNodeImpls().connect(nodegraph.getPrim());

    PvtApi* api = PvtApi::cast(RtApi::get());
    api->registerNodeDef(nodedef.getPrim());
    api->registerNodeGraph(nodegraph.getPrim());

    return nodedef.getPrim();
}

}
