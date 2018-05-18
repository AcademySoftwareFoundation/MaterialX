//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

namespace mx = MaterialX;

TEST_CASE("Geom strings", "[geom]")
{
    // Test for overlapping paths.
    REQUIRE(mx::geomStringsMatch("/", "/robot1"));
    REQUIRE(mx::geomStringsMatch("/robot1", "/robot1/left_arm"));
    REQUIRE(mx::geomStringsMatch("/robot1, /robot2", "/robot2/left_arm"));
    REQUIRE(!mx::geomStringsMatch("", "/robot1"));
    REQUIRE(!mx::geomStringsMatch("/robot1", "/robot2"));
    REQUIRE(!mx::geomStringsMatch("/robot1, /robot2", "/robot3"));

    // Test that one path contains another.
    REQUIRE(mx::geomStringsMatch("/", "/robot1", true));
    REQUIRE(!mx::geomStringsMatch("/robot1", "/", true));
}

TEST_CASE("Geom elements", "[geom]")
{
    mx::DocumentPtr doc = mx::createDocument();

    // Add geominfos and tokens
    mx::GeomInfoPtr geominfo1 = doc->addGeomInfo("geominfo1", "/robot1, /robot2");
    geominfo1->setTokenValue("asset", std::string("robot"));
    mx::GeomInfoPtr geominfo2 = doc->addGeomInfo("geominfo2", "/robot1");
    geominfo2->setTokenValue("id", std::string("01"));
    mx::GeomInfoPtr geominfo3 = doc->addGeomInfo("geominfo3", "/robot2");
    geominfo3->setTokenValue("id", std::string("02"));
    REQUIRE_THROWS_AS(doc->addGeomInfo("geominfo1"), mx::Exception&);

    // Create a node graph with a single image node.
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    nodeGraph->setFilePrefix("folder/");
    REQUIRE_THROWS_AS(doc->addNodeGraph(nodeGraph->getName()), mx::Exception&);
    mx::NodePtr image = nodeGraph->addNode("image");
    image->setParameterValue("file", std::string("%asset%id_diffuse_%UDIM.tif"), mx::FILENAME_TYPE_STRING);

    // Test filename string substitutions.
    mx::ParameterPtr fileParam = image->getParameter("file");
    mx::StringResolverPtr resolver1 = image->createStringResolver("/robot1");
    resolver1->setUdimString("1001");
    mx::StringResolverPtr resolver2 = image->createStringResolver("/robot2");
    resolver2->setUdimString("1002");
    REQUIRE(fileParam->getResolvedValueString(resolver1) == "folder/robot01_diffuse_1001.tif");
    REQUIRE(fileParam->getResolvedValueString(resolver2) == "folder/robot02_diffuse_1002.tif");

    // Create a base collection.
    mx::CollectionPtr collection1 = doc->addCollection("collection1");
    collection1->setIncludeGeom("/scene1");
    collection1->setExcludeGeom("/scene1/sphere2");
    REQUIRE(collection1->matchesGeomString("/scene1/sphere1"));
    REQUIRE(!collection1->matchesGeomString("/scene1/sphere2"));

    // Create a derived collection.
    mx::CollectionPtr collection2 = doc->addCollection("collection2");
    collection2->setIncludeCollection(collection1);
    REQUIRE(collection2->matchesGeomString("/scene1/sphere1"));
    REQUIRE(!collection2->matchesGeomString("/scene1/sphere2"));

    // Create and test an include cycle.
    collection1->setIncludeCollection(collection2);
    REQUIRE(!doc->validate());
    collection1->setIncludeCollection(nullptr);
    REQUIRE(doc->validate());

    // Test geometry string substitutions.
    collection1->setGeomPrefix("/root");
    REQUIRE(collection1->matchesGeomString("/root/scene1"));
    REQUIRE(!collection1->matchesGeomString("/root/scene2"));
}
