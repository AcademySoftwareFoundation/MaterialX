//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtRelationship.h>

namespace MaterialX
{

RT_DEFINE_RUNTIME_OBJECT(PvtRelationship, RtObjType::RELATIONSHIP, "PvtRelationship")

PvtRelationship::PvtRelationship(const RtToken& name, PvtPrim* parent) :
    PvtObject(name, parent)
{
    setTypeBit<PvtRelationship>();
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
