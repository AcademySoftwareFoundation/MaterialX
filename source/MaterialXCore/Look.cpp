//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXCore/Look.h>

#include <MaterialXCore/Document.h>

namespace MaterialX
{

const string MaterialAssign::MATERIAL_ATTRIBUTE = "material";
const string MaterialAssign::EXCLUSIVE_ATTRIBUTE = "exclusive";

const string Visibility::VIEWER_GEOM_ATTRIBUTE = "viewergeom";
const string Visibility::VIEWER_COLLECTION_ATTRIBUTE = "viewercollection";
const string Visibility::VISIBILITY_TYPE_ATTRIBUTE = "vistype";
const string Visibility::VISIBLE_ATTRIBUTE = "visible";

const string LookGroup::LOOKS_ATTRIBUTE = "looks";
const string LookGroup::ACTIVE_ATTRIBUTE = "active";

vector<MaterialAssignPtr> getGeometryBindings(const NodePtr& materialNode, const string& geom)
{
    vector<MaterialAssignPtr> matAssigns;
    for (LookPtr look : materialNode->getDocument()->getLooks())
    {
        for (MaterialAssignPtr matAssign : look->getMaterialAssigns())
        {
            if (matAssign->getReferencedMaterial() == materialNode)
            {
                if (geomStringsMatch(geom, matAssign->getActiveGeom()))
                {
                    matAssigns.push_back(matAssign);
                    continue;
                }
                CollectionPtr coll = matAssign->getCollection();
                if (coll && coll->matchesGeomString(geom))
                {
                    matAssigns.push_back(matAssign);
                    continue;
                }
            }
        }
    }
    return matAssigns;
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

void Look::append(const LookPtr& source)
{
    for (auto child : source->getChildren())
    {
        if (!child)
        {
            continue;
        }
        string name = source->getName() + "_" + child->getName();

        ConstElementPtr previous = getChild(name);
        if (previous)
        {
            name = createValidChildName(name);
        }

        // Create the copied element.
        ElementPtr childCopy = addChildOfCategory(child->getCategory(), name);
        childCopy->copyContentFrom(child);
    }
}

//
// MaterialAssign methods
//

NodePtr MaterialAssign::getReferencedMaterial() const
{
    return resolveRootNameReference<Node>(getMaterial());
}

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
// Lookgroup methods
//

LookVec LookGroup::getActiveLooks() const
{
    string looks = getActiveLook();
    if (looks.empty())
    {
        looks = getLooks();
    }
    const StringVec& lookList = splitString(looks, ARRAY_VALID_SEPARATORS);
    LookVec activeLooks;
    if (!lookList.empty())
    {
        const LookVec& lookElements = getDocument()->getLooks();
        if (!lookElements.empty())
        {
            for (const string& lookName : lookList)
            {
                for (const auto& lookElement : lookElements)
                {
                    if (lookElement->getName() == lookName)
                    {
                        activeLooks.push_back(lookElement);
                    }
                }
            }
        }
    }
    return activeLooks;
}

void LookGroup::append(const LookGroupPtr& sourceGroup, const string& appendAfterLook)
{
    string sourceLooks = sourceGroup->getLooks();
    if (sourceLooks.empty())
    {
        return;
    }

    // If look already exists in the look list then append if
     // not skipping duplicates
    const StringVec& sourceLookList = splitString(sourceLooks, ARRAY_VALID_SEPARATORS);
    StringVec destLookList = splitString(getLooks(), ARRAY_VALID_SEPARATORS);
    const StringSet destLookSet(destLookList.begin(), destLookList.end());

    StringVec postList;

    // If inserting after a given look, then append up to that look
    // and then append the remainder after.
    if (!appendAfterLook.empty() && destLookSet.count(appendAfterLook))
    {
        StringVec prependList = destLookList;
        destLookList.clear();

        // Add destination looks first. 
        bool prepend = true;
        for (const string& lookName : prependList)
        {
            if (prepend)
            {
                destLookList.push_back(lookName);
            }
            else
            {
                postList.push_back(lookName);
            }
            if (lookName == appendAfterLook)
            {
                prepend = false;
            }
        }
    }

    // Append the source list
    for (const string& lookName : sourceLookList)
    {
        if (!destLookSet.count(lookName))
        {
            destLookList.push_back(lookName);
        }
    }

    // Append anything remaining from destination list
    if (postList.size())
    {
        for (const string& lookName : postList)
        {
            destLookList.push_back(lookName);
        }
    }

    // Append looks to "active" look list. Order does no matter.
    string activeSourceLooks = sourceGroup->getActiveLook();
    const StringVec& sourceActiveLookList = splitString(activeSourceLooks, ARRAY_VALID_SEPARATORS);
    StringVec destActiveLookList = splitString(getActiveLook(), ARRAY_VALID_SEPARATORS);
    const StringSet destActiveLookSet(destActiveLookList.begin(), destActiveLookList.end());

    for (const string& activeLookName : sourceActiveLookList)
    {
        if (!destActiveLookSet.count(activeLookName))
        {
            destActiveLookList.push_back(activeLookName);
        }
    }

    // Update look and active look lists
    setLooks(mergeStringVec(destLookList, ARRAY_VALID_SEPARATORS));
    setActiveLook(mergeStringVec(destActiveLookList, ARRAY_VALID_SEPARATORS));
}

LookPtr LookGroup::combineLooks() 
{
    DocumentPtr document = getDocument();
    LookVec looks = getActiveLooks();
    if (looks.empty())
    {
        return nullptr;
    }

    // Create a new look as a copy of the first look
    LookPtr mergedLook = document->addLook();
    mergedLook->copyContentFrom(looks[0]);

    // Merge in subsequent looks if any
    for (size_t i=1; i<looks.size(); i++)
    {
        mergedLook->append(looks[i]);
    }
    const string& mergedLookName = mergedLook->getName();
    setActiveLook(mergedLookName);
    setLooks(mergedLookName);

    return mergedLook;
}


} // namespace MaterialX
