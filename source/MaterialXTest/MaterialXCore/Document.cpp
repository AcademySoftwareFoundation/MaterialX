//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/XmlIo.h>

namespace mx = MaterialX;

TEST_CASE("Document", "[document]")
{
    // Create a document.
    mx::DocumentPtr doc = mx::createDocument();

    // Create a node graph with a constant color output.
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr constant = nodeGraph->addNode("constant");
    constant->setInputValue("value", mx::Color3(0.5f));
    mx::OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);
    REQUIRE(doc->validate());

    // Create and test a type mismatch in a connection.
    output->setType("float");
    REQUIRE(!doc->validate());
    output->setType("color3");
    REQUIRE(doc->validate());

    // Test hierarchical name paths.
    REQUIRE(constant->getNamePath() == "nodegraph1/node1");
    REQUIRE(constant->getNamePath(nodeGraph) == "node1");

    // Test getting elements by path
    REQUIRE(doc->getDescendant("") == doc);
    REQUIRE(doc->getDescendant("nodegraph1") == nodeGraph);
    REQUIRE(doc->getDescendant("nodegraph1/node1") == constant);
    REQUIRE(doc->getDescendant("missingElement") == mx::ElementPtr());
    REQUIRE(doc->getDescendant("nodegraph1/missingNode") == mx::ElementPtr());
    REQUIRE(nodeGraph->getDescendant("") == nodeGraph);
    REQUIRE(nodeGraph->getDescendant("node1") == constant);
    REQUIRE(nodeGraph->getDescendant("missingNode") == mx::ElementPtr());

    // Create a simple shader interface.
    mx::NodeDefPtr simpleSrf = doc->addNodeDef("", "surfaceshader", "simpleSrf");
    simpleSrf->setInputValue("diffColor", mx::Color3(1.0f));
    simpleSrf->setInputValue("specColor", mx::Color3(0.0f));
    mx::InputPtr roughness = simpleSrf->setInputValue("roughness", 0.25f);
    REQUIRE(!roughness->getIsUniform());
    roughness->setIsUniform(true);
    REQUIRE(roughness->getIsUniform());

    // Instantiate shader and material nodes.
    mx::NodePtr shaderNode = doc->addNodeInstance(simpleSrf);
    mx::NodePtr materialNode = doc->addMaterialNode("", shaderNode);
    REQUIRE(materialNode->getUpstreamElement() == shaderNode);

    // Bind the diffuse color input to the constant color output.
    shaderNode->setConnectedOutput("diffColor", output);
    REQUIRE(shaderNode->getUpstreamElement() == constant);

    // Bind the roughness input to a value.
    mx::InputPtr instanceRoughness = shaderNode->setInputValue("roughness", 0.5f);
    REQUIRE(instanceRoughness->getValue()->asA<float>() == 0.5f);
    REQUIRE(instanceRoughness->getDefaultValue()->asA<float>() == 0.25f);

    // Create a collection 
    mx::CollectionPtr collection = doc->addCollection();
    REQUIRE(doc->getCollections().size() == 1);
    REQUIRE(doc->getCollection(collection->getName()));
    doc->removeCollection(collection->getName());
    REQUIRE(doc->getCollections().size() == 0);

    // Create a property set
    mx::PropertySetPtr propertySet = doc->addPropertySet();
    REQUIRE(doc->getPropertySets().size() == 1);
    REQUIRE(doc->getPropertySet(propertySet->getName()) != nullptr);
    doc->removePropertySet(propertySet->getName());
    REQUIRE(doc->getPropertySets().size() == 0);

    // Validate the document.
    REQUIRE(doc->validate());

    // Create a namespaced custom library.
    mx::DocumentPtr customLibrary = mx::createDocument();
    customLibrary->setNamespace("custom");
    mx::NodeGraphPtr customNodeGraph = customLibrary->addNodeGraph("NG_custom");
    mx::NodeDefPtr customNodeDef = customLibrary->addNodeDef("ND_simpleSrf", "surfaceshader", "simpleSrf");
    mx::ImplementationPtr customImpl = customLibrary->addImplementation("IM_custom");
    customNodeGraph->addNodeInstance(customNodeDef, "custom1");
    customImpl->setNodeDef(customNodeDef);
    REQUIRE(customLibrary->validate());

    // Import the custom library.
    doc->importLibrary(customLibrary);
    mx::NodeGraphPtr importedNodeGraph = doc->getNodeGraph("custom:NG_custom");
    mx::NodeDefPtr importedNodeDef = doc->getNodeDef("custom:ND_simpleSrf");
    mx::ImplementationPtr importedImpl = doc->getImplementation("custom:IM_custom");
    mx::NodePtr importedNode = importedNodeGraph->getNode("custom1");
    REQUIRE(importedNodeDef != nullptr);
    REQUIRE(importedNode->getNodeDef() == importedNodeDef);
    REQUIRE(importedImpl->getNodeDef() == importedNodeDef);

    // Validate the combined document.
    REQUIRE(doc->validate());
}

TEST_CASE("Version", "[document]")
{
    mx::DocumentPtr doc = mx::createDocument();
    mx::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/stdlib_defs.mtlx"), doc);
    mx::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/stdlib_ng.mtlx"), doc);
    mx::FileSearchPath searchPath("resources/Materials/TestSuite/stdlib/upgrade/");

    // 1.36 to 1.37
    {
        mx::readFromXmlFile(doc, "1_36_to_1_37.mtlx", searchPath);
        REQUIRE(doc->validate());

        mx::XmlWriteOptions writeOptions;
        writeOptions.writeXIncludeEnable = true;
        mx::writeToXmlFile(doc, "1_36_to_1_37_updated.mtlx", &writeOptions);

        mx::DocumentPtr doc2 = mx::createDocument();
        mx::readFromXmlFile(doc2, "1_36_to_1_37_updated.mtlx");
        REQUIRE(doc2->validate());

        // Check conversion to desired types occurred
        std::unordered_map<std::string, unsigned int> convertSet =
        {
            { "invertmatrix", 2},
            { "rotate2d", 1},
            { "rotate3d", 1},
            { "transformmatrix", 7},
            { "ifgreatereq", 7},
            { "separate2", 1},
            { "separate3", 1},
            { "separate4", 1},
            { "combine2", 1},
            { "combine3", 1},
            { "combine4", 1}
        };
        for (mx::NodePtr node : doc2->getNodes())
        {
            auto convertItem = convertSet.find(node->getCategory());
            if (convertItem != convertSet.end())
            {
                convertItem->second--;
            }
        }
        for (auto convertItem : convertSet)
        {
            REQUIRE((convertItem.second == 0));
        }
    }

    // 1.37 to 1.38
    {
        doc = mx::createDocument();
        mx::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/stdlib_defs.mtlx"), doc);
        mx::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/stdlib_ng.mtlx"), doc);
        mx::readFromXmlFile(doc, "1_37_to_1_38.mtlx", searchPath);
        REQUIRE(doc->validate());

        mx::XmlWriteOptions writeOptions;
        writeOptions.writeXIncludeEnable = false;
        mx::writeToXmlFile(doc, "1_37_to_1_38_updated.mtlx", &writeOptions);

        mx::DocumentPtr doc2 = mx::createDocument();
        mx::readFromXmlFile(doc2, "1_37_to_1_38_updated.mtlx");
        REQUIRE(doc2->validate());

        // atan2 test
        const std::string ATAN2 = "atan2";
        mx::StringMap ATAN2_MAP;
        ATAN2_MAP["in1"] = "in2";
        ATAN2_MAP["in2"] = "in1";

        for (mx::ElementPtr elem : doc->traverseTree())
        {
            mx::NodePtr node = elem->asA<mx::Node>();
            if (!node)
            {
                continue;
            }
            const std::string& nodeCategory = node->getCategory();
            if (nodeCategory == ATAN2)
            {
                const std::string &nodePath = node->getNamePath();
                for (auto in : ATAN2_MAP)
                {
                    mx::ElementPtr input = node->getChild(in.first);
                    if (input)
                    {
                        mx::ElementPtr newNode = doc2->getDescendant(nodePath);
                        REQUIRE((newNode && newNode->getChild(in.second)));
                    }
                }
            }
        }

        mx::NodeGraphPtr testNodeGraph = doc2->getNodeGraph("NG_Test");
        REQUIRE(!testNodeGraph->getNode("add"));
        REQUIRE(testNodeGraph->getNode("add1"));
        REQUIRE(testNodeGraph->getNode("add2"));
        REQUIRE(testNodeGraph->getNode("add2")->getInput("in1")->getInterfaceName() == "add");
        REQUIRE(testNodeGraph->getNode("add1")->getInput("in1")->getNodeName() == "add2");
    }
}

