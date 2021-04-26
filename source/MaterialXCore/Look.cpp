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
const string LookGroup::ENABLED_ATTRIBUTE = "enabled";

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

LookVec LookGroup::getEnabledLooks() const
{
    string looks = getEnabledLooksString();
    if (looks.empty())
    {
        looks = getLooks();
    }
    const StringVec& lookList = splitString(looks, ARRAY_VALID_SEPARATORS);
    LookVec enabledLooks;
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
                        enabledLooks.push_back(lookElement);
                    }
                }
            }
        }
    }
    return enabledLooks;
}

static void appendLooks(StringVec& destLookList, const StringSet& destLookSet, const StringVec& sourceLookList, const string& appendAfterLook)
{
    StringVec postList;
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
    for (const string& lookName : sourceLookList)
    {
        if (!destLookSet.count(lookName))
        {
            destLookList.push_back(lookName);
        }
    }
    if (postList.size())
    {
        for (const string& lookName : postList)
        {
            destLookList.push_back(lookName);
        }
    }
}

void LookGroup::appendLookGroup(const LookGroupPtr& sourceGroup, const string& appendAfterLook)
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

    appendLooks(destLookList, destLookSet, sourceLookList, appendAfterLook);

    // Append looks to "active" look list. Order does no matter.
    string enabledSourceLooks = sourceGroup->getEnabledLooksString();
    const StringVec& sourceEnabledLookList = splitString(enabledSourceLooks, ARRAY_VALID_SEPARATORS);
    StringVec destEnabledLookList = splitString(getEnabledLooksString(), ARRAY_VALID_SEPARATORS);
    const StringSet destEnabledLookSet(destEnabledLookList.begin(), destEnabledLookList.end());

    for (const string& enabledLookName : sourceEnabledLookList)
    {
        if (!destEnabledLookSet.count(enabledLookName))
        {
            destEnabledLookList.push_back(enabledLookName);
        }
    }

    // Update look and active look lists
    setLooks(mergeStringVec(destLookList, ARRAY_VALID_SEPARATORS));
    setEnabledLooks(mergeStringVec(destEnabledLookList, ARRAY_VALID_SEPARATORS));
}


void LookGroup::appendLook(const string& sourceLook, const string& appendAfterLook)
{
    if (sourceLook.empty())
    {
        return;
    }

    // If look already exists in the look list then append if
    // not skipping duplicates
    const StringVec& sourceLookList { sourceLook };
    StringVec destLookList = splitString(getLooks(), ARRAY_VALID_SEPARATORS);
    const StringSet destLookSet(destLookList.begin(), destLookList.end());

    appendLooks(destLookList, destLookSet, sourceLookList, appendAfterLook);

    StringVec destEnabledLookList = splitString(getEnabledLooksString(), ARRAY_VALID_SEPARATORS);
    const StringSet destEnabledLookSet(destEnabledLookList.begin(), destEnabledLookList.end());

    if (destEnabledLookSet.count(sourceLook) == 0)
    {
        destEnabledLookList.push_back(sourceLook);
    }

    // Update look list
    setLooks(mergeStringVec(destLookList, ARRAY_VALID_SEPARATORS));
    setEnabledLooks(mergeStringVec(destEnabledLookList, ARRAY_VALID_SEPARATORS));
}

LookPtr LookGroup::combineLooks() 
{
    DocumentPtr document = getDocument();
    LookVec looks = getEnabledLooks();
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
    setEnabledLooks(mergedLookName);
    setLooks(mergedLookName);

    return mergedLook;
}


} // namespace MaterialX
