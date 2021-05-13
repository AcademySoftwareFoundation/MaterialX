//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtApi.h>

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
            addPrimAttribute(RtString::DOC, RtType::STRING);
            addPrimAttribute(RtString::NODE, RtType::INTERNSTRING);
            addPrimAttribute(RtString::INHERIT, RtType::INTERNSTRING);
            addPrimAttribute(RtString::NODEGROUP, RtType::INTERNSTRING);
            addPrimAttribute(RtString::VERSION, RtType::INTERNSTRING);
            addPrimAttribute(RtString::ISDEFAULTVERSION, RtType::BOOLEAN);
            addPrimAttribute(RtString::TARGET, RtType::INTERNSTRING);
            addPrimAttribute(RtString::UINAME, RtType::STRING);
            addPrimAttribute(RtString::INTERNALGEOMPROPS, RtType::INTERNSTRING);
            addPrimAttribute(RtString::NAMESPACE, RtType::INTERNSTRING);

            addInputAttribute(RtString::DOC, RtType::STRING);
            addInputAttribute(RtString::UNIFORM, RtType::BOOLEAN);
            addInputAttribute(RtString::DEFAULTGEOMPROP, RtType::INTERNSTRING);
            addInputAttribute(RtString::ENUM, RtType::STRING);
            addInputAttribute(RtString::ENUMVALUES, RtType::STRING);
            addInputAttribute(RtString::UINAME, RtType::STRING);
            addInputAttribute(RtString::UIFOLDER, RtType::STRING);
            addInputAttribute(RtString::UIVISIBLE, RtType::BOOLEAN);

            addInputAttributeByType(RtType::COLOR3, RtString::COLORSPACE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::COLOR3, RtString::UIMIN, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, RtString::UIMAX, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, RtString::UISOFTMIN, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, RtString::UISOFTMAX, RtType::COLOR3);
            addInputAttributeByType(RtType::COLOR3, RtString::UISTEP, RtType::COLOR3);

            addInputAttributeByType(RtType::COLOR4, RtString::COLORSPACE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::COLOR4, RtString::UIMIN, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, RtString::UIMAX, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, RtString::UISOFTMIN, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, RtString::UISOFTMAX, RtType::COLOR4);
            addInputAttributeByType(RtType::COLOR4, RtString::UISTEP, RtType::COLOR4);

            addInputAttributeByType(RtType::FLOAT, RtString::UNIT, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::FLOAT, RtString::UNITTYPE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::FLOAT, RtString::UIMIN, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, RtString::UIMAX, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, RtString::UISOFTMIN, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, RtString::UISOFTMAX, RtType::FLOAT);
            addInputAttributeByType(RtType::FLOAT, RtString::UISTEP, RtType::FLOAT);

            addInputAttributeByType(RtType::VECTOR2, RtString::UNIT, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR2, RtString::UNITTYPE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR2, RtString::UIMIN, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, RtString::UIMAX, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, RtString::UISOFTMIN, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, RtString::UISOFTMAX, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, RtString::UISTEP, RtType::VECTOR2);
            addInputAttributeByType(RtType::VECTOR2, RtString::DEFAULTGEOMPROP, RtType::INTERNSTRING);

            addInputAttributeByType(RtType::VECTOR3, RtString::UNIT, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR3, RtString::UNITTYPE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR3, RtString::UIMIN, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, RtString::UIMAX, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, RtString::UISOFTMIN, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, RtString::UISOFTMAX, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, RtString::UISTEP, RtType::VECTOR3);
            addInputAttributeByType(RtType::VECTOR3, RtString::DEFAULTGEOMPROP, RtType::INTERNSTRING);

            addInputAttributeByType(RtType::VECTOR4, RtString::UNIT, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR4, RtString::UNITTYPE, RtType::INTERNSTRING);
            addInputAttributeByType(RtType::VECTOR4, RtString::UIMIN, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, RtString::UIMAX, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, RtString::UISOFTMIN, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, RtString::UISOFTMAX, RtType::VECTOR4);
            addInputAttributeByType(RtType::VECTOR4, RtString::UISTEP, RtType::VECTOR4);

            addOutputAttribute(RtString::DOC, RtType::STRING);
            addOutputAttribute(RtString::DEFAULTINPUT, RtType::INTERNSTRING);
            addOutputAttribute(RtString::DEFAULT, RtType::STRING);
        }
    };
}

DEFINE_TYPED_SCHEMA(RtNodeDef, "nodedef");

RtPrim RtNodeDef::createPrim(const RtString& typeName, const RtString& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, name, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createAttribute(RtString::NODE, RtType::INTERNSTRING);
    prim->createRelationship(RtString::NODEIMPL);

    return primH;
}

const RtPrimSpec& RtNodeDef::getPrimSpec() const
{
    static const PvtNodeDefPrimSpec s_primSpec;
    return s_primSpec;
}

void RtNodeDef::setNode(const RtString& node)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::NODE, RtType::INTERNSTRING);
    attr->asInternString() = node;
}

const RtString& RtNodeDef::getNode() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::NODE, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
}

RtString RtNodeDef::getNamespacedNode() const
{
    const RtString& node = getNode();
    const RtString& namespaceString = getNamespace();
    if (namespaceString)
    {
        return RtString(namespaceString.str() + NAME_PREFIX_SEPARATOR + node.str());
    }
    return node;
}

void RtNodeDef::setNodeGroup(const RtString& nodegroup)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::NODEGROUP, RtType::INTERNSTRING);
    attr->asInternString() = nodegroup;
}

const RtString& RtNodeDef::getNodeGroup() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::NODEGROUP, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
}

void RtNodeDef::setTarget(const RtString& nodegroup)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::TARGET, RtType::INTERNSTRING);
    attr->asInternString() = nodegroup;
}

const RtString& RtNodeDef::getTarget() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::TARGET, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
}

void RtNodeDef::setIneritance(const RtString& inherit)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::INHERIT, RtType::INTERNSTRING);
    attr->asInternString() = inherit;
}

const RtString& RtNodeDef::getIneritance() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::INHERIT, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
}

void RtNodeDef::setVersion(const RtString& version)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::VERSION, RtType::INTERNSTRING);
    attr->asInternString() = version;
}

const RtString& RtNodeDef::getVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::VERSION, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
}

void RtNodeDef::setIsDefaultVersion(bool isDefault)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::ISDEFAULTVERSION, RtType::BOOLEAN);
    attr->asBool() = isDefault;
}

bool RtNodeDef::getIsDefaultVersion() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::ISDEFAULTVERSION, RtType::BOOLEAN);
    return attr ? attr->asBool() : false;
}

void RtNodeDef::setNamespace(const RtString& space)
{
    RtTypedValue* attr = prim()->createAttribute(RtString::NAMESPACE, RtType::INTERNSTRING);
    attr->asInternString() = space;
}

const RtString& RtNodeDef::getNamespace() const
{
    RtTypedValue* attr = prim()->getAttribute(RtString::NAMESPACE, RtType::INTERNSTRING);
    return attr ? attr->asInternString() : RtString::EMPTY;
}

void RtNodeDef::setDoc(const string& doc) {
    RtTypedValue* attr = prim()->createAttribute(RtString::DOC, RtType::STRING);
    attr->asString() = doc;
}

const string& RtNodeDef::getDoc() const {
    RtTypedValue* attr = prim()->getAttribute(RtString::DOC, RtType::STRING);
    return attr ? attr->asString() : EMPTY_STRING;
}

bool RtNodeDef::isVersionCompatible(const RtString& version) const
{
    // Test if either the version matches or if no version passed in if this is the default version.
    return ((version == getVersion()) ||
            (version.str().empty() && getIsDefaultVersion()));
}

RtRelationship RtNodeDef::getNodeImpls() const
{
    PvtRelationship* rel = prim()->getRelationship(RtString::NODEIMPL);
    return rel ? rel->hnd() : RtRelationship();
}

RtPrim RtNodeDef::getNodeImpl(const RtString& target) const
{
    RtRelationship rel = getNodeImpls();
    for (RtObject obj : rel.getConnections())
    {
        if (obj.isA<RtPrim>())
        {
            RtPrim prim(obj);
            if (prim.hasApi<RtNodeImpl>() || prim.hasApi<RtNodeGraph>())
            {
                const RtTypedValue* attr = prim.getAttribute(RtString::TARGET, RtType::INTERNSTRING);
                const RtString primTarget = attr ? attr->asInternString() : RtString::EMPTY;
                if (primTarget.empty() || primTarget == target)
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
        RtTypedValue* data = input.getAttribute(RtString::UIFOLDER);
        if (data && data->getType() == RtType::STRING)
        {
            layout.uifolder[input.getName()] = data->asString();
        }
    }
    return layout;
}

}
