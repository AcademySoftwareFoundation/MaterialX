//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNodeDef.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    static const RtToken NODEDEF("nodedef");
}

DEFINE_TYPED_SCHEMA(RtNode, "node");

RtPrim RtNode::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    const RtPrim master = RtApi::get().getMasterPrim(typeName);
    if (!master)
    {
        throw ExceptionRuntimeError("No master prim registered for typeName '" + typeName.str() + "'");
    }

    // Make sure this is a valid nodedef.
    RtNodeDef nodedefSchema(master);
    if (!nodedefSchema)
    {
        throw ExceptionRuntimeError("Master prim with typeName '" + typeName.str() + "' is not a valid nodedef");
    }

    const PvtPrim* nodedef = PvtObject::ptr<PvtPrim>(master);

    const RtToken nodeName = (name == EMPTY_TOKEN ?
        RtToken(nodedefSchema.getNode().str() + "1") : name);

    PvtDataHandle nodeH = PvtPrim::createNew(&_typeInfo, nodeName, PvtObject::ptr<PvtPrim>(parent));
    PvtPrim* node = nodeH->asA<PvtPrim>();

    // Save the nodedef in a relationship.
    PvtRelationship* nodedefRelation = node->createRelationship(NODEDEF);
    nodedefRelation->addTarget(nodedef);

    // Create the interface according to nodedef.
    for (const PvtDataHandle& attrH : nodedef->getAllAttributes())
    {
        const PvtAttribute* attr = attrH->asA<PvtAttribute>();
        if (attr->isA<PvtInput>())
        {
            PvtInput* input = node->createInput(attr->getName(), attr->getType(), attr->getFlags());
            RtValue::copy(attr->getType(), attr->getValue(), input->getValue());
        }
        else if (attr->isA<PvtObject>())
        {
            PvtOutput* output = node->createOutput(attr->getName(), attr->getType(), attr->getFlags());
            RtValue::copy(attr->getType(), attr->getValue(), output->getValue());
        }
    }

    return nodeH;
}

RtPrim RtNode::getNodeDef() const
{
    PvtRelationship* nodedef = prim()->getRelationship(NODEDEF);
    return nodedef && nodedef->hasTargets() ? nodedef->getAllTargets()[0] : RtPrim();
}

void RtNode::setNodeDef(RtPrim nodeDef)
{
    PvtRelationship* nodedefRel = prim()->getRelationship(NODEDEF);
    if (!nodedefRel)
    {
        nodedefRel = prim()->createRelationship(NODEDEF);
    }
    else
    {
        nodedefRel->clearTargets();
    }
    nodedefRel->addTarget(PvtObject::ptr<PvtPrim>(nodeDef));
}

size_t RtNode::numInputs() const
{
    return prim()->numInputs();
}

RtInput RtNode::getInput(const RtToken& name) const
{
    PvtInput* input = prim()->getInput(name);
    return input ? input->hnd() : RtInput();
}

RtAttrIterator RtNode::getInputs() const
{
    RtObjTypePredicate<RtInput> filter;
    return RtAttrIterator(getPrim(), filter);
}

size_t RtNode::numOutputs() const
{
    return prim()->numOutputs();
}

RtOutput RtNode::getOutput(const RtToken& name) const
{
    PvtOutput* output = prim()->getOutput(name);
    return output ? output->hnd() : RtOutput();
}

RtOutput RtNode::getOutput() const
{
    PvtOutput* output = prim()->getOutput();
    return output ? output->hnd() : RtOutput();
}

RtAttrIterator RtNode::getOutputs() const
{
    RtObjTypePredicate<RtOutput> filter;
    return RtAttrIterator(getPrim(), filter);
}

}
