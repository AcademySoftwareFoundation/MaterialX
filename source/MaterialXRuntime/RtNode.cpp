//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/Tokens.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    // TODO: We should derive this from a data driven XML schema.
    class PvtNodePrimSpec : public PvtPrimSpec
    {
    public:
        PvtNodePrimSpec()
        {
            addPrimAttribute(Tokens::DOC, RtType::STRING);
            addPrimAttribute(Tokens::XPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::YPOS, RtType::FLOAT);
            addPrimAttribute(Tokens::WIDTH, RtType::INTEGER);
            addPrimAttribute(Tokens::HEIGHT, RtType::INTEGER);
            addPrimAttribute(Tokens::UICOLOR, RtType::COLOR3);
            addPrimAttribute(Tokens::UINAME, RtType::STRING);
            addPrimAttribute(Tokens::VERSION, RtType::TOKEN);
            addPrimAttribute(Tokens::COLORSPACE, RtType::TOKEN);
            addPrimAttribute(Tokens::FILEPREFIX, RtType::STRING);

            addInputAttribute(Tokens::DOC, RtType::STRING);
            addInputAttribute(Tokens::MEMBER, RtType::STRING);
            addInputAttribute(Tokens::CHANNELS, RtType::STRING);
            addInputAttribute(Tokens::UIADVANCED, RtType::BOOLEAN);
            addInputAttribute(Tokens::UIVISIBLE, RtType::BOOLEAN);

            addInputAttributeByType(RtType::COLOR3, Tokens::COLORSPACE, RtType::TOKEN);
            addInputAttributeByType(RtType::COLOR4, Tokens::COLORSPACE, RtType::TOKEN);

            addInputAttributeByType(RtType::FLOAT, Tokens::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::FLOAT, Tokens::UNITTYPE, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR2, Tokens::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR2, Tokens::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR2, Tokens::DEFAULTGEOMPROP, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR3, Tokens::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR3, Tokens::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR3, Tokens::DEFAULTGEOMPROP, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR4, Tokens::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR4, Tokens::UNITTYPE, RtType::TOKEN);

            addInputAttributeByType(RtType::FILENAME, Tokens::COLORSPACE, RtType::TOKEN);
            addInputAttributeByType(RtType::FILENAME, Tokens::FILEPREFIX, RtType::STRING);

            addOutputAttribute(Tokens::DOC, RtType::STRING);
            addOutputAttribute(Tokens::MEMBER, RtType::STRING);
            addOutputAttribute(Tokens::WIDTH, RtType::INTEGER);
            addOutputAttribute(Tokens::HEIGHT, RtType::INTEGER);
            addOutputAttribute(Tokens::BITDEPTH, RtType::INTEGER);

            addOutputAttributeByType(RtType::COLOR3, Tokens::COLORSPACE, RtType::TOKEN);
            addOutputAttributeByType(RtType::COLOR4, Tokens::COLORSPACE, RtType::TOKEN);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNode, "node");

RtPrim RtNode::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    RtPrim nodedef = RtApi::get().getDefinition<RtNodeDef>(typeName);
    if (!nodedef)
    {
        throw ExceptionRuntimeError("No nodedef registered with name '" + typeName.str() + "'");
    }
    return createNode(nodedef, name, parent);
}

RtPrim RtNode::createNode(RtPrim nodedef, const RtToken& name, RtPrim parent)
{
    // Make sure this is a valid nodedef.
    RtNodeDef nodedefSchema(nodedef);
    if (!nodedef)
    {
        throw ExceptionRuntimeError("Given nodedef with name '" + nodedef.getName().str() + "' is not valid");
    }

    PvtPrim* nodedefPrim = PvtObject::cast<PvtPrim>(nodedef);

    const RtToken nodeName = name == EMPTY_TOKEN ? nodedefSchema.getNode() : name;
    PvtObjHandle nodeH = PvtPrim::createNew(&_typeInfo, nodeName, PvtObject::cast<PvtPrim>(parent));
    PvtPrim* node = nodeH->asA<PvtPrim>();

    // Save the nodedef in a relationship.
    PvtRelationship* nodedefRelation = node->createRelationship(Tokens::NODEDEF);
    nodedefRelation->connect(nodedefPrim);

    // Copy version tag if used.
    const RtToken& version = nodedefSchema.getVersion();
    if (version != EMPTY_TOKEN)
    {
        RtTypedValue* attr = node->createAttribute(Tokens::VERSION, RtType::TOKEN);
        attr->asToken() = version;
    }

    // Create the interface according to nodedef.
    for (PvtObject* obj : nodedefPrim->getInputs())
    {
        const PvtInput* port = obj->asA<PvtInput>();
        PvtInput* input = node->createInput(port->getName(), port->getType(), port->getFlags());
        RtValue::copy(port->getType(), port->getValue(), input->getValue());
    }
    for (PvtObject* obj : nodedefPrim->getOutputs())
    {
        const PvtOutput* port = obj->asA<PvtOutput>();
        PvtOutput* output = node->createOutput(port->getName(), port->getType(), port->getFlags());
        RtValue::copy(port->getType(), port->getValue(), output->getValue());
    }

    return nodeH;
}

const RtPrimSpec& RtNode::getPrimSpec() const
{
    static const PvtNodePrimSpec s_primSpec;
    return s_primSpec;
}

void RtNode::setNodeDef(RtPrim nodeDef)
{
    PvtRelationship* nodedefRel = prim()->getRelationship(Tokens::NODEDEF);
    if (!nodedefRel)
    {
        nodedefRel = prim()->createRelationship(Tokens::NODEDEF);
    }
    else
    {
        nodedefRel->clearConnections();
    }
    nodedefRel->connect(PvtObject::cast(nodeDef));
}

RtPrim RtNode::getNodeDef() const
{
    PvtRelationship* nodedefRel = prim()->getRelationship(Tokens::NODEDEF);
    return nodedefRel && nodedefRel->hasConnections() ? nodedefRel->getConnection() : RtPrim();
}

void RtNode::setVersion(const RtToken& version)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::VERSION, RtType::TOKEN);
    attr->asToken() = version;
}

const RtToken& RtNode::getVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::VERSION, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

}
