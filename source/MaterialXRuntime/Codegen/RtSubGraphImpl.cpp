//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Codegen/RtSubGraphImpl.h>
#include <MaterialXRuntime/Identifiers.h>

#include <MaterialXRuntime/Private/PvtPath.h>
#include <MaterialXRuntime/Private/PvtPrim.h>

namespace MaterialX
{

DEFINE_TYPED_SCHEMA(RtSubGraphImpl, "nodeimpl:subgraphimpl");

RtPrim RtSubGraphImpl::createPrim(const RtIdentifier& typeName, const RtIdentifier& name, RtPrim parent)
{
    PvtPrim::validateCreation(_typeInfo, typeName, name, parent.getPath());

    static const RtIdentifier DEFAULT_NAME("subgraphimpl1");
    const RtIdentifier primName = name == EMPTY_IDENTIFIER ? DEFAULT_NAME : name;
    PvtObjHandle primH = PvtPrim::createNew(&_typeInfo, primName, PvtObject::cast<PvtPrim>(parent));

    PvtPrim* prim = primH->asA<PvtPrim>();
    prim->createRelationship(Identifiers::NODEIMPL);

    return primH;
}

void RtSubGraphImpl::initialize(const RtPrim& /*nodegraph*/)
{
}

RtPrim RtSubGraphImpl::getNodeGraph() const
{
    return RtPrim();
}

void RtSubGraphImpl::emitFunctionDefinition(const RtNode& /*node*/, GenContext& /*context*/, ShaderStage& /*stage*/) const
{
}

void RtSubGraphImpl::emitFunctionCall(const RtNode& /*node*/, GenContext& /*context*/, ShaderStage& /*stage*/) const
{
}

}
