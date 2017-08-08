//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Look.h>

namespace MaterialX
{

const string MaterialAssign::MATERIAL_ATTRIBUTE = "material";
const string MaterialAssign::EXCLUSIVE_ATTRIBUTE = "exclusive";

const string Visibility::VIEWER_GEOM_ATTRIBUTE = "viewergeom";
const string Visibility::VIEWER_COLLECTION_ATTRIBUTE = "viewercollection";
const string Visibility::VISIBILITY_TYPE_ATTRIBUTE = "vistype";
const string Visibility::VISIBLE_ATTRIBUTE = "visible";

//
// Look methods
//

MaterialAssignPtr Look::addMaterialAssign(const string& name, const string& material)
{
    MaterialAssignPtr matAssign = addChild<MaterialAssign>(name);
    if (!material.empty())
    {
        matAssign->setMaterial(material);
    }
    return matAssign;
}

void Look::setInheritsFrom(LookPtr look)
{
    for (LookInheritPtr inherit : getLookInherits())
    {
        removeLookInherit(inherit->getName());
    }
    if (look)
    {
        addLookInherit(look->getName());
    }
}

LookPtr Look::getInheritsFrom() const
{
    vector<LookInheritPtr> inherits = getLookInherits();
    if (inherits.empty())
    {
        return LookPtr();
    }
    return getRoot()->getChildOfType<Look>(inherits[0]->getName());
}

//
// MaterialAssign methods
//

MaterialPtr MaterialAssign::getReferencedMaterial() const
{
    return getRoot()->getChildOfType<Material>(getMaterial());   
}

} // namespace MaterialX
