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

vector<MaterialAssignPtr> Look::getActiveMaterialAssigns() const
{
    vector<MaterialAssignPtr> activeAssigns;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<MaterialAssignPtr> assigns = elem->asA<Look>()->getMaterialAssigns();
        activeAssigns.insert(activeAssigns.end(), assigns.begin(), assigns.end());
    }
    return activeAssigns;
}

vector<PropertyAssignPtr> Look::getActivePropertyAssigns() const
{
    vector<PropertyAssignPtr> activeAssigns;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<PropertyAssignPtr> assigns = elem->asA<Look>()->getPropertyAssigns();
        activeAssigns.insert(activeAssigns.end(), assigns.begin(), assigns.end());
    }
    return activeAssigns;
}

vector<PropertySetAssignPtr> Look::getActivePropertySetAssigns() const
{
    vector<PropertySetAssignPtr> activeAssigns;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<PropertySetAssignPtr> assigns = elem->asA<Look>()->getPropertySetAssigns();
        activeAssigns.insert(activeAssigns.end(), assigns.begin(), assigns.end());
    }
    return activeAssigns;
}

vector<VisibilityPtr> Look::getActiveVisibilities() const
{
    vector<VisibilityPtr> activeVisibilities;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<VisibilityPtr> visibilities = elem->asA<Look>()->getVisibilities();
        activeVisibilities.insert(activeVisibilities.end(), visibilities.begin(), visibilities.end());
    }
    return activeVisibilities;
}

void Look::setInheritsFrom(ElementPtr look)
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

ElementPtr Look::getInheritsFrom() const
{
    vector<LookInheritPtr> inherits = getLookInherits();
    if (inherits.empty())
    {
        return nullptr;
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
