//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/Tokens.h>

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
            addPrimAttribute(Tokens::DOC, RtType::STRING);
            addPrimAttribute(Tokens::NODE, RtType::TOKEN);
            addPrimAttribute(Tokens::INHERIT, RtType::TOKEN);
            addPrimAttribute(Tokens::NODEGROUP, RtType::TOKEN);
            addPrimAttribute(Tokens::VERSION, RtType::TOKEN);
            addPrimAttribute(Tokens::ISDEFAULTVERSION, RtType::BOOLEAN);
            addPrimAttribute(Tokens::TARGET, RtType::TOKEN);
            addPrimAttribute(Tokens::UINAME, RtType::STRING);
            addPrimAttribute(Tokens::INTERNALGEOMPROPS, RtType::TOKEN);
            addPrimAttribute(Tokens::NAMESPACE, RtType::TOKEN);

            addInputAttribute(Tokens::DOC, RtType::STRING);
            addInputAttribute(Tokens::UNIFORM, RtType::BOOLEAN);
            addInputAttribute(Tokens::DEFAULTGEOMPROP, RtType::TOKEN);
            addInputAttribute(Tokens::ENUM, RtType::STRING);
            addInputAttribute(Tokens::ENUMVALUES, RtType::STRING);
            addInputAttribute(Tokens::UINAME, RtType::STRING);
            addInputAttribute(Tokens::UIFOLDER, RtType::STRING);

            addInputAttributeByType(RtType::COLOR3, Tokens::COLORSPACE, RtType::TOKEN);
            addInputAttributeByType(RtType::COLOR3, Tokens::UIMIN, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, Tokens::UIMAX, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, Tokens::UISOFTMIN, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, Tokens::UISOFTMAX, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, Tokens::UISTEP, RtType::COLOR3);

            addInputAttributeByType(RtType::COLOR4, Tokens::COLORSPACE, RtType::TOKEN);
            addInputAttributeByType(RtType::COLOR4, Tokens::UIMIN, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, Tokens::UIMAX, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, Tokens::UISOFTMIN, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, Tokens::UISOFTMAX, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, Tokens::UISTEP, RtType::COLOR4);

            addInputAttributeByType(RtType::FLOAT, Tokens::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::FLOAT, Tokens::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::FLOAT, Tokens::UIMIN, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, Tokens::UIMAX, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, Tokens::UISOFTMIN, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, Tokens::UISOFTMAX, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, Tokens::UISTEP, RtType::FLOAT);

            addInputAttributeByType(RtType::VECTOR2, Tokens::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR2, Tokens::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR2, Tokens::UIMIN, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Tokens::UIMAX, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Tokens::UISOFTMIN, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Tokens::UISOFTMAX, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Tokens::UISTEP, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, Tokens::DEFAULTGEOMPROP, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR3, Tokens::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR3, Tokens::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR3, Tokens::UIMIN, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Tokens::UIMAX, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Tokens::UISOFTMIN, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Tokens::UISOFTMAX, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Tokens::UISTEP, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, Tokens::DEFAULTGEOMPROP, RtType::TOKEN);

            addInputAttributeByType(RtType::VECTOR4, Tokens::UNIT, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR4, Tokens::UNITTYPE, RtType::TOKEN);
            addInputAttributeByType(RtType::VECTOR4, Tokens::UIMIN, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, Tokens::UIMAX, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, Tokens::UISOFTMIN, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, Tokens::UISOFTMAX, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, Tokens::UISTEP, RtType::VECTOR4);

            addOutputAttribute(Tokens::DOC, RtType::STRING);
            addOutputAttribute(Tokens::DEFAULTINPUT, RtType::TOKEN);
            addOutputAttribute(Tokens::DEFAULT, RtType::STRING);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNodeDef, "nodedef");

RtPrim RtNodeDef::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, name, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createAttribute(Tokens::NODE, RtType::TOKEN);
    prim->createRelationship(Tokens::NODEIMPL);

    return primH;
}

const RtPrimSpec& RtNodeDef::getPrimSpec() const
{
    static const PvtNodeDefPrimSpec s_primSpec;
    return s_primSpec;
}

void RtNodeDef::setNode(const RtToken& node)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::NODE, RtType::TOKEN);
    attr->asToken() = node;
}

const RtToken& RtNodeDef::getNode() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::NODE, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

RtToken RtNodeDef::getNamespacedNode() const
{
    const RtToken& node = getNode();
    const RtToken& namespaceString = getNamespace();
    if (namespaceString != EMPTY_TOKEN)
    {
        return RtToken(namespaceString.str() + NAME_PREFIX_SEPARATOR + node.str());
    }
    return node;
}

void RtNodeDef::setNodeGroup(const RtToken& nodegroup)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::NODEGROUP, RtType::TOKEN);
    attr->asToken() = nodegroup;
}

const RtToken& RtNodeDef::getNodeGroup() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::NODEGROUP, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setTarget(const RtToken& nodegroup)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::TARGET, RtType::TOKEN);
    attr->asToken() = nodegroup;
}

const RtToken& RtNodeDef::getTarget() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::TARGET, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setIneritance(const RtToken& inherit)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::INHERIT, RtType::TOKEN);
    attr->asToken() = inherit;
}

const RtToken& RtNodeDef::getIneritance() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::INHERIT, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setVersion(const RtToken& version)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::VERSION, RtType::TOKEN);
    attr->asToken() = version;
}

const RtToken& RtNodeDef::getVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::VERSION, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setIsDefaultVersion(bool isDefault)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::ISDEFAULTVERSION, RtType::BOOLEAN);
    attr->asBool() = isDefault;
}

bool RtNodeDef::getIsDefaultVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::ISDEFAULTVERSION, RtType::BOOLEAN);
    return attr ? attr->asBool() : false;
}

void RtNodeDef::setNamespace(const RtToken& space)
{
    RtTypedValue* attr = prim()->createAttribute(Tokens::NAMESPACE, RtType::TOKEN);
    attr->asToken() = space;
}

const RtToken& RtNodeDef::getNamespace() const
{
    RtTypedValue* attr = prim()->getAttribute(Tokens::NAMESPACE, RtType::TOKEN);
    return attr ? attr->asToken() : EMPTY_TOKEN;
}

bool RtNodeDef::isVersionCompatible(const RtToken& version) const
{
    // Test if either the version matches or if no version passed in if this is the default version.
    return ((version == getVersion()) ||
            (version.str().empty() && getIsDefaultVersion()));
}

RtRelationship RtNodeDef::getNodeImpls() const
{
    PvtRelationship* rel = prim()->getRelationship(Tokens::NODEIMPL);
    return rel ? rel->hnd() : RtRelationship();
}

RtPrim RtNodeDef::getNodeImpl(const RtToken& target) const
{
    RtRelationship rel = getNodeImpls();
    for (RtObject obj : rel.getConnections())
    {
        if (obj.isA<RtPrim>())
        {
            RtNodeImpl impl(obj);
            if (impl.isValid() && (target == EMPTY_TOKEN || impl.getTarget() == target))
            {
                return impl.getPrim();
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
        RtTypedValue* data = input.getAttribute(Tokens::UIFOLDER);
        if (data && data->getType() == RtType::STRING)
        {
            layout.uifolder[input.getName()] = data->asString();
        }
    }
    return layout;
}

}
