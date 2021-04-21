//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>

namespace mx = MaterialX;

TEST_CASE("Look", "[look]")
{
    mx::DocumentPtr doc = mx::createDocument();

    // Create a material and look.
    mx::NodePtr shaderNode = doc->addNode("standard_surface", "", "surfaceshader");
    mx::NodePtr materialNode = doc->addMaterialNode("", shaderNode);
    mx::LookPtr look = doc->addLook();

    // Bind the material to a geometry string.
    mx::MaterialAssignPtr matAssign1 = look->addMaterialAssign("matAssign1", materialNode->getName());
    matAssign1->setGeom("/robot1");
    REQUIRE(matAssign1->getReferencedMaterial() == materialNode);
    REQUIRE(getGeometryBindings(materialNode, "/robot1").size() == 1);
    REQUIRE(getGeometryBindings(materialNode, "/robot2").size() == 0);

    // Bind the material to a geometric collection.
    mx::MaterialAssignPtr matAssign2 = look->addMaterialAssign("matAssign2", materialNode->getName());
    mx::CollectionPtr collection = doc->addCollection();
    collection->setIncludeGeom("/robot2");
    collection->setExcludeGeom("/robot2/left_arm");
    matAssign2->setCollection(collection);
    REQUIRE(getGeometryBindings(materialNode, "/robot2").size() == 1);
    REQUIRE(getGeometryBindings(materialNode, "/robot2/right_arm").size() == 1);
    REQUIRE(getGeometryBindings(materialNode, "/robot2/left_arm").size() == 0);

    // Create a property assignment.
    mx::PropertyAssignPtr propertyAssign = look->addPropertyAssign();
	propertyAssign->setProperty("twosided");
    propertyAssign->setGeom("/robot1");
    propertyAssign->setValue(true);
    REQUIRE(propertyAssign->getProperty() == "twosided");
    REQUIRE(propertyAssign->getGeom() == "/robot1");
    REQUIRE(propertyAssign->getValue()->isA<bool>());
    REQUIRE(propertyAssign->getValue()->asA<bool>() == true);

    // Create a property set assignment.
    mx::PropertySetPtr propertySet = doc->addPropertySet();
    propertySet->setPropertyValue("matte", false);
    REQUIRE(propertySet->getPropertyValue("matte")->isA<bool>());
    REQUIRE(propertySet->getPropertyValue("matte")->asA<bool>() == false);
    mx::PropertySetAssignPtr propertySetAssign = look->addPropertySetAssign();
	propertySetAssign->setPropertySet(propertySet);
    propertySetAssign->setGeom("/robot1");
    REQUIRE(propertySetAssign->getPropertySet() == propertySet);
    REQUIRE(propertySetAssign->getGeom() == "/robot1");
    
    // Create a variant set.
    mx::VariantSetPtr variantSet = doc->addVariantSet("damageVars");
    variantSet->addVariant("original");
    variantSet->addVariant("damaged");
    REQUIRE(variantSet->getVariants().size() == 2);

    // Create a visibility element.
    mx::VisibilityPtr visibility = look->addVisibility();
    REQUIRE(visibility->getVisible() == false);
    visibility->setVisible(true);
    REQUIRE(visibility->getVisible() == true);
    visibility->setGeom("/robot2");
    REQUIRE(visibility->getGeom() == "/robot2");
    visibility->setCollection(collection);
    REQUIRE(visibility->getCollection() == collection);

    // Create an inherited look.
    mx::LookPtr look2 = doc->addLook();
    look2->setInheritsFrom(look);
    REQUIRE(look2->getActivePropertySetAssigns().size() == 1);
    REQUIRE(look2->getActiveVisibilities().size() == 1);

    // Create and detect an inheritance cycle.
    look->setInheritsFrom(look2);
    REQUIRE(!doc->validate());
    look->setInheritsFrom(nullptr);
    REQUIRE(doc->validate());

    // Disconnect the inherited look.
    look2->setInheritsFrom(nullptr);
    REQUIRE(look2->getActivePropertySetAssigns().empty());
    REQUIRE(look2->getActiveVisibilities().empty());
}

TEST_CASE("LookGroup", "[look]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::LookGroupPtr lookGroup = doc->addLookGroup("lookgroup1");
    std::vector<mx::LookGroupPtr> lookGroups = doc->getLookGroups();
    REQUIRE(lookGroups.size() == 1);

    mx::StringMap mergeMap;

    const std::string looks = "look1,look2,look3,look4,look5";
    mx::StringVec looksVec = mx::splitString(looks, mx::ARRAY_VALID_SEPARATORS);
    for (const std::string& lookName : looksVec)
    {
        mx::LookPtr look = doc->addLook(lookName);
        look->addMaterialAssign("matA", "materialA");
        look->addMaterialAssign("matB", "materialB");
        look->addMaterialAssign("matC", "materialC");
        if (lookName != "look1")
        {
            mergeMap[lookName + "_matA"] = "materialA";
            mergeMap[lookName + "_matB"] = "materialB";
            mergeMap[lookName + "_matC"] = "materialC";
        }
        else
        {
            mergeMap["matA"] = "materialA";
            mergeMap["matB"] = "materialB";
            mergeMap["matC"] = "materialC";
        }
        REQUIRE(look != nullptr);
    }
    lookGroup->setLooks(looks);

    const std::string& looks2 = lookGroup->getLooks();
    mx::StringVec looksVec2 = mx::splitString(looks2, mx::ARRAY_VALID_SEPARATORS);
    REQUIRE(looksVec.size() == looksVec2.size());

    REQUIRE(lookGroup->getEnabledLooksString().empty());
    lookGroup->setEnabledLooks("look1");
    REQUIRE(lookGroup->getEnabledLooksString() == "look1");

    mx::LookGroupPtr copyLookGroup = doc->addLookGroup("lookgroup1_copy");
    copyLookGroup->copyContentFrom(lookGroup);
    mx::writeToXmlFile(doc, "looks_test.mtlx");

    // Combine looks in lookgroup test
    lookGroup->setEnabledLooks(looks);
    mx::LookPtr mergedLook = lookGroup->combineLooks();
    REQUIRE(mergedLook->getMaterialAssigns().size() == 15);
    for (auto ma : mergedLook->getMaterialAssigns())
    {
        REQUIRE(mergeMap.find(ma->getName()) != mergeMap.end());
        REQUIRE(mergeMap[ma->getName()] == ma->getMaterial());
    }

    mx::writeToXmlFile(doc, "looks_test_merged.mtlx");

    doc->removeLook(mergedLook->getName());
    REQUIRE(doc->getLooks().size() == 5);
    doc->removeLookGroup(lookGroup->getName());

    // Combine lookgroups test
    mx::LookGroupPtr lookGroup2 = doc->addLookGroup("lookgroup2");
    std::string lookGroup2_looks = "lookA,look1,lookC,look3,lookE";
    looksVec2 = mx::splitString(lookGroup2_looks, mx::ARRAY_VALID_SEPARATORS);
    for (const std::string& lookName2 : looksVec2)
    {
        // Skip duplcates
        if (doc->getLook(lookName2))
        {
            continue;
        }
        mx::LookPtr look = doc->addLook(lookName2);
        look->addMaterialAssign("matA", "materialA");
        look->addMaterialAssign("matB", "materialB");
        look->addMaterialAssign("matC", "materialC");
    }
    lookGroup2->setLooks(lookGroup2_looks);
    lookGroup2->setEnabledLooks("lookA,look3,lookE");

    // Append check
    mx::LookGroupPtr mergedCopyLookGroup = doc->addLookGroup("lookgroup1_copy_merged");
    mergedCopyLookGroup->copyContentFrom(copyLookGroup);
    mergedCopyLookGroup->appendLookGroup(lookGroup2);
    mx::writeToXmlFile(doc, "lookgroup_test_merged.mtlx");

    REQUIRE(mergedCopyLookGroup->getLooks() == std::string("look1, look2, look3, look4, look5, lookA, lookC, lookE"));
    REQUIRE(mergedCopyLookGroup->getEnabledLooksString() == std::string("look1, lookA, look3, lookE"));

    // Insert check
    mx::LookGroupPtr mergedCopyLookGroup2 = doc->addLookGroup("lookgroup1_copy_merged2");
    mergedCopyLookGroup2->copyContentFrom(copyLookGroup);
    mergedCopyLookGroup2->appendLookGroup(lookGroup2, std::string("look2"));
    mx::writeToXmlFile(doc, "lookgroup_test_merged2.mtlx");

    REQUIRE(mergedCopyLookGroup2->getLooks() == std::string("look1, look2, lookA, lookC, lookE, look3, look4, look5"));
    REQUIRE(mergedCopyLookGroup2->getEnabledLooksString() == std::string("look1, lookA, look3, lookE"));

    mx::LookGroupPtr mergedCopyLookGroup3 = doc->addLookGroup("lookgroup1_copy_merged3");
    mergedCopyLookGroup3->copyContentFrom(copyLookGroup);
    mergedCopyLookGroup3->appendLookGroup(lookGroup2, std::string("not found"));
    mx::writeToXmlFile(doc, "lookgroup_test_merge3.mtlx");

    REQUIRE(mergedCopyLookGroup2->getLooks() == std::string("look1, look2, lookA, lookC, lookE, look3, look4, look5"));
    REQUIRE(mergedCopyLookGroup2->getEnabledLooksString() == std::string("look1, lookA, look3, lookE"));

    doc->addLook("look6");
    mergedCopyLookGroup2->appendLook("look6", "lookC");
    REQUIRE(mergedCopyLookGroup2->getLooks() == std::string("look1, look2, lookA, lookC, look6, lookE, look3, look4, look5"));

    doc->removeLookGroup(lookGroup2->getName());
    doc->removeLookGroup(copyLookGroup->getName());
    doc->removeLookGroup(mergedCopyLookGroup->getName());
    doc->removeLookGroup(mergedCopyLookGroup2->getName());
    doc->removeLookGroup(mergedCopyLookGroup3->getName());
    lookGroups = doc->getLookGroups();
    REQUIRE(lookGroups.size() == 0);
}
