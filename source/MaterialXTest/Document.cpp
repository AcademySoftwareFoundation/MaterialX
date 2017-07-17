//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

namespace mx = MaterialX;

TEST_CASE("Document", "[document]")
{
    // Create a document.
    mx::DocumentPtr doc = mx::createDocument();

    // Create a node graph with a constant color output.
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr constant = nodeGraph->addNode("constant");
    constant->setParameterValue("value", mx::Color3(0.5f, 0.5f, 0.5f));
    mx::OutputPtr output = nodeGraph->addOutput();
    output->setConnectedNode(constant);
    REQUIRE(doc->validate());

    // Create and test a type mismatch.
    output->setType("float");
    REQUIRE(!doc->validate());
    output->setType("color3");
    REQUIRE(doc->validate());

    // Test hierarchical name paths.
    REQUIRE(constant->getNamePath() == "nodegraph1/node1");
    REQUIRE(constant->getNamePath(nodeGraph) == "node1");

    // Create a simple shader interface.
    mx::NodeDefPtr shader = doc->addNodeDef("", "surfaceshader", "simpleSrf");
    mx::InputPtr diffColor = shader->addInput("diffColor", "color3");
    mx::InputPtr specColor = shader->addInput("specColor", "color3");
    mx::ParameterPtr roughness = shader->addParameter("roughness", "float");

    // Create a material that instantiates the shader.
    mx::MaterialPtr material = doc->addMaterial();
    mx::ShaderRefPtr shaderRef = material->addShaderRef("", "simpleSrf");

    // Bind the diffuse color input to the constant color output.
    mx::BindInputPtr bindInput = shaderRef->addBindInput("diffColor");
    bindInput->setConnectedOutput(output);
    REQUIRE(diffColor->getUpstreamElement(material) == output);

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

    // Generate and verify require string.
    doc->generateRequireString();
    REQUIRE(doc->getRequireString().find(mx::Document::REQUIRE_STRING_MATNODEGRAPH) != std::string::npos);

    // Validate the document.
    REQUIRE(doc->validate());

    // Copy and reorder the document.
    mx::DocumentPtr doc2 = doc->copy();
    REQUIRE(*doc2 == *doc);
    int origIndex = doc2->getChildIndex(shader->getName());
    doc2->setChildIndex(shader->getName(), origIndex + 1);
    REQUIRE(*doc2 != *doc);
    doc2->setChildIndex(shader->getName(), origIndex);
    REQUIRE(*doc2 == *doc);
    REQUIRE_THROWS_AS(doc2->setChildIndex(shader->getName(), 100), mx::Exception);

    // Create and test an orphaned element.
    mx::ElementPtr orphan;
    {
        mx::DocumentPtr doc3 = doc->copy();
        for (mx::ElementPtr elem : doc3->traverseTree())
        {
            if (elem->isA<mx::Node>("constant"))
            {
                orphan = elem;
            }
        }
        REQUIRE(orphan);
    }
    REQUIRE_THROWS_AS(orphan->getDocument(), mx::ExceptionOrphanedElement);    

    // Test element renaming
    mx::NodeGraphPtr nodeGraph2 = doc->addNodeGraph();
    mx::NodePtr foo = nodeGraph2->addNode("constant", "foo");
    nodeGraph2->renameChild(foo->getName(), "bar");
    REQUIRE(foo->getName() == "bar");
    REQUIRE(nodeGraph2->getNode("bar") == foo);
    REQUIRE(nodeGraph2->getNode("foo") == nullptr);
}
