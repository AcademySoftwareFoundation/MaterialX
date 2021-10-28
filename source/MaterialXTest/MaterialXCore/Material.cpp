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

    mx::StringSet foundNames;

    // Material assignment test
    mx::StringSet assignmentMaterialPaths =
    {
        "top_level_material_assigned", // material node at top level
        "top_level_material_in_graph_assigned/surfacematerial1", // material node in nodegraph
        "top_level_material_def_assigned" // custom material at top level
    };
    for (auto look : doc->getLooks())
    {
        for (auto materialAssign : look->getMaterialAssigns())
        {
            std::vector<mx::InterfaceElementPtr> assignNodes = mx::getMaterialNodes(materialAssign, false);
            for (auto assignNode : assignNodes)
            {
                const std::string& graphNodeName = assignNode->getNamePath();
                CHECK(assignmentMaterialPaths.count(graphNodeName));
                foundNames.insert(graphNodeName);
            }
        }
    }
    CHECK(assignmentMaterialPaths.size() == foundNames.size());
    foundNames.clear();

    // Nodegraph level test
    mx::StringSet compareGraphNodeNames =
    {
        "top_level_material_in_graph_no_assign/surfacematerial1", // Material in a nodegraph
        "top_level_material_in_graph_assigned/surfacematerial1", // Material in a nodegraph
    };
    mx::StringSet compareImplGraphNodeNames = 
    {
        "NG_material_def_material_1_0/surfacematerial1" // Material in a nodegraph implementation
    };
    mx::StringSet graphNodeNames;
    for (auto nodeGraph : doc->getNodeGraphs())
    {
        std::vector<mx::InterfaceElementPtr> graphNodes = mx::getMaterialNodes(nodeGraph, false);
        if (!graphNodes.empty())
        {
            for (auto graphNode : graphNodes)
            {
                const std::string& nodeName = graphNode->getNamePath();
                CHECK((compareGraphNodeNames.count(nodeName) || 
                       compareImplGraphNodeNames.count(nodeName)));
                graphNodeNames.insert(nodeName);
            }
        }
    }
    CHECK((compareGraphNodeNames.size()+ compareImplGraphNodeNames.size()) == graphNodeNames.size());

    // Document level test
    mx::StringSet documentMaterialNodePaths = {
        "top_level_material_no_asssign",
        "top_level_material_assigned",
        "top_level_material_def",
        "top_level_material_def_assigned"
    };
    documentMaterialNodePaths.insert(compareGraphNodeNames.begin(), compareGraphNodeNames.end());
    std::vector<mx::InterfaceElementPtr> docNodes = mx::getMaterialNodes(doc, false);
    if (!docNodes.empty())
    {
        for (auto docNode : docNodes)
        {
            if (docNode->asA<mx::NodeGraph>())
            {
                std::vector<mx::InterfaceElementPtr> docGraphNodes  = mx::getMaterialNodes(docNode, false);
                for (auto docGraphNode : docGraphNodes)
                {
                    // Check that we find the same node names as by directly scanning nodegraphs
                    // above
                    const std::string& nodeName = docGraphNode->getNamePath();
                    CHECK(graphNodeNames.count(nodeName));
                    foundNames.insert(nodeName);
                }
            }
            else
            {
                const std::string& nodeName = docNode->getNamePath();
                CHECK(documentMaterialNodePaths.count(nodeName));
                foundNames.insert(nodeName);
            }
        }
        // Note: We are only comparing counts excluding the nodegraph
        // implementation which contains material node.
        CHECK(documentMaterialNodePaths.size() == foundNames.size());
    }
}