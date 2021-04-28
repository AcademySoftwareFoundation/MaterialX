//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/Identifiers.h>

#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

namespace
{
    // TODO: We should derive this from a data driven XML schema.
    class PvtNodeDefPrimSpec : public PvtPrimSpec
    {
    public:
        PvtNodeDefPrimSpec()
        {
            addPrimAttribute(Identifiers::DOC, RtType::STRING);
            addPrimAttribute(Identifiers::NODE, RtType::IDENTIFIER);
            addPrimAttribute(Identifiers::INHERIT, RtType::IDENTIFIER);
            addPrimAttribute(Identifiers::NODEGROUP, RtType::IDENTIFIER);
            addPrimAttribute(Identifiers::VERSION, RtType::IDENTIFIER);
            addPrimAttribute(Identifiers::ISDEFAULTVERSION, RtType::BOOLEAN);
            addPrimAttribute(Identifiers::TARGET, RtType::IDENTIFIER);
            addPrimAttribute(Identifiers::UINAME, RtType::STRING);
            addPrimAttribute(Identifiers::INTERNALGEOMPROPS, RtType::IDENTIFIER);
            addPrimAttribute(Identifiers::NAMESPACE, RtType::IDENTIFIER);

            addInputAttribute(Identifiers::DOC, RtType::STRING);
            addInputAttribute(Identifiers::UNIFORM, RtType::BOOLEAN);
            addInputAttribute(Identifiers::DEFAULTGEOMPROP, RtType::IDENTIFIER);
            addInputAttribute(Identifiers::ENUM, RtType::STRING);
            addInputAttribute(Identifiers::ENUMVALUES, RtType::STRING);
            addInputAttribute(Identifiers::UINAME, RtType::STRING);
            addInputAttribute(Identifiers::UIFOLDER, RtType::STRING);
            addInputAttribute(Identifiers::UIVISIBLE, RtType::BOOLEAN);

            addInputAttributeByType(RtType::COLOR3, Identifiers::COLORSPACE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::COLOR3, Identifiers::UIMIN, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, Identifiers::UIMAX, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, Identifiers::UISOFTMIN, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, Identifiers::UISOFTMAX, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, Identifiers::UISTEP, RtType::COLOR3);

            addInputAttributeByType(RtType::COLOR4, Identifiers::COLORSPACE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::COLOR4, Identifiers::UIMIN, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, Identifiers::UIMAX, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, Identifiers::UISOFTMIN, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, Identifiers::UISOFTMAX, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, Identifiers::UISTEP, RtType::COLOR4);

            addInputAttributeByType(RtType::FLOAT, Identifiers::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UNITTYPE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UIMIN, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UIMAX, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UISOFTMIN, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UISOFTMAX, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, Identifiers::UISTEP, RtType::FLOAT);

            addInputAttributeByType(RtType::VECTOR2, Identifiers::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UNITTYPE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UIMIN, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UIMAX, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UISOFTMIN, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UISOFTMAX, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::UISTEP, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Identifiers::DEFAULTGEOMPROP, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::VECTOR3, Identifiers::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UNITTYPE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UIMIN, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UIMAX, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UISOFTMIN, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UISOFTMAX, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::UISTEP, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Identifiers::DEFAULTGEOMPROP, RtType::IDENTIFIER);

            addInputAttributeByType(RtType::VECTOR4, Identifiers::UNIT, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UNITTYPE, RtType::IDENTIFIER);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UIMIN, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UIMAX, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UISOFTMIN, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UISOFTMAX, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, Identifiers::UISTEP, RtType::VECTOR4);

            addOutputAttribute(Identifiers::DOC, RtType::STRING);
            addOutputAttribute(Identifiers::DEFAULTINPUT, RtType::IDENTIFIER);
            addOutputAttribute(Identifiers::DEFAULT, RtType::STRING);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNodeDef, "nodedef");

RtPrim RtNodeDef::createPrim(const RtIdentifier& typeName, const RtIdentifier& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, name, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createAttribute(Identifiers::NODE, RtType::IDENTIFIER);
    prim->createRelationship(Identifiers::NODEIMPL);

    return primH;
}

const RtPrimSpec& RtNodeDef::getPrimSpec() const
{
    static const PvtNodeDefPrimSpec s_primSpec;
    return s_primSpec;
}

void RtNodeDef::setNode(const RtIdentifier& node)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::NODE, RtType::IDENTIFIER);
    attr->asIdentifier() = node;
}

const RtIdentifier& RtNodeDef::getNode() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::NODE, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : EMPTY_IDENTIFIER;
}

RtIdentifier RtNodeDef::getNamespacedNode() const
{
    const RtIdentifier& node = getNode();
    const RtIdentifier& namespaceString = getNamespace();
    if (namespaceString != EMPTY_IDENTIFIER)
    {
        return RtIdentifier(namespaceString.str() + NAME_PREFIX_SEPARATOR + node.str());
    }
    return node;
}

void RtNodeDef::setNodeGroup(const RtIdentifier& nodegroup)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::NODEGROUP, RtType::IDENTIFIER);
    attr->asIdentifier() = nodegroup;
}

const RtIdentifier& RtNodeDef::getNodeGroup() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::NODEGROUP, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : EMPTY_IDENTIFIER;
}

void RtNodeDef::setTarget(const RtIdentifier& nodegroup)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::TARGET, RtType::IDENTIFIER);
    attr->asIdentifier() = nodegroup;
}

const RtIdentifier& RtNodeDef::getTarget() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::TARGET, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : EMPTY_IDENTIFIER;
}

void RtNodeDef::setIneritance(const RtIdentifier& inherit)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::INHERIT, RtType::IDENTIFIER);
    attr->asIdentifier() = inherit;
}

const RtIdentifier& RtNodeDef::getIneritance() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::INHERIT, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : EMPTY_IDENTIFIER;
}

void RtNodeDef::setVersion(const RtIdentifier& version)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::VERSION, RtType::IDENTIFIER);
    attr->asIdentifier() = version;
}

const RtIdentifier& RtNodeDef::getVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::VERSION, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : EMPTY_IDENTIFIER;
}

void RtNodeDef::setIsDefaultVersion(bool isDefault)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::ISDEFAULTVERSION, RtType::BOOLEAN);
    attr->asBool() = isDefault;
}

bool RtNodeDef::getIsDefaultVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::ISDEFAULTVERSION, RtType::BOOLEAN);
    return attr ? attr->asBool() : false;
}

void RtNodeDef::setNamespace(const RtIdentifier& space)
{
    RtTypedValue* attr = prim()->createAttribute(Identifiers::NAMESPACE, RtType::IDENTIFIER);
    attr->asIdentifier() = space;
}

const RtIdentifier& RtNodeDef::getNamespace() const
{
    RtTypedValue* attr = prim()->getAttribute(Identifiers::NAMESPACE, RtType::IDENTIFIER);
    return attr ? attr->asIdentifier() : EMPTY_IDENTIFIER;
}

void RtNodeDef::setDoc(const string& doc) {
    RtTypedValue* attr = prim()->createAttribute(Identifiers::DOC, RtType::STRING);
    attr->asString() = doc;
}

const string& RtNodeDef::getDoc() const {
    RtTypedValue* attr = prim()->getAttribute(Identifiers::DOC, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

bool RtNodeDef::isVersionCompatible(const RtIdentifier& version) const
{
    // Test if either the version matches or if no version passed in if this is the default version.
    return ((version == getVersion()) ||
            (version.str().empty() && getIsDefaultVersion()));
}

RtRelationship RtNodeDef::getNodeImpls() const
{
    PvtRelationship* rel = prim()->getRelationship(Identifiers::NODEIMPL);
    return rel ? rel->hnd() : RtRelationship();
}

RtPrim RtNodeDef::getNodeImpl(const RtIdentifier& target) const
{
    RtRelationship rel = getNodeImpls();
    for (RtObject obj : rel.getConnections())
    {
        if (obj.isA<RtPrim>())
        {
            RtPrim prim(obj);
            if (prim.hasApi<RtNodeImpl>() || prim.hasApi<RtNodeGraph>())
            {
                const RtTypedValue* attr = prim.getAttribute(Identifiers::TARGET, RtType::IDENTIFIER);
                const RtIdentifier primTarget = attr ? attr->asIdentifier() : EMPTY_IDENTIFIER;
                if (primTarget == EMPTY_IDENTIFIER || primTarget == target)
                {
                    return prim;
                }
            }
        }
    }
    return RtPrim();
}

RtNodeLayout RtNodeDef::getNodeLayout()
{
    RtNodeLayout layout;
    for (size_t i=0; i<numInputs(); ++i)
    {
        RtInput input = getInput(i);
        layout.order.push_back(input.getName());
        RtTypedValue* data = input.getAttribute(Identifiers::UIFOLDER);
        if (data && data->getType() == RtType::STRING)
        {
            layout.uifolder[input.getName()] = data->asString();
        }
    }
    return layout;
}

}
