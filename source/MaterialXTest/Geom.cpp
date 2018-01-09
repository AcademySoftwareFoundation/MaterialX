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
    mx::GeomInfoPtr geominfo1 = doc->addGeomInfo("geominfo1", "/robot1,/robot2");
    geominfo1->setGeomAttrValue("asset", std::string("robot"));
    mx::GeomInfoPtr geominfo2 = doc->addGeomInfo("geominfo2", "/robot1");
    geominfo2->setGeomAttrValue("id", std::string("01"));
    mx::GeomInfoPtr geominfo3 = doc->addGeomInfo("geominfo3", "/robot2");
    geominfo3->setGeomAttrValue("id", std::string("02"));
    REQUIRE_THROWS_AS(doc->addGeomInfo("geominfo1"), mx::Exception);

    // Create a node graph with a single image node.
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    nodeGraph->setFilePrefix("folder/");
    REQUIRE_THROWS_AS(doc->addNodeGraph(nodeGraph->getName()), mx::Exception);
    mx::NodePtr image = nodeGraph->addNode("image");
    image->setParameterValue("file", std::string("%asset%id_diffuse_%UDIM.tif"), mx::FILENAME_TYPE_STRING);

    // Test file name string substitutions.
    mx::ParameterPtr fileParam = image->getParameter("file");
    mx::StringResolverPtr resolver1 = image->createStringResolver("/robot1");
    resolver1->setUdimString("1001");
    mx::StringResolverPtr resolver2 = image->createStringResolver("/robot2");
    resolver2->setUdimString("1002");
    REQUIRE(fileParam->getResolvedValueString(resolver1) == "folder/robot01_diffuse_1001.tif");
    REQUIRE(fileParam->getResolvedValueString(resolver2) == "folder/robot02_diffuse_1002.tif");

    // Test geometry string substitutions.
    mx::CollectionPtr collection = doc->addCollection("collection1");
    mx::CollectionAddPtr collectionAdd = collection->addCollectionAdd("collectionAdd1");
    std::string geom = "|group1|sphere1";
    collectionAdd->setGeom(geom);
    collectionAdd->setGeomPrefix("/geomPrefix1");
    mx::StringResolverPtr resolver3 = collectionAdd->createStringResolver();
    resolver3->setGeomNameSubstitution("|", "/");
    REQUIRE(resolver3->resolve(collectionAdd->getGeom(), MaterialX::GEOMNAME_TYPE_STRING) == "/geomPrefix1/group1/sphere1");
}
