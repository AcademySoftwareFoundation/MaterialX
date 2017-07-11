//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

namespace mx = MaterialX;

TEST_CASE("Geom", "[geom]")
{
    mx::DocumentPtr doc = mx::createDocument();

    // Add geominfos and geomattrs
    mx::GeomInfoPtr geominfo1 = doc->addGeomInfo("geominfo1", "robot1,robot2");
    geominfo1->setGeomAttrValue("asset", std::string("robot"));
    mx::GeomInfoPtr geominfo2 = doc->addGeomInfo("geominfo2", "robot1");
    geominfo2->setGeomAttrValue("id", std::string("01"));
    mx::GeomInfoPtr geominfo3 = doc->addGeomInfo("geominfo3", "robot2");
    geominfo3->setGeomAttrValue("id", std::string("02"));
    REQUIRE_THROWS_AS(doc->addGeomInfo("geominfo1"), mx::Exception);

    // Test string substitutions.
    REQUIRE(doc->applyStringSubstitutions("%asset%id_diffuse.tif", "robot1") ==
                                        "robot01_diffuse.tif");
    REQUIRE(doc->applyStringSubstitutions("%asset%id_diffuse.tif", "robot2") ==
                                        "robot02_diffuse.tif");

    // Collection add / remove test
    mx::CollectionPtr collection = doc->addCollection("robot_collection"); 
    REQUIRE(doc->getCollections().size() == 1);
    mx::CollectionAddPtr collectadd1 = collection->addCollectionAdd("collection_add1");
    mx::CollectionRemovePtr collectremove1 = collection->addCollectionRemove("collection_remove1");
    REQUIRE(collection->getCollectionAdds().size() == 1);
    REQUIRE(collection->getCollectionRemoves().size() == 1);
    collection->removeCollectionAdd(collectadd1->getName());
    collection->removeCollectionRemove(collectremove1->getName());
    REQUIRE(collection->getCollectionAdds().size() == 0);
    REQUIRE(collection->getCollectionRemoves().size() == 0);
    doc->removeCollection(collection->getName());
    REQUIRE(doc->getCollections().size() == 0);
}
