//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtRelationship.h>

namespace MaterialX
{

PvtRelationship::PvtRelationship(const RtToken& name, PvtPrim* parent) :
    PvtObject(RtObjType::RELATIONSHIP, name, parent)
{
}

void PvtRelationship::removeTarget(const PvtObject* target)
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
