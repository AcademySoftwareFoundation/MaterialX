//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Util.h>
#include <MaterialXCore/Document.h>

namespace mx = MaterialX;

TEST_CASE("String utilities", "[util]")
{
    REQUIRE(mx::createValidName("test.name.1") == "test_name_1");
    REQUIRE(mx::createValidName("test*name>2") == "test_name_2");
    REQUIRE(mx::createValidName("testName...") == "testName___");

    REQUIRE(mx::incrementName("testName") == "testName2");
    REQUIRE(mx::incrementName("testName0") == "testName1");
    REQUIRE(mx::incrementName("testName99") == "testName100");
    REQUIRE(mx::incrementName("1testName1") == "1testName2");
    REQUIRE(mx::incrementName("41") == "42");

    REQUIRE(mx::splitString("robot1, robot2", ", ") == (std::vector<std::string>{"robot1", "robot2"}));
    REQUIRE(mx::splitString("[one...two...three]", "[.]") == (std::vector<std::string>{"one", "two", "three"}));
}

TEST_CASE("Print utilities", "[util]")
{
    // Create a document.
    mx::DocumentPtr doc = mx::createDocument();

    // Create a node graph with the following structure:
    //
    //   [constant1] [constant2]      [image2]
    //           \   /          \    /
    // [image1] [add1]          [add2]
    //        \  /   \______      |   
    //    [multiply]        \__ [add3]         [noise3d]
    //             \____________  |  ____________/
    //                          [mix]
    //                            |
    //                         [output]
    //
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr image1 = nodeGraph->addNode("image", "image1");
    mx::NodePtr image2 = nodeGraph->addNode("image", "image2");
    mx::NodePtr multiply = nodeGraph->addNode("multiply", "multiply");
    mx::NodePtr constant1 = nodeGraph->addNode("constant", "constant1");
    mx::NodePtr constant2 = nodeGraph->addNode("constant", "constant2");
    mx::NodePtr add1 = nodeGraph->addNode("add", "add1");
    mx::NodePtr add2 = nodeGraph->addNode("add", "add2");
    mx::NodePtr add3 = nodeGraph->addNode("add", "add3");
    mx::NodePtr noise3d = nodeGraph->addNode("noise3d", "noise3d");
    mx::NodePtr mix = nodeGraph->addNode("mix", "mix");
    mx::OutputPtr output = nodeGraph->addOutput("output");
    add1->setConnectedNode("in1", constant1);
    add1->setConnectedNode("in2", constant2);
    add2->setConnectedNode("in1", constant2);
    add2->setConnectedNode("in2", image2);
    add3->setConnectedNode("in1", add1);
    add3->setConnectedNode("in2", add2);
    multiply->setConnectedNode("in1", image1);
    multiply->setConnectedNode("in2", add1);
    mix->setConnectedNode("fg", multiply);
    mix->setConnectedNode("bg", add3);
    mix->setConnectedNode("mask", noise3d);
    output->setConnectedNode(mix);

    // Validate the document.
    REQUIRE(doc->validate());

    const std::string blessed =
        "digraph {\n" \
        "    \"image1\" [shape=box];\n" \
        "    \"image2\" [shape=box];\n" \
        "    \"multiply\" [shape=box];\n" \
        "    \"constant1\" [shape=box];\n" \
        "    \"constant2\" [shape=box];\n" \
        "    \"add1\" [shape=box];\n" \
        "    \"add2\" [shape=box];\n" \
        "    \"add3\" [shape=box];\n" \
        "    \"noise3d\" [shape=box];\n" \
        "    \"mix\" [shape=box];\n" \
        "    \"mix\" -> \"output\" [label=\"\"];\n" \
        "    \"multiply\" -> \"mix\" [label=\"fg\"];\n" \
        "    \"image1\" -> \"multiply\" [label=\"in1\"];\n" \
        "    \"add1\" -> \"multiply\" [label=\"in2\"];\n" \
        "    \"constant1\" -> \"add1\" [label=\"in1\"];\n" \
        "    \"constant2\" -> \"add1\" [label=\"in2\"];\n" \
        "    \"add3\" -> \"mix\" [label=\"bg\"];\n" \
        "    \"add1\" -> \"add3\" [label=\"in1\"];\n" \
        "    \"add2\" -> \"add3\" [label=\"in2\"];\n" \
        "    \"constant2\" -> \"add2\" [label=\"in1\"];\n" \
        "    \"image2\" -> \"add2\" [label=\"in2\"];\n" \
        "    \"noise3d\" -> \"mix\" [label=\"mask\"];\n" \
        "}\n";

    const std::string dot = mx::printGraphDot(nodeGraph);

    REQUIRE(dot == blessed);
}
