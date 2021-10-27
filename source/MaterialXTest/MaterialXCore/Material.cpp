//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/Util.h>

namespace mx = MaterialX;

TEST_CASE("Material", "[material]")
{
    mx::DocumentPtr doc = mx::createDocument();

    // Create a base shader nodedef.
    mx::NodeDefPtr simpleSrf = doc->addNodeDef("ND_simpleSrf", "surfaceshader", "simpleSrf");
    simpleSrf->setInputValue("diffColor", mx::Color3(1.0f));
    simpleSrf->setInputValue("specColor", mx::Color3(0.0f));
    simpleSrf->setInputValue("roughness", 0.25f);
    simpleSrf->setTokenValue("texId", "01");
    REQUIRE(simpleSrf->getInputValue("diffColor")->asA<mx::Color3>() == mx::Color3(1.0f));
    REQUIRE(simpleSrf->getInputValue("specColor")->asA<mx::Color3>() == mx::Color3(0.0f));
    REQUIRE(simpleSrf->getInputValue("roughness")->asA<float>() == 0.25f);
    REQUIRE(simpleSrf->getTokenValue("texId") == "01");

    // Create an inherited shader nodedef.
    mx::NodeDefPtr anisoSrf = doc->addNodeDef("ND_anisoSrf", "surfaceshader", "anisoSrf");
    anisoSrf->setInheritsFrom(simpleSrf);
    anisoSrf->setInputValue("anisotropy", 0.0f);
    REQUIRE(anisoSrf->getInheritsFrom() == simpleSrf);

    // Instantiate shader and material nodes.
    mx::NodePtr shaderNode = doc->addNode(anisoSrf->getNodeString(), "", anisoSrf->getType());
    mx::NodePtr materialNode = doc->addMaterialNode("", shaderNode);
    REQUIRE(materialNode->getUpstreamElement() == shaderNode);
    REQUIRE(shaderNode->getNodeDef() == anisoSrf);

    // Set nodedef and shader node qualifiers.
    shaderNode->setVersionString("2.0");
    REQUIRE(shaderNode->getNodeDef() == nullptr);
    anisoSrf->setVersionString("2");
    shaderNode->setVersionString("2");
    REQUIRE(shaderNode->getNodeDef() == anisoSrf);
    shaderNode->setType("volumeshader");
    REQUIRE(shaderNode->getNodeDef() == nullptr);
    shaderNode->setType("surfaceshader");
    REQUIRE(shaderNode->getNodeDef() == anisoSrf);

    // Bind a shader input to a value.
    mx::InputPtr instanceSpecColor = shaderNode->setInputValue("specColor", mx::Color3(1.0f));
    REQUIRE(instanceSpecColor->getValue()->asA<mx::Color3>() == mx::Color3(1.0f));
    REQUIRE(instanceSpecColor->getDefaultValue()->asA<mx::Color3>() == mx::Color3(0.0f));
    REQUIRE(doc->validate());
}

#include <iostream>

TEST_CASE("Material Discovery", "[material]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FileSearchPath searchPath;
    const mx::FilePath currentPath = mx::FilePath::getCurrentPath();
    searchPath.append(currentPath / mx::FilePath("libraries"));
    searchPath.append(currentPath / mx::FilePath("resources/Materials/TestSuite"));

    searchPath.append(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    loadLibraries({ "targets", "stdlib", "pbrlib" }, searchPath, doc);

    mx::FilePath filename = "stdlib/materials/material_node_discovery.mtlx";
    mx::readFromXmlFile(doc, filename, searchPath);


    // Material assignment test
    for (auto look : doc->getLooks())
    {
        for (auto materialAssign : look->getMaterialAssigns())
        {
            std::vector<mx::NodePtr> assignNodes = mx::getMaterialNodes(materialAssign, false, false);
            if (!assignNodes.empty())
            {
                std::cout << "*** Material node for assignment: " << materialAssign->getNamePath() << std::endl;
                for (auto n : assignNodes)
                {
                    std::cout << "Found material node: " << n->getNamePath() << std::endl;
                }
            }
        }
    }

    // Nodegraph level test
    for (auto nodeGraph : doc->getNodeGraphs())
    {
        std::vector<mx::NodePtr> connectedGraphNodes = mx::getMaterialNodes(nodeGraph, false, false);
        if (!connectedGraphNodes.empty())
        {
            std::cout << "*** Connected nodes for graph: " << nodeGraph->getNamePath() << std::endl;
            for (auto n : connectedGraphNodes)
            {
                std::cout << "Found output material node: " << n->getNamePath() << std::endl;
            }
        }
        std::vector<mx::NodePtr> unconnectedGraphNodes = mx::getMaterialNodes(nodeGraph, true, false);
        if (!unconnectedGraphNodes.empty())
        {
            std::cout << "*** UN-Connected nodes for graph: " << nodeGraph->getNamePath() << std::endl;
            for (auto n : unconnectedGraphNodes)
            {
                std::cout << "Found output material node: " << n->getNamePath() << std::endl;
            }
        }
    }

    // Document level test
    std::vector<mx::NodePtr> connectedDocNodes = mx::getMaterialNodes(doc, false, false);
    if (!connectedDocNodes.empty())
    {
        std::cout << "*** Connected nodes for document: " << std::endl;
        for (auto n : connectedDocNodes)
        {
            std::cout << "Found output material node: " << n->getNamePath() << std::endl;
        }
    }
    std::vector<mx::NodePtr> unconnectedDocNodes = mx::getMaterialNodes(doc, true, false);
    if (!unconnectedDocNodes.empty())
    {
        std::cout << "*** UN-Connected nodes for document: " << std::endl;
        for (auto n : unconnectedDocNodes)
        {
            std::cout << "Found output material node: " << n->getNamePath() << std::endl;
        }
    }
}