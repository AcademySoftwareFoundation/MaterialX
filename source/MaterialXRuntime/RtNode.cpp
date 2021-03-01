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
    static const RtToken NODEDEF("nodedef");

    static const RtTokenVec PUBLIC_INPUT_COLOR_METADATA_NAMES
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("Value"),
        RtToken("nodename"),
        RtToken("nodegraph"),
        RtToken("output"),
        RtToken("member"),
        RtToken("channels"),
        RtToken("colorspace")
    };

    static const RtTokenVec PUBLIC_INPUT_FLOAT_METADATA_NAMES
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("Value"),
        RtToken("nodename"),
        RtToken("nodegraph"),
        RtToken("output"),
        RtToken("member"),
        RtToken("channels"),
        RtToken("unit"),
        RtToken("unittype")
    };

    static const RtTokenVec PUBLIC_INPUT_METADATA_NAMES
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("Value"),
        RtToken("nodename"),
        RtToken("nodegraph"),
        RtToken("output"),
        RtToken("member"),
        RtToken("channels")
    };

    static const RtTokenVec PUBLIC_OUTPUT_COLOR_METADATA_NAMES
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("nodename"),
        RtToken("output"),
        RtToken("member"),
        RtToken("colorspace"),
        RtToken("width"),
        RtToken("height"),
        RtToken("bitdepth")
    };
    static const RtTokenVec PUBLIC_OUTPUT_METADATA_NAMES
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("nodename"),
        RtToken("output"),
        RtToken("member"),
        RtToken("width"),
        RtToken("height"),
        RtToken("bitdepth")
    };

    static const RtTokenVec PUBLIC_METADATA_NAMES
    {
        RtToken("name"),
        RtToken("type"),
        RtToken("value"),
        RtToken("doc"),
        RtToken("xpos"),
        RtToken("ypos"),
        RtToken("width"),
        RtToken("height"),
        RtToken("uicolor"),
        RtToken("uiname"),
        RtToken("uivisible"),
        RtToken("uiadvanced"),
        RtToken("version"),
        RtToken("cms"),
        RtToken("cmsconfig"),
        RtToken("colorspace"),
        RtToken("namespace")
    };

    static const RtTokenVec PUBLIC_EMPTY_METADATA_NAMES;
}

DEFINE_TYPED_SCHEMA(RtNode, "node");

RtPrim RtNode::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    const RtPrim prim = RtApi::get().getNodeDef(typeName);
    if (!prim)
    {
        throw ExceptionRuntimeError("No nodedef registered with name '" + typeName.str() + "'");
    }

    // Make sure this is a valid nodedef.
    RtNodeDef nodedef(prim);
    if (!nodedef)
    {
        throw ExceptionRuntimeError("Nodedef with name '" + typeName.str() + "' is not valid");
    }

    const PvtPrim* nodedefPrim = PvtObject::ptr<PvtPrim>(prim);

    const RtToken nodeName = name == EMPTY_TOKEN ? nodedef.getNode() : name;
    PvtDataHandle nodeH = PvtPrim::createNew(&_typeInfo, nodeName, PvtObject::ptr<PvtPrim>(parent));
    PvtPrim* node = nodeH->asA<PvtPrim>();

    // Save the nodedef in a relationship.
    PvtRelationship* nodedefRelation = node->createRelationship(NODEDEF);
    nodedefRelation->addTarget(nodedefPrim);

    // Copy over meta-data from nodedef to node. 
    // TODO: Checks with ILM need to be made to make sure that the appropriate
    // meta-data set. TBD if target should be set.
    RtTokenSet copyList = { Tokens::VERSION };
    const vector<RtToken>& metadata = nodedefPrim->getMetadataOrder();
    for (const RtToken& dataName : metadata)
    { 
        if (copyList.count(dataName))
        {
            const RtTypedValue* src = nodedefPrim->getMetadata(dataName);
            RtTypedValue* v = src ? node->addMetadata(dataName, src->getType()) : nullptr;
            if (v)
            {
                RtToken valueToCopy = src->getValue().asToken();
                v->getValue().asToken() = valueToCopy;
            }
        }
    }

    // Create the interface according to nodedef.
    for (const PvtDataHandle& attrH : nodedefPrim->getAllAttributes())
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

const RtToken& RtNode::getVersion() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::VERSION, RtType::TOKEN);
    return v ? v->getValue().asToken() : EMPTY_TOKEN;
}

void RtNode::setVersion(const RtToken& version)
{
    RtTypedValue* v = prim()->addMetadata(Tokens::VERSION, RtType::TOKEN);
    v->getValue().asToken() = version;
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
    return getPrim().getInputs();
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
    return getPrim().getOutputs();
}

const RtTokenVec& RtNode::getPublicMetadataNames() const
{
    return PUBLIC_METADATA_NAMES;
}

const RtTokenVec& RtNode::getPublicPortMetadataNames(const RtToken& name) const
{
    RtInput input = getInput(name);
    if (input)
    {
        const RtToken& type = input.getType();
        if (type == RtType::COLOR3 || type == RtType::COLOR4 || type == RtType::FILENAME)
        {
            return PUBLIC_INPUT_COLOR_METADATA_NAMES;
        }
        else if(type == RtType::FLOAT || type == RtType::VECTOR2 || type == RtType::VECTOR3 || type == RtType::VECTOR4)
        {
            return PUBLIC_INPUT_FLOAT_METADATA_NAMES;
        }
        else
        {
            return PUBLIC_INPUT_METADATA_NAMES;
        }
    }
    else
    {
        RtOutput output = getOutput(name);
        if (output)
        {
            const RtToken& type = output.getType();
            if (type == RtType::COLOR3 || type == RtType::COLOR4 || type == RtType::FILENAME)
            {
                return PUBLIC_OUTPUT_COLOR_METADATA_NAMES;
            }
            else
            {
                return PUBLIC_OUTPUT_METADATA_NAMES;
            }
        }
    }

    return PUBLIC_EMPTY_METADATA_NAMES;
}

}
