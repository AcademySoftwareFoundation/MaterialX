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

    // Add look and material.
    mx::LookPtr look = doc->addLook();
    mx::MaterialPtr material = doc->addMaterial();
    REQUIRE(doc->getLooks().size() == 1);
    REQUIRE(doc->getMaterials().size() == 1);

    // Add inherited look.
    mx::LookPtr look2 = doc->addLook();
    look2->setInheritsFrom(look);
    REQUIRE(look2->getInheritsFrom() == look);
    look2->setInheritsFrom(nullptr);
    REQUIRE(look2->getInheritsFrom() == nullptr);
    REQUIRE(look2->getLookInherits().empty());

    // Add material assignment.
    mx::MaterialAssignPtr materialAssign = look->addMaterialAssign("", material->getName());
    REQUIRE(material->getReferencingMaterialAssigns()[0] == materialAssign);
    REQUIRE(materialAssign->getReferencedMaterial() == material);
    materialAssign->setGeom("geom1");
    REQUIRE(materialAssign->getGeom() == "geom1");
    materialAssign->setCollection("collection1");
    REQUIRE(materialAssign->getCollection() == "collection1");
    materialAssign->setExclusive(true);
    REQUIRE(materialAssign->getExclusive());

    // Add property assignment.
    mx::PropertyAssignPtr propertyAssign = look->addPropertyAssign("twosided");
    propertyAssign->setGeom("geom1");
    propertyAssign->setValue(true);
    REQUIRE(propertyAssign->getGeom() == "geom1");
    REQUIRE(propertyAssign->getValue()->isA<bool>());
    REQUIRE(propertyAssign->getValue()->asA<bool>() == true);

    // Add property set assignment.
    mx::PropertySetPtr propertySet = doc->addPropertySet();
    REQUIRE(doc->getPropertySets().size() == 1);
    mx::PropertyPtr property = propertySet->addProperty("matte");
    property->setValue(false);
    REQUIRE(property->getValue()->isA<bool>());
    REQUIRE(property->getValue()->asA<bool>() == false);
    mx::PropertySetAssignPtr propertySetAssign = look->addPropertySetAssign(propertySet->getName());
    REQUIRE(look->getPropertySetAssigns().size() == 1);

    // Add visibility.
    mx::VisibilityPtr visibility = look->addVisibility();
    REQUIRE(look->getVisibilities().size() == 1);
    visibility->setGeom("geom1");
    REQUIRE(visibility->getGeom() == "geom1");
    visibility->setCollection("collection1");
    REQUIRE(visibility->getCollection() == "collection1");
}
