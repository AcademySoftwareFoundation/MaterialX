//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtPath.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeGraph.h>

#include <MaterialXRuntime/Private/PvtStage.h>

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

RtStagePtr RtStage::createNew(const RtToken& name)
{
    // Create the shared stage object.
    RtStagePtr stage(new RtStage());

    // Create the private stage implementation.
    stage->_ptr = new PvtStage(name, RtStageWeakPtr(stage));

    // Return the shared wrapper object.
    return stage;
}

const RtToken& RtStage::getName() const
{
    return _cast(_ptr)->getName();
}

const RtTokenVec& RtStage::getSourceUri() const
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

RtPrim RtStage::getImplementation(const RtNodeDef& definition) const
{
    const RtToken& nodeDefName = definition.getName();

    RtSchemaPredicate<RtNodeGraph> filter;
    for (RtPrim child : _cast(_ptr)->getRootPrim()->getChildren(filter))
    {
        RtNodeGraph nodeGraph(child);
        // Check if there is a definition name match and
        // if the versions are compatible.
        if (nodeGraph.getDefinition() == nodeDefName &&
            definition.isVersionCompatible(nodeGraph.getVersion()))
        {
            PvtPrim* graphPrim = PvtObject::ptr<PvtPrim>(child);
            return RtPrim(graphPrim->hnd());
        }
    }

    // TODO: Return an empty prim for now. When support is added in to be able to
    // access non-nodegraph implementations, this method should throw an exception if not found.
    return RtPrim();
}

RtPrim RtStage::createNodeDef(RtNodeGraph& nodeGraph, 
                              const RtToken& nodeDefName, 
                              const RtToken& nodeName, 
                              const RtToken& version,
                              bool isDefaultVersion,
                              const RtToken& nodeGroup)
{
    // Must have a nodedef name and a node name
    if (nodeDefName == EMPTY_TOKEN ||
        nodeName == EMPTY_TOKEN)
    {
        throw ExceptionRuntimeError("Cannot create nodedef with definition name'" + nodeDefName.str() 
                                    + "', and node name: '" + nodeName.str() + "'");
    }

    PvtStage* stage = _cast(_ptr);
    PvtPrim* prim = stage->createPrim(stage->getPath(), nodeDefName, RtNodeDef::typeName());

    RtNodeDef nodedef(prim->hnd());
    if (nodedef.isMasterPrim())
    {
        throw ExceptionRuntimeError("Definition to create already exists '" + nodeDefName.str() + "'");
    }

    // Set node, version and optional node group
    nodedef.setNode(nodeName);
    if (version != EMPTY_TOKEN)
    {
        nodedef.setVersion(version);
        nodeGraph.setVersion(version);

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

    // Add an input per nodegraph input
    for (auto input : nodeGraph.getInputs())
    {
        RtInput attr = nodedef.createInput(input.getName(), input.getType());
        attr.setUniform(input.asA<RtInput>().isUniform());

        const PvtObject* obj = PvtObject::hnd(input)->asA<PvtObject>();
        const vector<RtToken>& metadataNames = obj->getMetadataOrder();
        for (auto metadataName : metadataNames)
        {
            const RtTypedValue* metadataValue = obj->getMetadata(metadataName);
            if (metadataValue)
            {
                RtTypedValue* vNew = attr.addMetadata(metadataName, metadataValue->getType());
                vNew->getValue().asToken() = metadataValue->getValue().asToken();
            }
        }
        if (!input.getValueString().empty())
        {
            attr.setValue(input.getValue());
        }
    }

    // Add an output per nodegraph output
    for (auto output : nodeGraph.getOutputs())
    {
        RtAttribute attr = nodedef.createOutput(output.getName(), output.getType());
        const PvtObject* obj = PvtObject::hnd(output)->asA<PvtObject>();
        const vector<RtToken>& metadataNames = obj->getMetadataOrder();
        for (auto metadataName : metadataNames)
        {
            const RtTypedValue* metadataValue = obj->getMetadata(metadataName);
            if (metadataValue)
            {
                RtTypedValue* vNew = attr.addMetadata(metadataName, metadataValue->getType());
                vNew->getValue().asToken() = metadataValue->getValue().asToken();
            }
        }
        if (!output.getValueString().empty())
        {
            attr.setValue(output.getValue());
        }
    }

    // Set the definition on the nodegraph
    nodeGraph.setDefinition(nodeDefName);

    // Add definiion
    nodedef.registerMasterPrim();

    return prim->hnd();
}

}
