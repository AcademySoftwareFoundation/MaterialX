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
    // TODO: We should derive this from a data driven XML schema.
    class PvtNodePrimSpec : public PvtPrimSpec
    {
    public:
        PvtNodePrimSpec()
        {
            addPrimAttribute(RtString::DOC, RtType::STRING);
            addPrimAttribute(RtString::XPOS, RtType::FLOAT);
            addPrimAttribute(RtString::YPOS, RtType::FLOAT);
            addPrimAttribute(RtString::WIDTH, RtType::INTEGER);
            addPrimAttribute(RtString::HEIGHT, RtType::INTEGER);
            addPrimAttribute(RtString::UICOLOR, RtType::COLOR3);
            addPrimAttribute(RtString::UINAME, RtType::STRING);
            addPrimAttribute(RtString::VERSION, RtType::INTERNSTRING);
            addPrimAttribute(RtString::COLORSPACE, RtType::INTERNSTRING);
            addPrimAttribute(RtString::FILEPREFIX, RtType::STRING);

            addInputAttribute(RtString::DOC, RtType::STRING);
            addInputAttribute(RtString::MEMBER, RtType::STRING);
            addInputAttribute(RtString::CHANNELS, RtType::STRING);
            addInputAttribute(RtString::UIADVANCED, RtType::BOOLEAN);
            addInputAttribute(RtString::UIVISIBLE, RtType::BOOLEAN);

            addInputAttributeByType(RtType::COLOR3, RtString::COLORSPACE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::COLOR4, RtString::COLORSPACE, RtType::INTERNSTRING);

            addInputAttributeByType(RtType::FLOAT, RtString::UNIT, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::FLOAT, RtString::UNITTYPE, RtType::INTERNSTRING);

            addInputAttributeByType(RtType::VECTOR2, RtString::UNIT, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR2, RtString::UNITTYPE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR2, RtString::DEFAULTGEOMPROP, RtType::INTERNSTRING);

            addInputAttributeByType(RtType::VECTOR3, RtString::UNIT, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR3, RtString::UNITTYPE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR3, RtString::DEFAULTGEOMPROP, RtType::INTERNSTRING);

            addInputAttributeByType(RtType::VECTOR4, RtString::UNIT, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR4, RtString::UNITTYPE, RtType::INTERNSTRING);

            addInputAttributeByType(RtType::FILENAME, RtString::COLORSPACE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::FILENAME, RtString::FILEPREFIX, RtType::STRING);

            addOutputAttribute(RtString::DOC, RtType::STRING);
            addOutputAttribute(RtString::MEMBER, RtType::STRING);
            addOutputAttribute(RtString::WIDTH, RtType::INTEGER);
            addOutputAttribute(RtString::HEIGHT, RtType::INTEGER);
            addOutputAttribute(RtString::BITDEPTH, RtType::INTEGER);

            addOutputAttributeByType(RtType::COLOR3, RtString::COLORSPACE, RtType::INTERNSTRING);
            addOutputAttributeByType(RtType::COLOR4, RtString::COLORSPACE, RtType::INTERNSTRING);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNode, "node");

RtPrim RtNode::createPrim(const RtString& typeName, const RtString& name, RtPrim parent)
{
    RtPrim nodedef = RtApi::get().getDefinition<RtNodeDef>(typeName);
    if (!nodedef)
    {
        throw ExceptionRuntimeError("No nodedef registered with name '" + typeName.str() + "'");
    }
    return createNode(nodedef, name, parent);
}

RtPrim RtNode::createNode(RtPrim nodedef, const RtString& name, RtPrim parent)
{
    // Make sure this is a valid nodedef.
    RtNodeDef nodedefSchema(nodedef);
    if (!nodedef)
    {
        throw ExceptionRuntimeError("Given nodedef with name '" + nodedef.getName().str() + "' is not valid");
    }

    PvtPrim* nodedefPrim = PvtObject::cast<PvtPrim>(nodedef);

    const RtString nodeName = name.empty() ? nodedefSchema.getNode() : name;
    PvtObjHandle nodeH = PvtPrim::createNew(&_typeInfo, nodeName, PvtObject::cast<PvtPrim>(parent));
    PvtPrim* node = nodeH->asA<PvtPrim>();

    // Save the nodedef in a relationship.
    PvtRelationship* nodedefRelation = node->createRelationship(RtString::NODEDEF);
    nodedefRelation->connect(nodedefPrim);

    // Copy version tag if used.
    const RtString& version = nodedefSchema.getVersion();
    if (!version.empty())
    {
        RtTypedValue* attr = node->createAttribute(RtString::VERSION, RtType::INTERNSTRING);
        attr->asInternString() = version;
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
    PvtRelationship* nodedefRel = prim()->getRelationship(RtString::NODEDEF);
    if (!nodedefRel)
    {
        nodedefRel = prim()->createRelationship(RtString::NODEDEF);
    }
    else
    {
        nodedefRel->clearConnections();
    }
    nodedefRel->connect(PvtObject::cast(nodeDef));
}

RtPrim RtNode::getNodeDef() const
{
    PvtRelationship* nodedefRel = prim()->getRelationship(RtString::NODEDEF);
    return nodedefRel && nodedefRel->hasConnections() ? nodedefRel->getConnection() : RtPrim();
}

void RtNode::setVersion(const RtString& version)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::VERSION, RtType::INTERNSTRING);
    attr->asInternString() = version;
}

const RtString& RtNode::getVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::VERSION, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
}

}
