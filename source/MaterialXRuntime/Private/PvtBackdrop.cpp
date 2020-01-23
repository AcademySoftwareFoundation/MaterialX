//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtBackdrop.h>

/// @file
/// TODO: Docs

namespace MaterialX
{

const RtObjType PvtBackdrop::_typeId = RtObjType::BACKDROP;
const RtToken PvtBackdrop::_typeName = RtToken("backdrop");

const RtToken PvtBackdrop::CONTAINS;

PvtBackdrop::PvtBackdrop(const RtToken& name, PvtPrim* parent) :
    PvtPrim(name, parent)
{
}

PvtDataHandle PvtBackdrop::createNew(const RtToken& name, PvtPrim* parent)
{
    if (parent->getChild(name))
    {
        throw ExceptionRuntimeError("A child named '" + name.str() + "' already exists in prim '" + parent->getName().str() + "'");
    }
    PvtDataHandle backdropH = PvtDataHandle(new PvtBackdrop(name, parent));

    PvtBackdrop* backdrop = backdropH->asA<PvtBackdrop>();
    backdrop->createRelationship(PvtBackdrop::CONTAINS);

    return backdropH;
}

}
