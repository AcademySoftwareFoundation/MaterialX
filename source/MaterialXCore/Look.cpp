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
// MaterialAssign methods
//

vector<VariantAssignPtr> MaterialAssign::getActiveVariantAssigns() const
{
    vector<VariantAssignPtr> activeAssigns;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<VariantAssignPtr> assigns = elem->asA<MaterialAssign>()->getVariantAssigns();
        activeAssigns.insert(activeAssigns.end(), assigns.begin(), assigns.end());
    }
    return activeAssigns;
}

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

vector<VariantAssignPtr> Look::getActiveVariantAssigns() const
{
    vector<VariantAssignPtr> activeAssigns;
    for (ConstElementPtr elem : traverseInheritance())
    {
        vector<VariantAssignPtr> assigns = elem->asA<Look>()->getVariantAssigns();
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

//
// MaterialAssign methods
//

MaterialPtr MaterialAssign::getReferencedMaterial() const
{
    return resolveRootNameReference<Material>(getMaterial());   
}

} // namespace MaterialX
