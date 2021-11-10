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

namespace
{

// Utility function for testing material discovery
std::vector<mx::InterfaceElementPtr> getMaterials(mx::ElementPtr root, bool skipIncludes)
{
    std::vector<mx::InterfaceElementPtr> materialNodes;
    std::unordered_set<mx::ElementPtr> processedSources;

    // Look for any material nodes exposed as outputs
    // on the nodegraph. Any other nodes are assumed to 
    // be "hidden" on purpose.
    mx::NodeGraphPtr nodeGraph = root->asA<mx::NodeGraph>();
    if (nodeGraph)
    {
        for (auto nodeGraphOutput : nodeGraph->getMaterialOutputs())
        {
            mx::NodePtr nodeGraphNode = nodeGraphOutput->getConnectedNode();
            if (nodeGraphNode && !processedSources.count(nodeGraphNode))
            {
                materialNodes.push_back(nodeGraphNode);
                processedSources.insert(nodeGraphNode);
            }
        }
    }

    // Find all nodes which have a "material" output at the document level
    mx::DocumentPtr document = root->asA<mx::Document>();
    if (document)
    {
        for (auto documentOutput : document->getMaterialOutputs(skipIncludes))
        {
            mx::NodePtr documentNode = documentOutput->getConnectedNode();
            if (documentNode && !processedSources.count(documentNode))
            {
                materialNodes.push_back(documentNode);
                processedSources.insert(documentNode);
            }
        }
        for (auto documentNode : document->getMaterialNodes())
        {
            if (!processedSources.count(documentNode))
            {
                materialNodes.push_back(documentNode);
                processedSources.insert(documentNode);
            }
        }
    }

    // Look for materials associated with a material assignment
    mx::MaterialAssignPtr materialAssign = root->asA<mx::MaterialAssign>();
    if (materialAssign)
    {
        for (auto assignOutput : materialAssign->getMaterialOutputs())
        {
            mx::NodePtr assignNode = assignOutput->getConnectedNode();
            if (assignNode && !processedSources.count(assignNode))
            {
                materialNodes.push_back(assignNode);
                processedSources.insert(assignNode);
            }
        }
        mx::NodePtr referenceMaterial = materialAssign->getReferencedMaterial();
        if (referenceMaterial && !processedSources.count(referenceMaterial))
        {
            materialNodes.push_back(referenceMaterial);
        }
    }

    return materialNodes;
}

}

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
            std::vector<mx::InterfaceElementPtr> assignNodes = getMaterials(materialAssign, false);
            for (auto assignNode : assignNodes)
            {
                if (assignNode && assignNode->isA<mx::NodeGraph>())
                {
                    std::vector<mx::InterfaceElementPtr> assignGraphNodes = getMaterials(assignNode, false);
                    for (auto assignGraphNode : assignGraphNodes)
                    {
                        const std::string& graphNodeName = assignGraphNode->getNamePath();
                        CHECK(assignmentMaterialPaths.count(graphNodeName));
                        foundNames.insert(graphNodeName);
                    }
                }
                else
                {
                    const std::string& graphNodeName = assignNode->getNamePath();
                    CHECK(assignmentMaterialPaths.count(graphNodeName));
                    foundNames.insert(graphNodeName);
                }
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
        std::vector<mx::InterfaceElementPtr> graphNodes = getMaterials(nodeGraph, false);
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
    std::vector<mx::InterfaceElementPtr> docNodes = getMaterials(doc, false);
    if (!docNodes.empty())
    {
        for (auto docNode : docNodes)
        {
            if (docNode->isA<mx::NodeGraph>())
            {
                std::vector<mx::InterfaceElementPtr> docGraphNodes  = getMaterials(docNode, false);
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
        // Note: We are only comparing counts excluding any nodegraph
        // implementations which contain material nodes.
        CHECK(documentMaterialNodePaths.size() == foundNames.size());
    }
}