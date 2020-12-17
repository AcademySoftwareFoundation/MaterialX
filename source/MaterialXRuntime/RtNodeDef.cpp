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

DEFINE_TYPED_SCHEMA(RtNodeDef, "nodedef");

RtPrim RtNodeDef::createPrim(const RtToken& typeName, const RtToken& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    PvtDataHandle primH = PvtPrim::createNew(&_typeInfo, name, PvtObject::ptr<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->addMetadata(Tokens::NODE, RtType::TOKEN);
    prim->createRelationship(Tokens::NODEIMPL);

    return primH;
}

const RtToken& RtNodeDef::getNode() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::NODE);
    return v ? v->getValue().asToken() : EMPTY_TOKEN;
}

RtToken RtNodeDef::getNamespacedNode() const
{
    const RtToken& nodeToken = getNode();
    string nodeString = nodeToken.c_str();
    string namespaceString = getNamespace().c_str();
    if (!namespaceString.empty())
    {
        return RtToken(namespaceString + NAME_PREFIX_SEPARATOR + nodeString);
    }
    return nodeToken;
}

void RtNodeDef::setNode(const RtToken& node)
{
    RtTypedValue* v = prim()->addMetadata(Tokens::NODE, RtType::TOKEN);
    v->getValue().asToken() = node;
}

const RtToken& RtNodeDef::getNodeGroup() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::NODEGROUP, RtType::TOKEN);
    return v ? v->getValue().asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setNodeGroup(const RtToken& nodegroup)
{
    RtTypedValue* v = prim()->addMetadata(Tokens::NODEGROUP, RtType::TOKEN);
    v->getValue().asToken() = nodegroup;
}

const RtToken& RtNodeDef::getTarget() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::TARGET, RtType::TOKEN);
    return v ? v->getValue().asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setTarget(const RtToken& nodegroup)
{
    RtTypedValue* v = prim()->addMetadata(Tokens::TARGET, RtType::TOKEN);
    v->getValue().asToken() = nodegroup;
}

const RtToken& RtNodeDef::getIneritance() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::INHERIT, RtType::TOKEN);
    return v ? v->getValue().asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setIneritance(const RtToken& inherit)
{
    RtTypedValue* v = prim()->addMetadata(Tokens::INHERIT, RtType::TOKEN);
    v->getValue().asToken() = inherit;
}

const RtToken& RtNodeDef::getVersion() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::VERSION, RtType::TOKEN);
    return v ? v->getValue().asToken() : EMPTY_TOKEN;
}

void RtNodeDef::setVersion(const RtToken& version)
{
    RtTypedValue* v = prim()->addMetadata(Tokens::VERSION, RtType::TOKEN);
    v->getValue().asToken() = version;
}

bool RtNodeDef::getIsDefaultVersion() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::IS_DEFAULT_VERSION, RtType::BOOLEAN);
    return v ? v->getValue().asBool() : false;
}

void RtNodeDef::setIsDefaultVersion(bool isDefault)
{
    RtTypedValue* v = prim()->addMetadata(Tokens::IS_DEFAULT_VERSION, RtType::BOOLEAN);
    v->getValue().asBool() = isDefault;
}

const string& RtNodeDef::getNamespace() const
{
    RtTypedValue* v = prim()->getMetadata(Tokens::NAMESPACE, RtType::STRING);
    return v ? v->getValue().asString() : EMPTY_TOKEN;
}

void RtNodeDef::setNamespace(const string& space)
{
    RtTypedValue* v = prim()->addMetadata(Tokens::NAMESPACE, RtType::STRING);
    v->getValue().asString() = space;
}

bool RtNodeDef::isVersionCompatible(const RtToken& version) const
{
    // Test if either the version matches or if no version passed in if this is the default version.
    return ((version == getVersion()) ||
            (version.str().empty() && getIsDefaultVersion()));
}

RtInput RtNodeDef::createInput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    return prim()->createInput(name, type, flags)->hnd();
}

void RtNodeDef::removeInput(const RtToken& name)
{
    PvtInput* input = prim()->getInput(name);
    if (!(input && input->isA<PvtInput>()))
    {
        throw ExceptionRuntimeError("No input found with name '" + name.str() + "'");
    }
    prim()->removeAttribute(name);
}

RtOutput RtNodeDef::createOutput(const RtToken& name, const RtToken& type, uint32_t flags)
{
    return prim()->createOutput(name, type, flags)->hnd();
}

void RtNodeDef::removeOutput(const RtToken& name)
{
    PvtOutput* output = prim()->getOutput(name);
    if (!(output && output->isA<PvtOutput>()))
    {
        throw ExceptionRuntimeError("No output found with name '" + name.str() + "'");
    }
    prim()->removeAttribute(name);
}

size_t RtNodeDef::numInputs() const
{
    return prim()->numInputs();
}

RtInput RtNodeDef::getInput(const RtToken& name) const
{
    PvtInput* input = prim()->getInput(name);
    return input ? input->hnd() : RtInput();
}

RtAttrIterator RtNodeDef::getInputs() const
{
    RtObjTypePredicate<RtInput> filter;
    return RtAttrIterator(getPrim(), filter);
}

size_t RtNodeDef::numOutputs() const
{
    return prim()->numOutputs();
}

RtOutput RtNodeDef::getOutput(const RtToken& name) const
{
    PvtOutput* output = prim()->getOutput(name);
    return output ? output->hnd() : RtOutput();
}

RtOutput RtNodeDef::getOutput() const
{
    PvtOutput* output = prim()->getOutput();
    return output ? output->hnd() : RtOutput();
}

RtAttrIterator RtNodeDef::getOutputs() const
{
    RtObjTypePredicate<RtOutput> filter;
    return RtAttrIterator(getPrim(), filter);
}

RtRelationship RtNodeDef::getNodeImpls() const
{
    PvtRelationship* rel = prim()->getRelationship(Tokens::NODEIMPL);
    return rel ? rel->hnd() : RtRelationship();
}

RtPrim RtNodeDef::getNodeImpl(const RtToken& target) const
{
    RtRelationship rel = getNodeImpls();
    for (RtObject obj : rel.getTargets())
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
    for (RtAttribute input : getInputs())
    {
        layout.order.push_back(input.getName());
        RtTypedValue* data = input.getMetadata(Tokens::UIFOLDER);
        if (data && data->getType() == RtType::STRING)
        {
            layout.uifolder[input.getName()] = data->getValue().asString();
        }
    }
    return layout;
}

}
