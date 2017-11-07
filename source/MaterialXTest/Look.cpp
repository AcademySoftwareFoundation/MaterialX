//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

namespace mx = MaterialX;

TEST_CASE("Look", "[look]")
{
    mx::DocumentPtr doc = mx::createDocument();

    // Create a material and look.
    mx::MaterialPtr material = doc->addMaterial("material1");
    mx::LookPtr look = doc->addLook("look1");
    REQUIRE(doc->getMaterials().size() == 1);
    REQUIRE(doc->getLooks().size() == 1);

    // Bind the material to a geometry string.
    mx::MaterialAssignPtr matAssign1 = look->addMaterialAssign("matAssign1", material->getName());
    matAssign1->setGeom("/robot1");
    REQUIRE(matAssign1->getReferencedMaterial() == material);
    REQUIRE(material->getBoundGeomStrings()[0] == "/robot1");

    // Bind the material to a geometric collection.
    mx::MaterialAssignPtr matAssign2 = look->addMaterialAssign("matAssign2", material->getName());
    mx::CollectionPtr collection = doc->addCollection();
    mx::CollectionAddPtr collectionAdd = collection->addCollectionAdd();
    collectionAdd->setGeom("/robot2");
    mx::CollectionRemovePtr collectionRemove = collection->addCollectionRemove();
    collectionRemove->setGeom("/robot2/left_arm");
    matAssign2->setCollection(collection);
    REQUIRE(material->getBoundGeomCollections()[0] == collection);

    // Create a property assignment.
    mx::PropertyAssignPtr propertyAssign = look->addPropertyAssign("twosided");
    propertyAssign->setGeom("/robot1");
    propertyAssign->setValue(true);
    REQUIRE(propertyAssign->getGeom() == "/robot1");
    REQUIRE(propertyAssign->getValue()->isA<bool>());
    REQUIRE(propertyAssign->getValue()->asA<bool>() == true);

    // Create a property set assignment.
    mx::PropertySetPtr propertySet = doc->addPropertySet();
    REQUIRE(doc->getPropertySets().size() == 1);
    mx::PropertyPtr property = propertySet->addProperty("matte");
    property->setValue(false);
    REQUIRE(property->getValue()->isA<bool>());
    REQUIRE(property->getValue()->asA<bool>() == false);
    mx::PropertySetAssignPtr propertySetAssign = look->addPropertySetAssign(propertySet->getName());
    REQUIRE(look->getPropertySetAssigns().size() == 1);

    // Create a visibility element.
    mx::VisibilityPtr visibility = look->addVisibility();
    REQUIRE(look->getVisibilities().size() == 1);
    visibility->setGeom("/robot2");
    REQUIRE(visibility->getGeom() == "/robot2");
    visibility->setCollection(collection);
    REQUIRE(visibility->getCollection() == collection);

    // Create an inherited look.
    mx::LookPtr look2 = doc->addLook();
    look2->setInheritsFrom(look);
    REQUIRE(look2->getInheritsFrom() == look);
    look2->setInheritsFrom(nullptr);
    REQUIRE(look2->getInheritsFrom() == nullptr);
    REQUIRE(look2->getLookInherits().empty());
}
