//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifdef MATERIALX_BUILD_RUNTIME

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Observer.h>

#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/File.h>

#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtPath.h>
#include <MaterialXRuntime/RtFileIo.h>
#include <MaterialXRuntime/RtTraversal.h>

#include <MaterialXGenShader/Util.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

namespace mx = MaterialX;

TEST_CASE("Runtime: Token", "[runtime]")
{
    mx::RtToken tok1 = "hej";
    mx::RtToken tok2 = "hey";
    mx::RtToken tok3 = "hej";
    REQUIRE(tok1 != tok2);
    REQUIRE(tok1 == tok3);
    REQUIRE(tok1 == "hej");
    REQUIRE(tok1 == std::string("hej"));
    REQUIRE("hej" == tok1);
    REQUIRE(std::string("hej") == tok1);

    mx::RtTokenMap<int> intMap;
    intMap["one"] = 1;
    intMap["two"] = 2;
    intMap["three"] = 3;
    REQUIRE(intMap.size() == 3);
    REQUIRE(intMap.count("one"));
    REQUIRE(intMap["one"] == 1);
    REQUIRE(intMap.count("two"));
    REQUIRE(intMap["two"] == 2);
    REQUIRE(intMap.count("three"));
    REQUIRE(intMap["three"] == 3);

    mx::RtTokenSet intSet;
    intSet.insert("one");
    intSet.insert("two");
    intSet.insert("three");
    REQUIRE(intSet.size() == 3);
    REQUIRE(intSet.count("one"));
    REQUIRE(intSet.count("two"));
    REQUIRE(intSet.count("three"));
    REQUIRE(!intSet.count("four"));
}

TEST_CASE("Runtime: Values", "[runtime]")
{
    mx::RtValue v1, v2, v3;
    v1.asInt() = 42;
    v2.asFloat() = 1.0f;
    v3.asFloat() = 2.0f;
    REQUIRE(v1.asInt() == 42);
    REQUIRE(v2.asFloat() + v3.asFloat() == 3.0f);

    mx::RtValue color3;
    color3.asColor3() = mx::Color3(1.0f, 0.0f, 0.0f);
    REQUIRE(color3.asColor3()[0] == 1.0f);
    REQUIRE(color3.asFloat() == 1.0f);

    mx::RtValue vector4;
    vector4.asVector4() = mx::Vector4(4.0f, 3.0f, 2.0f, 1.0f);
    REQUIRE(vector4.asVector4()[0] == 4.0f);
    REQUIRE(vector4.asVector4()[1] == 3.0f);
    REQUIRE(vector4.asVector4()[2] == 2.0f);
    REQUIRE(vector4.asVector4()[3] == 1.0f);

    mx::RtValue v4 = vector4;
    REQUIRE(v4.asVector4()[0] == 4.0f);

    mx::RtValue ptr;
    ptr.asPtr() = &vector4;
    REQUIRE(ptr.asPtr() == &vector4);
    ptr.clear();
    REQUIRE(ptr.asPtr() == (void*)0);

    // Test creating large values.
    // An stage is needed to take ownership of allocated data.
    mx::RtStage stage = mx::RtStage::createNew("stage1");
    mx::RtObject rootPrim = stage.getRootPrim();

    const std::string teststring("MaterialX");
    mx::RtValue str(teststring, rootPrim);
    REQUIRE(str.asString() == teststring);

    const mx::Matrix33 testmatrix(mx::Matrix33::IDENTITY);
    mx::RtValue mtx33(testmatrix, rootPrim);
    REQUIRE(mtx33.asMatrix33().isEquivalent(testmatrix, 1e-6f));
    mtx33.asMatrix33()[0][0] = 42.0f;
    REQUIRE(!mtx33.asMatrix33().isEquivalent(testmatrix, 1e-6f));

    // Test unmarshalling values from string representations.
    // For small values (<=16byts) the same value instance can be reused
    // For multiple value types.
    mx::RtValue value = mx::RtValue::createNew(mx::RtType::BOOLEAN, rootPrim);
    mx::RtValue::fromString(mx::RtType::BOOLEAN, "true", value);
    REQUIRE(value.asBool());
    mx::RtValue::fromString(mx::RtType::BOOLEAN, "false", value);
    REQUIRE(!value.asBool());
    mx::RtValue::fromString(mx::RtType::INTEGER, "23", value);
    REQUIRE(value.asInt() == 23);
    mx::RtValue::fromString(mx::RtType::FLOAT, "1234.5678", value);
    REQUIRE(fabs(value.asFloat() - 1234.5678f) < 1e-3f);
    mx::RtValue::fromString(mx::RtType::COLOR2, "1.0, 2.0", value);
    REQUIRE(value.asColor2() == mx::Color2(1.0, 2.0));
    mx::RtValue::fromString(mx::RtType::COLOR3, "1.0, 2.0, 3.0", value);
    REQUIRE(value.asColor3() == mx::Color3(1.0, 2.0, 3.0));
    mx::RtValue::fromString(mx::RtType::COLOR4, "1.0, 2.0, 3.0, 4.0", value);
    REQUIRE(value.asColor4() == mx::Color4(1.0, 2.0, 3.0, 4.0));
    mx::RtValue::fromString(mx::RtType::TOKEN, "materialx", value);
    REQUIRE(value.asToken() == mx::RtToken("materialx"));
    // For large values (>16bytes) we need to allocate a new value instance per type
    mx::RtValue matrix33Value = mx::RtValue::createNew(mx::RtType::MATRIX33, rootPrim);
    mx::RtValue::fromString(mx::RtType::MATRIX33, "1.0, 0.0, 0.0,  0.0, 1.0, 0.0,  0.0, 0.0, 1.0", matrix33Value);
    REQUIRE(matrix33Value.asMatrix33() == mx::Matrix33::IDENTITY);
    mx::RtValue matrix44Value = mx::RtValue::createNew(mx::RtType::MATRIX44, rootPrim);
    mx::RtValue::fromString(mx::RtType::MATRIX44, "1.0, 0.0, 0.0, 0.0,  0.0, 1.0, 0.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 0.0, 1.0", matrix44Value);
    REQUIRE(matrix44Value.asMatrix44() == mx::Matrix44::IDENTITY);
    mx::RtValue stringValue = mx::RtValue::createNew(mx::RtType::STRING, rootPrim);
    mx::RtValue::fromString(mx::RtType::STRING, "materialx", stringValue);
    REQUIRE(stringValue.asString() == "materialx");
    REQUIRE_THROWS(mx::RtValue::fromString(mx::RtType::INTEGER, "true", value));
}

TEST_CASE("Runtime: Types", "[runtime]")
{
    // Make sure the standard types are registered
    const mx::RtTypeDef* floatType = mx::RtTypeDef::findType("float");
    REQUIRE(floatType != nullptr);
    REQUIRE(floatType->getBaseType() == mx::RtTypeDef::BASETYPE_FLOAT);
    const mx::RtTypeDef* integerType = mx::RtTypeDef::findType("integer");
    REQUIRE(integerType != nullptr);
    REQUIRE(integerType->getBaseType() == mx::RtTypeDef::BASETYPE_INTEGER);
    const mx::RtTypeDef* booleanType = mx::RtTypeDef::findType("boolean");
    REQUIRE(booleanType != nullptr);
    REQUIRE(booleanType->getBaseType() == mx::RtTypeDef::BASETYPE_BOOLEAN);
    const mx::RtTypeDef* color2Type = mx::RtTypeDef::findType("color2");
    REQUIRE(color2Type != nullptr);
    REQUIRE(color2Type->getBaseType() == mx::RtTypeDef::BASETYPE_FLOAT);
    const mx::RtTypeDef* color3Type = mx::RtTypeDef::findType("color3");
    REQUIRE(color3Type != nullptr);
    REQUIRE(color3Type->getBaseType() == mx::RtTypeDef::BASETYPE_FLOAT);
    REQUIRE(color3Type->getSemantic() == mx::RtTypeDef::SEMANTIC_COLOR);
    const mx::RtTypeDef* color4Type = mx::RtTypeDef::findType("color4");
    REQUIRE(color4Type != nullptr);
    REQUIRE(color4Type->getBaseType() == mx::RtTypeDef::BASETYPE_FLOAT);
    REQUIRE(color4Type->getSemantic() == mx::RtTypeDef::SEMANTIC_COLOR);

    // Make sure we can register a new custom type
    auto createFoo = [](mx::RtObject&) -> mx::RtValue
    {
        return mx::RtValue(7);
    };
    auto copyFoo = [](const mx::RtValue& src, mx::RtValue& dest)
    {
        dest = src;
    };
    auto compareFoo = [](const mx::RtValue& a, const mx::RtValue& b) -> bool
    {
        return a == b;
    };
    auto toStringFoo = [](const mx::RtValue&, std::string& dest)
    {
        dest = "42";
    };
    auto fromStringFoo = [](const std::string&, mx::RtValue& dest)
    {
        dest = mx::RtValue(42);
    };
    mx::RtValueFuncs fooFuncs = { createFoo, copyFoo, compareFoo, toStringFoo, fromStringFoo };
    const mx::RtTypeDef* fooType = mx::RtTypeDef::registerType("foo", mx::RtTypeDef::BASETYPE_FLOAT, fooFuncs, mx::RtTypeDef::SEMANTIC_COLOR, 5);
    REQUIRE(fooType != nullptr);
    const mx::RtTypeDef* fooType2 = mx::RtTypeDef::findType("foo");
    REQUIRE(fooType2 == fooType);

    // Test create/parse/copy values
    // An stage is needed to hold allocated data.
    mx::RtStage stage = mx::RtStage::createNew("stage1");
    mx::RtObject rootPrim = stage.getRootPrim();

    mx::RtValue fooValue = fooType->createValue(rootPrim);
    REQUIRE(fooValue.asInt() == 7);
    fooType->fromStringValue("bar", fooValue);
    REQUIRE(fooValue.asInt() == 42);

    const mx::RtTypeDef* stringType = mx::RtTypeDef::findType("string");
    mx::RtValue stringValue1 = stringType->createValue(rootPrim);
    mx::RtValue stringValue2 = stringType->createValue(rootPrim);
    stringValue1.asString() = "foobar";
    stringType->copyValue(stringValue1, stringValue2);
    REQUIRE(stringValue2.asString() == "foobar");

    mx::RtValue intValue = integerType->createValue(rootPrim);
    integerType->fromStringValue("12345", intValue);
    REQUIRE(intValue.asInt() == 12345);

    // Make sure we can't use a name already take
    REQUIRE_THROWS(mx::RtTypeDef::registerType("color3", mx::RtTypeDef::BASETYPE_FLOAT, fooFuncs));

    // Make sure we can't request an unknown type
    REQUIRE(mx::RtTypeDef::findType("bar") == nullptr);

    // Make sure a type is connectable to its own type
    // TODO: Extend to test more types when type auto cast is implemented.
    REQUIRE(floatType->getValidConnectionTypes().count(floatType->getName()));
    REQUIRE(!floatType->getValidConnectionTypes().count(color4Type->getName()));

    // Test aggregate types component index/name
    REQUIRE(color4Type->getComponentIndex("r") == 0);
    REQUIRE(color4Type->getComponentIndex("g") == 1);
    REQUIRE(color4Type->getComponentIndex("b") == 2);
    REQUIRE(color4Type->getComponentIndex("a") == 3);
    REQUIRE_THROWS(color4Type->getComponentIndex("q"));
    REQUIRE(color4Type->getComponentName(0) == "r");
    REQUIRE(color4Type->getComponentName(1) == "g");
    REQUIRE(color4Type->getComponentName(2) == "b");
    REQUIRE(color4Type->getComponentName(3) == "a");
    REQUIRE_THROWS(color4Type->getComponentName(7));
}

TEST_CASE("Runtime: Paths", "[runtime]")
{
    mx::RtObject stageObj = mx::RtStage::createNew("root");
    mx::RtStage stage(stageObj);

    mx::RtPath empty;
    REQUIRE(!stage.getPrimAtPath(empty).isValid());

    mx::RtPath root("/");
    REQUIRE(stage.getPrimAtPath(root).isValid());
}

TEST_CASE("Runtime: Prims", "[runtime]")
{
    mx::RtObject stageObj = mx::RtStage::createNew("root");
    mx::RtStage stage(stageObj);

    // Test validity/bool operators
    REQUIRE(stageObj);
    REQUIRE(!stageObj == false);
    REQUIRE(stage);
    REQUIRE(!stage == false);

    // Test creating a prim of each type
    mx::RtObject nodedefObj = stage.createPrim("/ND_add_float", mx::RtNodeDef::typeName());
    REQUIRE(nodedefObj.isValid());
    REQUIRE(nodedefObj.getObjType() == mx::RtObjType::NODEDEF);
    REQUIRE(nodedefObj.getObjTypeName() == mx::RtNodeDef::typeName());
    REQUIRE(nodedefObj.hasApi(mx::RtApiType::NODEDEF));
    REQUIRE(nodedefObj.hasApi(mx::RtApiType::PRIM));
    mx::RtNodeDef nodeDef(nodedefObj);
    nodeDef.setNodeTypeName("add");
    REQUIRE(nodeDef.getNodeTypeName() == "add");

    mx::RtObject nodeObj = stage.createPrim("/add1", mx::RtNode::typeName(), nodedefObj);
    REQUIRE(nodeObj.isValid());
    REQUIRE(nodeObj.getObjType() == mx::RtObjType::NODE);
    REQUIRE(nodeObj.getObjTypeName() == mx::RtNode::typeName());
    REQUIRE(nodeObj.hasApi(mx::RtApiType::NODE));
    REQUIRE(nodeObj.hasApi(mx::RtApiType::PRIM));
    mx::RtNode node(nodeObj);
    REQUIRE(node.getPrimTypeName() == nodeDef.getNodeTypeName());

    mx::RtObject graphObj = stage.createPrim("/graph1", mx::RtNodeGraph::typeName());
    REQUIRE(graphObj.isValid());
    REQUIRE(graphObj.getObjType() == mx::RtObjType::NODEGRAPH);
    REQUIRE(graphObj.getObjTypeName() == mx::RtNodeGraph::typeName());
    REQUIRE(graphObj.hasApi(mx::RtApiType::NODEGRAPH));
    REQUIRE(graphObj.hasApi(mx::RtApiType::PRIM));

    mx::RtObject fooObj = stage.createPrim("/graph1", "foo");
    REQUIRE(fooObj.isValid());
    REQUIRE(fooObj.getObjType() == mx::RtObjType::PRIM);
    REQUIRE(fooObj.getObjTypeName() == mx::RtPrim::typeName());
    REQUIRE(fooObj.hasApi(mx::RtApiType::PRIM));
}

TEST_CASE("Runtime: Nodes", "[runtime]")
{
    mx::RtStage stage = mx::RtStage::createNew("root");

    // Create a new nodedef object for defining an add node
    mx::RtNodeDef nodedef = stage.createPrim("/ND_add_float", mx::RtNodeDef::typeName());
    nodedef.setNodeTypeName("add");

    // Test adding metadata
    mx::RtTypedValue* version = nodedef.addMetadata("version", "float");
    version->getValue().asFloat() = 1.0f;
    REQUIRE(version->getValue().asFloat() == 1.0);

    // Add attributes to the nodedef
    nodedef.createAttribute("in1", "float");
    nodedef.createAttribute("in2", "float");
    nodedef.createAttribute("out", "float", mx::RtAttrFlag::OUTPUT);

    // Test the new attributes
    mx::RtAttribute out = nodedef.getAttribute("out");
    REQUIRE(out.isValid());
    REQUIRE(out.isOutput());
    REQUIRE(!out.isConnectable());
    REQUIRE(out.getType() == "float");
    REQUIRE(out.getValue().asFloat() == 0.0f);
    mx::RtAttribute foo = nodedef.getAttribute("foo");
    REQUIRE(!foo.isValid());
    mx::RtAttribute in1 = nodedef.getAttribute("in1");
    REQUIRE(in1.isValid());
    REQUIRE(in1.isInput());
    REQUIRE(!in1.isConnectable());
    in1.getValue().asFloat() = 7.0f;
    REQUIRE(in1.getValue().asFloat() == 7.0f);

    // Test deleting an attribute
    nodedef.createAttribute("in3", "float");
    mx::RtAttribute in3 = nodedef.getAttribute("in3");
    REQUIRE(in3);
    nodedef.removeAttribute(in3.getName());
    in3 = nodedef.getAttribute("in3");
    REQUIRE(!in3);

    // Node creation without a nodedef should throw.
    REQUIRE_THROWS(stage.createPrim("/add1", mx::RtNode::typeName()));

    // Create two new node instances from the add nodedef
    mx::RtObject add1Obj = stage.createPrim(mx::RtNode::typeName(), nodedef.getObject());
    mx::RtObject add2Obj = stage.createPrim(mx::RtNode::typeName(), nodedef.getObject());
    REQUIRE(add1Obj.isValid());
    REQUIRE(add2Obj.isValid());

    // Attach the node API to these objects
    mx::RtNode add1(add1Obj);
    mx::RtNode add2(add2Obj);
    REQUIRE(add1.getName() == "add1");
    REQUIRE(add2.getName() == "add2");

/*
    // Get the node instance ports
    mx::RtPort add1_in1 = add1.findPort("in1");
    mx::RtPort add1_in2 = add1.findPort("in2");
    mx::RtPort add1_out = add1.findPort("out");
    mx::RtPort add2_in1 = add2.findPort("in1");
    mx::RtPort add2_in2 = add2.findPort("in2");
    mx::RtPort add2_out = add2.findPort("out");
    REQUIRE(add1_in1.isValid());
    REQUIRE(add1_in2.isValid());
    REQUIRE(add1_out.isValid());
    REQUIRE(add2_in1.isValid());
    REQUIRE(add2_in2.isValid());
    REQUIRE(add2_out.isValid());

    // Test setting port attributes
    const mx::RtToken meter("meter");
    const mx::RtToken srgb("srgb");
    add1_in1.setUnit(meter);
    add1_in2.setColorSpace(srgb);
    REQUIRE(add1_in1.getUnit() == meter);
    REQUIRE(add1_in1.getColorSpace() == mx::EMPTY_TOKEN);
    REQUIRE(add1_in2.getUnit() == mx::EMPTY_TOKEN);
    REQUIRE(add1_in2.getColorSpace() == srgb);
    mx::RtAttribute* fooAttr = add1_in1.addAttribute("foo", mx::RtType::FLOAT);
    fooAttr->getValue().asFloat() = 7.0f;
    REQUIRE(fooAttr == add1_in1.getAttribute("foo"));
    add1_in1.removeAttribute("foo");
    REQUIRE(nullptr == add1_in1.getAttribute("foo"));

    // Test port connectability
    REQUIRE(add1_out.canConnectTo(add2_in1));
    REQUIRE(add2_in1.canConnectTo(add1_out));
    REQUIRE(!add1_out.canConnectTo(add1_in1));
    REQUIRE(!add1_out.canConnectTo(add2_out));
    REQUIRE(!add2_in1.canConnectTo(add1_in1));

    // Make port connections
    mx::RtNode::connect(add1_out, add2_in1);
    REQUIRE(add1_out.isConnected());
    REQUIRE(add2_in1.isConnected());

    // Try connecting already connected ports
    REQUIRE_THROWS(mx::RtNode::connect(add1_out, add2_in1));

    // Break port connections
    mx::RtNode::disconnect(add1_out, add2_in1);
    REQUIRE(!add1_out.isConnected());
    REQUIRE(!add2_in1.isConnected());

    // Make more port connections, now testing
    // the port connectTo method
    add1_out.connectTo(add2_in1);
    add1_out.connectTo(add2_in2);
    size_t numDest = add1_out.numDestinationPorts();
    REQUIRE(numDest == 2);
    REQUIRE(add1_out.getDestinationPort(0) == add2_in1);
    REQUIRE(add1_out.getDestinationPort(1) == add2_in2);
    REQUIRE(add2_in1.getSourcePort() == add1_out);
    REQUIRE(add2_in2.getSourcePort() == add1_out);

    // Test node creation when name is not unique
    mx::RtNode add3 = mx::RtNode::createNew(stageObj, addDefObj, "add1");
    REQUIRE(add3.getName() == "add3");

    // Find object by path
    mx::RtObject elem1 = stage.findElementByPath("/add1/in2");
    REQUIRE(elem1.isValid());
    REQUIRE(elem1.hasApi(mx::RtApiType::PORTDEF));
    REQUIRE(mx::RtPortDef(elem1).getName() == "in2");
    REQUIRE(mx::RtPortDef(elem1).isInput());

    // Test RtPath
    mx::RtPath path1(add1Obj);
    REQUIRE(path1.isValid());
    REQUIRE(path1.asString() == "/add1");
    REQUIRE(path1.hasApi(mx::RtApiType::NODE));
    REQUIRE(path1.getObjType() == mx::RtObjType::NODE);
    mx::RtPath path2(addDefObj);
    REQUIRE(path2.isValid());
    REQUIRE(path2.asString() == "/ND_add_float");
    REQUIRE(path2.hasApi(mx::RtApiType::NODEDEF));
    path2.push("in1");
    REQUIRE(path2.isValid());
    REQUIRE(path2.asString() == "/ND_add_float/in1");
    REQUIRE(path2.hasApi(mx::RtApiType::PORTDEF));
    path2.pop();
    REQUIRE(path2.isValid());
    REQUIRE(path2.asString() == "/ND_add_float");
    path2.pop();
    REQUIRE(path2.isValid());
    REQUIRE(path2.isRoot());
    REQUIRE(path2.getObject() == stageObj);
    REQUIRE(path2.asString() == "/");

    mx::RtPath stagePath(stageObj);
    REQUIRE(stagePath.isValid());
    REQUIRE(stagePath.isRoot());
    REQUIRE(stagePath.getObject() == stageObj);
    REQUIRE(stagePath.asString() == "/");
    REQUIRE(stagePath == path2);

    mx::RtPath pathA, pathB;
    REQUIRE(!pathA.isValid());
    pathA.setObject(add1Obj);
    REQUIRE(pathA.isValid());
    pathB.setObject(add1Obj);
    REQUIRE(pathA == pathB);
    pathB.setObject(add2Obj);
    REQUIRE(pathA != pathB);
    pathB = pathA;
    REQUIRE(pathA == pathB);
    mx::RtPath pathC = pathB;
    REQUIRE(pathC == pathB);
 */
}

/*
TEST_CASE("Runtime: NodeGraphs", "[runtime]")
{
    mx::RtObject stageObj = mx::RtStage::createNew("root");
    mx::RtStage stage(stageObj);

    // Create a new nodedef for an add node.
    mx::RtObject addFloatObj = mx::RtNodeDef::createNew(stageObj, "ND_add_float", "add");
    mx::RtPortDef::createNew(addFloatObj, "in1", "float");
    mx::RtPortDef::createNew(addFloatObj, "in2", "float");
    mx::RtPortDef::createNew(addFloatObj, "out", "float", mx::RtPortFlag::OUTPUT);

    // Create a nodegraph object.
    mx::RtObject graphObj1 = mx::RtNodeGraph::createNew(stageObj, "graph1");
    REQUIRE(graphObj1.isValid());

    // Attach the nodegraph API to the object.
    mx::RtNodeGraph graph1(graphObj1);

    // Create add nodes in the graph.
    mx::RtObject addObj1 = mx::RtNode::createNew(graphObj1, addFloatObj, "add1");
    mx::RtObject addObj2 = mx::RtNode::createNew(graphObj1, addFloatObj, "add2");
    REQUIRE(graph1.numNodes() == 2);

    // Test deleting a node.
    mx::RtObject addObj3 = mx::RtNode::createNew(graphObj1, addFloatObj, "add3");
    REQUIRE(graph1.numNodes() == 3);
    graph1.removeNode(addObj3);
    REQUIRE(graph1.numNodes() == 2);

    // Test deleting a node by path.
    mx::RtObject addObj4 = mx::RtNode::createNew(graphObj1, addFloatObj, "add4");
    mx::RtPath addObj4Path(addObj4);
    const std::string pathStr = addObj4Path.asString();
    REQUIRE(stage.findElementByPath(pathStr));
    stage.removeElementByPath(addObj4Path);
    REQUIRE(!stage.findElementByPath(pathStr));

    // Add interface port definitions to the graph.
    graph1.addPort("a", "float");
    graph1.addPort("b", "float");
    graph1.addPort("out", "float", mx::RtPortFlag::OUTPUT);
    REQUIRE(graph1.numPorts() == 3);
    REQUIRE(graph1.findInputSocket("a").isValid());
    REQUIRE(graph1.findInputSocket("b").isValid());
    REQUIRE(graph1.findOutputSocket("out").isValid());
    REQUIRE(graph1.getInputSocket(0).isOutput());
    REQUIRE(graph1.getInputSocket(1).isOutput());
    REQUIRE(graph1.getOutputSocket(0).isInput());

    // Test deleting a port.
    graph1.addPort("c", "float");
    REQUIRE(graph1.numPorts() == 4);
    REQUIRE(graph1.findInputSocket("c").isValid());
    mx::RtObject cPort = mx::RtNodeDef(graph1.getNodeDef()).findPort("c");
    REQUIRE(cPort);
    graph1.removePort(cPort);
    REQUIRE(graph1.numPorts() == 3);
    REQUIRE(!graph1.findInputSocket("c").isValid());

    // Attach the node API for the two node objects.
    mx::RtNode add1(addObj1);
    mx::RtNode add2(addObj2);

    // Connect the graph nodes to each other and the interface.
    mx::RtNode::connect(graph1.findInputSocket("a"), add1.findPort("in1"));
    mx::RtNode::connect(graph1.findInputSocket("b"), add1.findPort("in2"));
    mx::RtNode::connect(add1.findPort("out"), add2.findPort("in1"));
    mx::RtNode::connect(graph1.findInputSocket("a"), add2.findPort("in2"));
    mx::RtNode::connect(add2.findPort("out"), graph1.findOutputSocket("out"));
    REQUIRE(graph1.findInputSocket("a").numDestinationPorts() == 2);
    REQUIRE(graph1.findInputSocket("b").numDestinationPorts() == 1);
    REQUIRE(graph1.findOutputSocket("out").getSourcePort() == add2.findPort("out"));

    // Test query of parent and root.
    REQUIRE(add1.getParent() == graphObj1);
    REQUIRE(add1.getRoot() == stageObj);
    REQUIRE(graph1.getParent() == stageObj);
    REQUIRE(graph1.getRoot() == stageObj);
    REQUIRE(stage.getRoot() == stageObj);
    REQUIRE(!stage.getParent().isValid());

    // Test finding a port by path.
    mx::RtObject node = stage.findElementByPath("/graph1/add2");
    mx::RtObject portdef = stage.findElementByPath("/graph1/add2/out");
    REQUIRE(node.isValid());
    REQUIRE(portdef.isValid());
    REQUIRE(node.hasApi(mx::RtApiType::NODE));
    REQUIRE(portdef.hasApi(mx::RtApiType::PORTDEF));

    // Test RtPath
    mx::RtPath path1(addObj1);
    REQUIRE(path1.isValid());
    REQUIRE(path1.asString() == "/graph1/add1");
    REQUIRE(path1.getObject() == addObj1);
    path1.pop();
    REQUIRE(path1.isValid());
    REQUIRE(path1.asString() == "/graph1");
    REQUIRE(path1.getObject() == graphObj1);
    path1.push(add2.getName());
    REQUIRE(path1.isValid());
    REQUIRE(path1.asString() == "/graph1/add2");
    REQUIRE(path1.getObject() == addObj2);

    // Test getting a port instance from node and portdef.
    mx::RtPort port1(node, portdef);
    REQUIRE(port1 == add2.findPort("out"));
}

TEST_CASE("Runtime: FileIo", "[runtime]")
{
    mx::FileSearchPath searchPath;
    searchPath.append(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    {
        // Load in stdlib
        // Create a stage and import the document data.
        mx::RtObject stageObj = mx::RtStage::createNew("stdlib");
        mx::RtFileIo stageIo(stageObj);
        stageIo.readLibraries({ "stdlib" }, searchPath);

        // Get a nodegraph and write a dot file for inspection.
        mx::RtStage stage(stageObj);
        mx::RtObject graphDef = stage.findElementByName("ND_tiledimage_float");
        REQUIRE(graphDef);
        mx::RtNodeGraph graph = stage.findElementByName("NG_tiledimage_float");
        REQUIRE(graph);
        std::ofstream dotfile;
        dotfile.open(graph.getName().str() + ".dot");
        dotfile << graph.asStringDot();
        dotfile.close();
        mx::RtObject image1 = mx::RtNode::createNew(stageObj, graphDef);
        REQUIRE(image1.hasApi(mx::RtApiType::NODE));

        // Get a nodedef and create two new instances of it.
        mx::RtObject multiplyColor3Obj = stage.findElementByName("ND_multiply_color3");
        REQUIRE(multiplyColor3Obj);
        mx::RtNode mult1 = mx::RtNode::createNew(stageObj, multiplyColor3Obj);
        mx::RtNode mult2 = mx::RtNode::createNew(stageObj, multiplyColor3Obj);
        REQUIRE(mult1);
        REQUIRE(mult2);
        mult2.findPort("in1").getValue().asColor3() = mx::Color3(0.3f, 0.5f, 0.4f);
        mult2.findPort("in2").getValue().asColor3() = mx::Color3(0.6f, 0.3f, 0.5f);
        mult2.findPort("in2").setColorSpace("srgb_texture");

        mx::RtNodeGraph stageGraph = mx::RtNodeGraph::createNew(stageObj);
        REQUIRE(stageGraph);
        REQUIRE(stageGraph.getName() == "nodegraph1");

        // Move the image node into the graph.
        stageGraph.addNode(image1);

        // Save the stage to file for inspection.
        stageIo.write(stage.getName().str() + "_export.mtlx", nullptr);

        // Write out the node using a given node definition
        mx::RtWriteOptions writeOptions;
        writeOptions.writeFilter = [graphDef](const mx::RtObject& obj) -> bool
        {  
            if (obj.getObjType() == mx::RtObjType::NODE)
            {
                mx::RtNode node(obj);
                mx::RtNodeDef nodedef = node.getNodeDef();
                return  (nodedef.getName().str() == "ND_tiledimage_float");
            }
            return false;
        };
        writeOptions.writeIncludes = false;
        stageIo.write(stage.getName().str() + "_nodedef_export.mtlx", &writeOptions);

        // Write out a nodegraph only
        writeOptions.writeFilter = [](const mx::RtObject& obj) -> bool
        {
            return (obj.hasApi(mx::RtApiType::NODEGRAPH));
        };
        stageIo.write(stage.getName().str() + "_nodegraph_export.mtlx", &writeOptions);
    }

    {
        // Load stdlib into a stage
        mx::RtStage stdlibStage = mx::RtStage::createNew("stdlib");
        mx::RtFileIo(stdlibStage.getObject()).readLibraries({ "stdlib" }, searchPath);

        // Create a new working space stage.
        mx::RtStage stage = mx::RtStage::createNew("main_with_xincludes");

        // Add reference to stdlib
        stage.addReference(stdlibStage.getObject());

        // Create a small node network.
        mx::RtObject tiledimageDef = stage.findElementByName("ND_tiledimage_color3");
        mx::RtObject texcoordDef = stage.findElementByName("ND_texcoord_vector2");
        REQUIRE(tiledimageDef);
        REQUIRE(texcoordDef);
        mx::RtNode tiledimage1 = mx::RtNode::createNew(stage.getObject(), tiledimageDef);
        mx::RtNode texcoord1 = mx::RtNode::createNew(stage.getObject(), texcoordDef);
        REQUIRE(tiledimage1);
        REQUIRE(texcoord1);
        mx::RtPort tiledimage1_texcoord = tiledimage1.findPort("texcoord");
        mx::RtPort tiledimage1_file = tiledimage1.findPort("file");
        mx::RtPort texcoord1_index = texcoord1.findPort("index");
        mx::RtPort texcoord1_out = texcoord1.findPort("out");
        REQUIRE(tiledimage1_texcoord);
        REQUIRE(tiledimage1_file);
        REQUIRE(texcoord1_index);
        REQUIRE(texcoord1_out);
        tiledimage1_file.getValue().asString() = "myimagetexture.png";
        texcoord1_out.connectTo(tiledimage1_texcoord);
        texcoord1_index.getValue().asInt() = 2;

        // Test write and read-back with explicit xincludes
        mx::RtWriteOptions writeOptions;
        writeOptions.writeIncludes = true;

        mx::RtFileIo fileIO(stage.getObject());
        fileIO.write(stage.getName().str() + "_export.mtlx", &writeOptions);

        // Test read and write to stream. Note that the steream includes
        // xincludes with content that overlaps whats in the stdlib stage,
        // so the "skip duplicates" flag must be set.
        std::stringstream stream1;
        fileIO.write(stream1, &writeOptions);
        REQUIRE(!stream1.str().empty());

        mx::RtStage streamStage = mx::RtStage::createNew("stream");
        mx::RtFileIo streamFileIO(streamStage.getObject());
        streamStage.addReference(stdlibStage.getObject());
        mx::RtReadOptions readOptions;
        readOptions.skipConflictingElements = true;
        streamFileIO.read(stream1, &readOptions);
        streamFileIO.write(streamStage.getName().str() + "_export.mtlx", &writeOptions);
    }
}

TEST_CASE("Runtime: FileIo NodeGraph", "[runtime]")
{
    mx::FileSearchPath searchPath;
    searchPath.append(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
 
    // Load in stdlib to a stage.
    mx::RtStage libStage = mx::RtStage::createNew("libs");
    mx::RtFileIo libFileIo(libStage.getObject());
    libFileIo.readLibraries({ "stdlib" }, searchPath);

    // Create a main stage referencing the libs stage.
    mx::RtStage mainStage = mx::RtStage::createNew("main");
    mainStage.addReference(libStage.getObject());

    // Create a nodegraph.
    mx::RtNodeGraph graph = mx::RtNodeGraph::createNew(mainStage.getObject(), "testgraph");
    graph.addPort("in", mx::RtType::FLOAT);
    graph.addPort("out", mx::RtType::FLOAT, mx::RtPortFlag::OUTPUT);
    mx::RtPort graphIn = graph.findPort("in");
    mx::RtPort graphOut = graph.findPort("out");
    mx::RtPort graphInSocket = graph.findInputSocket("in");
    mx::RtPort graphOutSocket = graph.findOutputSocket("out");
    REQUIRE(graphIn);
    REQUIRE(graphOut);
    REQUIRE(graphInSocket);
    REQUIRE(graphOutSocket);

    // Add nodes to the graph.
    mx::RtNodeDef addNodeDef = mainStage.findElementByName("ND_add_float");
    mx::RtNode add1 = mx::RtNode::createNew(graph.getObject(), addNodeDef.getObject());
    mx::RtNode add2 = mx::RtNode::createNew(graph.getObject(), addNodeDef.getObject());
    mx::RtNode::connect(graphInSocket, add1.findPort("in1"));
    mx::RtNode::connect(add1.findPort("out"), add2.findPort("in1"));
    mx::RtNode::connect(add2.findPort("out"), graphOutSocket);
    // Add an unconnected node.
    mx::RtNode::createNew(graph.getObject(), addNodeDef.getObject());

    // Create a node on root level and connect it downstream after the graph.
    mx::RtNode add4 = mx::RtNode::createNew(mainStage.getObject(), addNodeDef.getObject());
    mx::RtNode::connect(graphOut, add4.findPort("in1"));

    mx::RtWriteOptions options;
    options.writeIncludes = false;

    // Save the stage to file for inspection.
    const mx::FilePath filename = graph.getName().str() + "_export.mtlx";
    mx::RtFileIo fileIo(mainStage.getObject());
    fileIo.write(filename, &options);

    // Read the saved file to another stage.
    mx::RtStage anotherStage = mx::RtStage::createNew("another");
    anotherStage.addReference(libStage.getObject());
    fileIo.setObject(anotherStage.getObject());
    fileIo.read(filename, searchPath);

    // Save the re-read stage to file for inspection.
    const mx::FilePath filename2 = graph.getName().str() + "_another_export.mtlx";
    fileIo.write(filename2, &options);

    std::ifstream file1(filename.asString());
    std::string str1((std::istreambuf_iterator<char>(file1)), std::istreambuf_iterator<char>());
    std::ifstream file2(filename2.asString());
    std::string str2((std::istreambuf_iterator<char>(file2)), std::istreambuf_iterator<char>());
    REQUIRE(str1 == str2);
}

TEST_CASE("Runtime: Rename", "[runtime]")
{
    // Load in stdlib as a referenced stage ("libs").
    mx::RtObject libStageObj = mx::RtStage::createNew("libs");
    mx::RtStage libStage(libStageObj);
    mx::FileSearchPath searchPath;
    searchPath.append(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    mx::RtFileIo libFileIo(libStageObj);
    libFileIo.readLibraries({ "stdlib" }, searchPath);

    // Create a main stage.
    mx::RtObject mainStageObj = mx::RtStage::createNew("main");
    mx::RtStage mainStage(mainStageObj);
    mainStage.addReference(libStageObj);

    // Create some nodes.
    mx::RtObject nodedefObj = mainStage.findElementByName("ND_add_float");
    mx::RtNodeDef nodedef(nodedefObj);
    REQUIRE(nodedef.isValid());
    mx::RtNode node1 = mx::RtNode::createNew(mainStage.getObject(), nodedefObj);
    mx::RtNode node2 = mx::RtNode::createNew(mainStage.getObject(), nodedefObj);
    REQUIRE(node1.isValid());
    REQUIRE(node2.isValid());

    REQUIRE(node1.getName() == "add1");
    REQUIRE(node2.getName() == "add2");

    // Test node renaming
    node1.setName("foo");
    REQUIRE(node1.getName() == "foo");
    node2.setName("foo");
    REQUIRE(node2.getName() == "foo1");
    node1.setName("add1");
    REQUIRE(node1.getName() == "add1");
    node2.setName("foo");
    REQUIRE(node2.getName() == "foo");
    node2.setName("add2");
    REQUIRE(node2.getName() == "add2");

    // Test that a rename to existing "add1" results in the
    // old name "add2" since that is still a unique name.
    node2.setName("add1");
    REQUIRE(node2.getName() == "add2");
}

TEST_CASE("Runtime: Stage References", "[runtime]")
{
    // Load in stdlib as a referenced stage ("libs").
    mx::RtObject libStageObj = mx::RtStage::createNew("libs");
    mx::RtStage libStage(libStageObj);
    mx::FileSearchPath searchPath;
    searchPath.append(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    mx::RtFileIo libFileIo(libStageObj);
    libFileIo.readLibraries({ "stdlib", "pbrlib" }, searchPath);

    // Create a main stage.
    mx::RtObject mainStageObj = mx::RtStage::createNew("main");
    mx::RtStage mainStage(mainStageObj);
    mainStage.addReference(libStageObj);

    // Test access and usage of contents from the referenced library.
    mx::RtObject nodedefObj = mainStage.findElementByName("ND_complex_ior");
    mx::RtNodeDef nodedef(nodedefObj);
    REQUIRE(nodedef.isValid());
    mx::RtNode node1 = mx::RtNode::createNew(mainStage.getObject(), nodedefObj);
    REQUIRE(node1.isValid());

    // Write the main stage to a new document,
    // writing only the non-referenced content.
    mx::RtWriteOptions options;
    options.writeIncludes = true;
    mx::RtFileIo mainFileIO(mainStageObj);
    mainFileIO.write(mainStage.getName().str() + ".mtlx", &options);

    // Make sure removal of the non-referenced node works
    const mx::RtToken nodeName = node1.getName();
    mainStage.removeElement(node1.getObject());
    node1 = mainStage.findElementByName(nodeName);
    REQUIRE(!node1.isValid());

    // Test RtPath on referenced contents
    mx::RtPath path1(nodedefObj);
    REQUIRE(path1.isValid());
    REQUIRE(path1.asString() == "/ND_complex_ior");
    REQUIRE(path1.getObject() == nodedefObj);
    path1.push("reflectivity");
    REQUIRE(path1.isValid());
    REQUIRE(path1.asString() == "/ND_complex_ior/reflectivity");
    REQUIRE(path1.getObject() == nodedef.findPort("reflectivity"));
}

TEST_CASE("Runtime: Traversal", "[runtime]")
{
    mx::FileSearchPath searchPath;
    searchPath.append(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));

    // Load standard libraries in a stage.
    mx::RtObject libStageObj = mx::RtStage::createNew("libs");
    mx::RtFileIo libStageIO(libStageObj);
    libStageIO.readLibraries({ "stdlib", "pbrlib" }, searchPath);

    // Count elements traversing the full stdlib stage
    mx::RtStage libStage(libStageObj);
    size_t nodeCount = 0, nodeDefCount = 0, nodeGraphCount = 0;
    for (auto it = libStage.traverseStage(); !it.isDone(); ++it)
    {
        switch ((*it).getObjType())
        {
        case mx::RtObjType::NODE:
            nodeCount++;
            break;
        case mx::RtObjType::NODEDEF:
            nodeDefCount++;
            break;
        case mx::RtObjType::NODEGRAPH:
            nodeGraphCount++;
            break;
        default:
            break;
        }
    }

    // Loading the same libraries into a MaterialX document
    // and tests the same element counts.
    mx::DocumentPtr doc = mx::createDocument();
    loadLibraries({ "stdlib", "pbrlib" }, searchPath, doc);
    REQUIRE(nodeCount == doc->getNodes().size());
    REQUIRE(nodeDefCount == doc->getNodeDefs().size());
    REQUIRE(nodeGraphCount == doc->getNodeGraphs().size());

    // Create a main stage.
    mx::RtStage mainStage = mx::RtStage::createNew("main");
    mainStage.addReference(libStageObj);

    mx::RtNodeDef nodedef = mainStage.findElementByName("ND_subtract_vector3");
    REQUIRE(nodedef);
    mx::RtObject nodeObj = mx::RtNode::createNew(mainStage.getObject(), nodedef.getObject());
    REQUIRE(nodeObj);

    // Travers using a filter to return only node objects.
    mx::RtObjectFilter<mx::RtObjType::NODE> nodeFilter;
    nodeCount = 0, nodeDefCount = 0, nodeGraphCount = 0;
    for (auto it = mainStage.traverseStage(nodeFilter); !it.isDone(); ++it)
    {
        switch ((*it).getObjType())
        {
        case mx::RtObjType::NODE:
            nodeCount++;
            break;
        case mx::RtObjType::NODEDEF:
            nodeDefCount++;
            break;
        case mx::RtObjType::NODEGRAPH:
            nodeGraphCount++;
            break;
        default:
            break;
        }
    }
    REQUIRE(nodeCount == 1);
    REQUIRE(nodeDefCount == 0);
    REQUIRE(nodeGraphCount == 0);

    // Travers using a filter to return only objects supporting the nodegraph API.
    mx::RtApiFilter<mx::RtApiType::NODEGRAPH> apiFilter;
    nodeCount = 0, nodeDefCount = 0, nodeGraphCount = 0;
    for (auto it = mainStage.traverseStage(apiFilter); !it.isDone(); ++it)
    {
        switch ((*it).getObjType())
        {
        case mx::RtObjType::NODE:
            nodeCount++;
            break;
        case mx::RtObjType::NODEDEF:
            nodeDefCount++;
            break;
        case mx::RtObjType::NODEGRAPH:
            nodeGraphCount++;
            break;
        default:
            break;
        }
    }
    REQUIRE(nodeCount == 0);
    REQUIRE(nodeDefCount == 0);
    REQUIRE(nodeGraphCount == 65);

    // Traverse a nodegraph using tree traversal.
    mx::RtNodeGraph nodegraph = mainStage.findElementByName("NG_tiledimage_float");
    REQUIRE(nodegraph);
    nodeCount = 0;
    for (auto it = nodegraph.traverseTree(); !it.isDone(); ++it)
    {
        if ((*it).getObjType() == mx::RtObjType::NODE)
        {
            nodeCount++;
        }
    }
    REQUIRE(nodeCount == 5);

    // Filter for finding input ports.
    auto inputFilter = [](const mx::RtObject& obj) -> bool
    {
        if (obj.hasApi(mx::RtApiType::PORTDEF))
        {
            mx::RtPortDef portdef(obj);
            return portdef.isInput();
        }
        return false;
    };

    // Travers a nodedef finding all its inputs.
    mx::RtNodeDef generalized_schlick_brdf = mainStage.findElementByName("ND_generalized_schlick_brdf");
    REQUIRE(generalized_schlick_brdf);
    size_t inputCount = 0;
    for (auto it = generalized_schlick_brdf.traverseTree(inputFilter); !it.isDone(); ++it)
    {
        inputCount++;
    }
    REQUIRE(inputCount == 9);

    // Filter for finding nodedefs of BSDF nodes.
    auto bsdfFilter = [](const mx::RtObject& obj) -> bool
    {
        if (obj.hasApi(mx::RtApiType::NODEDEF))
        {
            mx::RtNodeDef nodedef(obj);
            return nodedef.numOutputs() == 1 && mx::RtPortDef(nodedef.getPort(0)).getType() == mx::RtType::BSDF;
        }
        return false;
    };

    // Travers to find all nodedefs for BSDF nodes.
    size_t bsdfCount = 0;
    for (auto it = mainStage.traverseStage(bsdfFilter); !it.isDone(); ++it)
    {
        bsdfCount++;
    }
    REQUIRE(bsdfCount == 14);

    // Find the output port on the nodegraph above,
    // and test traversing the graph upstream from 
    // this output.
    mx::RtPort outSocket = nodegraph.getOutputSocket(0);
    REQUIRE(outSocket.isInput());
    REQUIRE(outSocket.isConnected());
    REQUIRE(outSocket.getSourcePort());
    size_t numEdges = 0;
    for (auto it = outSocket.traverseUpstream(); !it.isDone(); ++it)
    {
        ++numEdges;
    }
    REQUIRE(numEdges == 16);
}
*/

#endif // MATERIALX_BUILD_RUNTIME
