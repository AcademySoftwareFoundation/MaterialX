//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/XmlIo.h>

namespace mx = MaterialX;

TEST_CASE("Document", "[document]")
{
    // Create a document.
    mx::DocumentPtr doc = mx::createDocument();

    // Test version strings.
    REQUIRE(mx::stringStartsWith(mx::getVersionString(), doc->getVersionString()));

    // Test version integers.
    REQUIRE(doc->getVersionIntegers().first == std::get<0>(mx::getVersionIntegers()));
    REQUIRE(doc->getVersionIntegers().second == std::get<1>(mx::getVersionIntegers()));

    // Create a node graph with a constant color output.
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr constant = nodeGraph->addNode("constant");
    constant->setInputValue("value", mx::Color3(0.5f));
    mx::OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);
    REQUIRE(output->isColorType());
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
    mx::NodeDefPtr simpleSrf = doc->addNodeDef("", mx::SURFACE_SHADER_TYPE_STRING, "simpleSrf");
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
    mx::NodeDefPtr customNodeDef = customLibrary->addNodeDef("ND_simpleSrf", mx::SURFACE_SHADER_TYPE_STRING, "simpleSrf");
    mx::ImplementationPtr customImpl = customLibrary->addImplementation("IM_custom");
    customNodeGraph->addNodeInstance(customNodeDef, "custom1");
    customImpl->setNodeDef(customNodeDef);
    REQUIRE(customLibrary->validate());

    // Import the custom library.
    mx::DocumentPtr customDatalibrary = mx::createDocument();
    customDatalibrary->importLibrary(customLibrary);

    // Set data library
    doc->setDataLibrary(customDatalibrary);
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

TEST_CASE("Document equivalence", "[document]")
{
    mx::DocumentPtr doc = mx::createDocument();
    std::unordered_multimap<std::string, std::string> inputMap;

    inputMap.insert({ "color3", "  1.0,   +2.0,  3.0   " });
    inputMap.insert({ "color4", "1.0,   2.00, 0.3000, -4" });
    inputMap.insert({ "integer", "  12 " });
    inputMap.insert({ "matrix33",
                      "01.0,         2.0,  0000.2310, "
                      "   01.0,         2.0,  0000.2310, "
                      "01.0,         2.0,  0000.2310       " });
    inputMap.insert({ "matrix44",
                      "01.0,         2.0,  0000.2310, 0.100, "
                      "01.0,         2.0,  0000.2310, 0.100, "
                      "01.0,         2.0,  0000.2310, 0.100, "
                      "01.0,         2.0,  0000.2310, 0.100" });
    inputMap.insert({ "vector2", "1.0,   0.012345608" }); // For precision check
    inputMap.insert({ "vector3", "  1.0,   +2.0,  3.0   " });
    inputMap.insert({ "vector4", "1.0,   2.00, 0.3000, -4" });
    inputMap.insert({ "string", "mystring" });
    inputMap.insert({ "boolean", "false" });
    inputMap.insert({ "filename", "filename1" });
    inputMap.insert({ "float", "  1.2e-10  " });
    inputMap.insert({ "float", "  00.1000  " });

    unsigned int index = 0;
    mx::ElementPtr child = doc->addNodeGraph("mygraph");
    mx::NodeGraphPtr graph = child->asA<mx::NodeGraph>();
    // Add comment block at the start of the first doc to check skipping
    mx::ElementPtr comment = doc->addChildOfCategory(mx::CommentElement::CATEGORY);
    comment->setDocString("Comment 1");
    for (auto it = inputMap.begin(); it != inputMap.end(); ++it)
    {
        const std::string inputType = (*it).first;
        mx::InputPtr input = graph->addInput("input_" + std::to_string(index), inputType);
        if (inputType == "float")
        {
            input->setAttribute(mx::ValueElement::UI_MIN_ATTRIBUTE, "  0.0100 ");
            input->setAttribute(mx::ValueElement::UI_MAX_ATTRIBUTE, "  01.0100 ");
            index++;
        }
        else
        {
            input->setName("input_" + inputType); // Set by name for difference in order test
        }
        input->setValueString((*it).second);
    }

    mx::DocumentPtr doc2 = mx::createDocument();
    std::unordered_multimap<std::string, std::string> inputMap2;
    inputMap2.insert({ "color4", "1, 2, 0.3, -4" }); 
    inputMap2.insert({ "integer", "12" });
    inputMap2.insert({ "matrix33", "1, 2, 0.231,  1, 2, 0.231,  1, 2, 0.231,  1, 2, 0.231" });
    inputMap2.insert({ "matrix44", "1, 2, 0.231, 0.1, 1, 2, 0.231, 0.1, 1, 2, 0.231, 0.1, 1, 2, 0.231, 0.1" });
    inputMap2.insert({ "vector2", "1, 0.012345611" }); // For precision check
    inputMap2.insert({ "string", "mystring" });
    inputMap2.insert({ "boolean", "false" });
    inputMap2.insert({ "color3", "1, 2, 3" });
    inputMap2.insert({ "vector3", "1, 2, 3" });
    inputMap2.insert({ "vector4", "1, 2, 0.3, -4" });
    inputMap2.insert({ "filename", "filename1" });
    inputMap2.insert({ "float", "1.2e-10" });
    inputMap2.insert({ "float", "0.1" });

    index = 0;
    child = doc2->addNodeGraph("mygraph");
    graph = child->asA<mx::NodeGraph>();
    std::vector<mx::InputPtr> floatInputs;
    for (auto it = inputMap2.begin(); it != inputMap2.end(); ++it)
    {
        const std::string inputType = (*it).first;
        mx::InputPtr input = graph->addInput("input_" + std::to_string(index), inputType);
        // Note: order of value and ui attributes is different for ordering comparison
        input->setValueString((*it).second);
        if (inputType == "float")
        {
            input->setAttribute(mx::ValueElement::UI_MIN_ATTRIBUTE, "  0.01");
            input->setAttribute(mx::ValueElement::UI_MAX_ATTRIBUTE, "  1.01");
            floatInputs.push_back(input);
            index++;
        }
        else
        {
            input->setName("input_" + inputType);
        }
    }
    // Add comment blocks at end of second doc to check value and count checks
    comment = doc2->addChildOfCategory(mx::CommentElement::CATEGORY);
    comment->setDocString("Comment 2");
    comment = doc2->addChildOfCategory(mx::CommentElement::CATEGORY);
    comment->setDocString("Comment 3");

    mx::ElementEquivalenceOptions options;
    std::string message;

    // Check that this fails when not performing value comparisons
    options.performValueComparisons = false;
    bool equivalent = doc->isEquivalent(doc2, options, &message);
    REQUIRE(!equivalent);

    // Check attribute values 
    options.performValueComparisons = true;
    equivalent = doc->isEquivalent(doc2, options, &message);
    REQUIRE(equivalent);

    unsigned int currentPrecision = mx::Value::getFloatPrecision();
    // This will compare 0.012345608 versus: 1, 0.012345611 for input10
    options.floatPrecision = 8;
    equivalent = doc->isEquivalent(doc2, options);
    REQUIRE(!equivalent);
    options.floatPrecision = currentPrecision;

    // Check attribute filtering of inputs
    options.attributeExclusionList = { mx::ValueElement::UI_MIN_ATTRIBUTE, mx::ValueElement::UI_MAX_ATTRIBUTE };
    for (mx::InputPtr floatInput : floatInputs)
    {
        floatInput->setAttribute(mx::ValueElement::UI_MIN_ATTRIBUTE, "0.9");
        floatInput->setAttribute(mx::ValueElement::UI_MAX_ATTRIBUTE, "100.0");
    }
    equivalent = doc->isEquivalent(doc2, options, &message);
    REQUIRE(equivalent);
    for (mx::InputPtr floatInput : floatInputs)
    {
        floatInput->setAttribute(mx::ValueElement::UI_MIN_ATTRIBUTE, "  0.01");
        floatInput->setAttribute(mx::ValueElement::UI_MAX_ATTRIBUTE, "  1.01");
    }

    // Check for child name mismatch
    mx::ElementPtr mismatchElement = doc->getDescendant("mygraph/input_color4");
    std::string previousName = mismatchElement->getName();
    mismatchElement->setName("mismatch_color4");
    equivalent = doc->isEquivalent(doc2, options, &message);
    REQUIRE(!equivalent);
    mismatchElement->setName(previousName);
    equivalent = doc->isEquivalent(doc2, options, &message);
    REQUIRE(equivalent);

    // Check for functional nodegraphs
    mx::NodeGraphPtr nodeGraph = doc->getNodeGraph("mygraph");
    REQUIRE(nodeGraph);
    doc->addNodeDef("ND_mygraph");
    nodeGraph->setNodeDefString("ND_mygraph");
    mx::NodeGraphPtr nodeGraph2 = doc2->getNodeGraph("mygraph");
    REQUIRE(nodeGraph2);
    doc2->addNodeDef("ND_mygraph");
    nodeGraph2->setNodeDefString("ND_mygraph");
    equivalent = doc->isEquivalent(doc2, options, &message);
    REQUIRE(!equivalent);
}
