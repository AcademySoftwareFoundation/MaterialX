//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/Util.h>

#include <unordered_set>

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

    // 1. Find all materials referenced by material assignments.
    //    Tests both finding direct node connections as well as traversing through
    //    connected nodegraphs
    mx::StringSet assignmentMaterialPaths =
    {
        "top_level_material_assigned", // reference to material node from standard library
        "top_level_material_def_assigned", // reference to a custom material node
        "top_level_material_in_graph_assigned/surfacematerial1" // reference of material node in nodegraph
    };
    std::unordered_set<mx::ElementPtr> processedSources;
    std::vector<mx::InterfaceElementPtr> assignNodes;
    for (auto look : doc->getLooks())
    {
        for (auto materialAssign : look->getMaterialAssigns())
        {
            for (auto assignOutput : materialAssign->getMaterialOutputs())
            {
                mx::NodePtr assignNode = assignOutput->getConnectedNode();
                if (assignNode && !processedSources.count(assignNode))
                {
                    assignNodes.push_back(assignNode);
                    processedSources.insert(assignNode);
                }
            }
            mx::NodePtr referenceMaterial = materialAssign->getReferencedMaterial();
            if (referenceMaterial && !processedSources.count(referenceMaterial))
            {
                assignNodes.push_back(referenceMaterial);
            }
        }
    }
    for (auto assignNode : assignNodes)
    {
        const std::string& graphNodeName = assignNode->getNamePath();
        CHECK(assignmentMaterialPaths.count(graphNodeName));
        foundNames.insert(graphNodeName);
    }
    CHECK(assignmentMaterialPaths.size() == foundNames.size());
    foundNames.clear();

    // 2. Nodegraph level test
    //    Look for any material nodes exposed as outputs on the nodegraph. 
    //    Any other nodes are assumed to be "hidden" on purpose.
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
        std::vector<mx::InterfaceElementPtr> graphNodes;
        for (auto nodeGraphOutput : nodeGraph->getMaterialOutputs())
        {
            mx::NodePtr nodeGraphNode = nodeGraphOutput->getConnectedNode();
            if (nodeGraphNode)
            {
                graphNodes.push_back(nodeGraphNode);
            }
        }
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
    CHECK((compareGraphNodeNames.size() + compareImplGraphNodeNames.size()) == graphNodeNames.size());

    // 3. Document level test
    //    Find all material nodes at the top level or within top level nodegraphs
    mx::StringSet documentMaterialNodePaths = {
        "top_level_material_no_asssign",
        "top_level_material_assigned",
        "top_level_material_def",
        "top_level_material_def_assigned"
    };
    documentMaterialNodePaths.insert(compareGraphNodeNames.begin(), compareGraphNodeNames.end());
    processedSources.clear();
    std::vector<mx::InterfaceElementPtr> docNodes;
    if (doc)
    {
        for (auto documentOutput : doc->getMaterialOutputs())
        {
            mx::NodePtr documentNode = documentOutput->getConnectedNode();
            if (documentNode && !processedSources.count(documentNode))
            {
                docNodes.push_back(documentNode);
                processedSources.insert(documentNode);
            }
        }
        CHECK(!docNodes.empty());
        for (auto documentNode : doc->getMaterialNodes())
        {
            if (!processedSources.count(documentNode))
            {
                docNodes.push_back(documentNode);
                processedSources.insert(documentNode);
            }
        }
    }
    if (!docNodes.empty())
    {
        for (auto docNode : docNodes)
        {
            const std::string& nodeName = docNode->getNamePath();
            CHECK(documentMaterialNodePaths.count(nodeName));
            foundNames.insert(nodeName);
        }
        // Note: We are only comparing counts excluding any nodegraph
        // implementations which contain material nodes.
        CHECK(documentMaterialNodePaths.size() == foundNames.size());
    }
}