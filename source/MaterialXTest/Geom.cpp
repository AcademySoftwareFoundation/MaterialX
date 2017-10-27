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

    // Create a node graph with a single image node.
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    nodeGraph->setFilePrefix("folder/");
    REQUIRE_THROWS_AS(doc->addNodeGraph(nodeGraph->getName()), mx::Exception);
    mx::NodePtr image = nodeGraph->addNode("image");
    image->setParameterValue("file", std::string("%asset%id_diffuse_%UDIM.tif"), mx::FILENAME_TYPE_STRING);

    // Test string substitutions.
    mx::ParameterPtr fileParam = image->getParameter("file");
    mx::StringResolverPtr resolver1 = image->createStringResolver("robot1");
    resolver1->setUdimString("1001");
    mx::StringResolverPtr resolver2 = image->createStringResolver("robot2");
    resolver2->setUdimString("1002");
    REQUIRE(fileParam->getResolvedValueString(resolver1) == "folder/robot01_diffuse_1001.tif");
    REQUIRE(fileParam->getResolvedValueString(resolver2) == "folder/robot02_diffuse_1002.tif");

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
