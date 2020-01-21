//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtRelationship.h>

namespace MaterialX
{

const RtObjType PvtRelationship::typeId = RtObjType::RELATIONSHIP;
const RtToken PvtRelationship::typeName = RtToken("relationship");

PvtRelationship::PvtRelationship(const RtToken& name, PvtPrim* parent) :
    PvtPathItem(name, parent)
{
}

void PvtRelationship::removeTarget(const PvtPathItem* target)
{
    for (auto it = _targets.begin(); it != _targets.end(); ++it)
    {
        if (it->get() == target)
        {
            _targets.erase(it);
            break;
        }
    }
}

}
