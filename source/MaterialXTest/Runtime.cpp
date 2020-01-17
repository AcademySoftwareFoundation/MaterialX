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

namespace
{
    // Commonly used tokens.
    const mx::RtToken X("x");
    const mx::RtToken Y("y");
    const mx::RtToken Z("z");
    const mx::RtToken W("w");
    const mx::RtToken R("r");
    const mx::RtToken G("g");
    const mx::RtToken B("b");
    const mx::RtToken A("a");
    const mx::RtToken ADD("add");
    const mx::RtToken IN1("in1");
    const mx::RtToken IN2("in2");
    const mx::RtToken IN3("in3");
    const mx::RtToken OUT("out");
    const mx::RtToken FOO("foo");
    const mx::RtToken BAR("bar");
    const mx::RtToken VERSION("version");
    const mx::RtToken ROOT("root");
    const mx::RtToken MAIN("main");
    const mx::RtToken LIB("lib");
    const mx::RtToken NONAME("");
}

TEST_CASE("Runtime: Token", "[runtime]")
{
    mx::RtToken tok1("hej");
    mx::RtToken tok2("hey");
    mx::RtToken tok3("hej");
    REQUIRE(tok1 != tok2);
    REQUIRE(tok1 == tok3);
    REQUIRE(tok1 == "hej");
    REQUIRE(tok1 == std::string("hej"));
    REQUIRE("hej" == tok1);
    REQUIRE(std::string("hej") == tok1);

    mx::RtToken one("one");
    mx::RtToken two("two");
    mx::RtToken three("three");
    mx::RtTokenMap<int> intMap;
    intMap[one] = 1;
    intMap[two] = 2;
    intMap[three] = 3;
    REQUIRE(intMap.size() == 3);
    REQUIRE(intMap.count(one));
    REQUIRE(intMap[one] == 1);
    REQUIRE(intMap.count(two));
    REQUIRE(intMap[two] == 2);
    REQUIRE(intMap.count(three));
    REQUIRE(intMap[three] == 3);

    mx::RtTokenSet intSet;
    intSet.insert(one);
    intSet.insert(two);
    intSet.insert(three);
    REQUIRE(intSet.size() == 3);
    REQUIRE(intSet.count(one));
    REQUIRE(intSet.count(two));
    REQUIRE(intSet.count(three));
    REQUIRE(!intSet.count(tok1));
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
    mx::RtStage stage = mx::RtStage::createNew(MAIN);
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
    const mx::RtTypeDef* floatType = mx::RtTypeDef::findType(mx::RtType::FLOAT);
    REQUIRE(floatType != nullptr);
    REQUIRE(floatType->getBaseType() == mx::RtTypeDef::BASETYPE_FLOAT);
    const mx::RtTypeDef* integerType = mx::RtTypeDef::findType(mx::RtType::INTEGER);
    REQUIRE(integerType != nullptr);
    REQUIRE(integerType->getBaseType() == mx::RtTypeDef::BASETYPE_INTEGER);
    const mx::RtTypeDef* booleanType = mx::RtTypeDef::findType(mx::RtType::BOOLEAN);
    REQUIRE(booleanType != nullptr);
    REQUIRE(booleanType->getBaseType() == mx::RtTypeDef::BASETYPE_BOOLEAN);
    const mx::RtTypeDef* color2Type = mx::RtTypeDef::findType(mx::RtType::COLOR2);
    REQUIRE(color2Type != nullptr);
    REQUIRE(color2Type->getBaseType() == mx::RtTypeDef::BASETYPE_FLOAT);
    const mx::RtTypeDef* color3Type = mx::RtTypeDef::findType(mx::RtType::COLOR3);
    REQUIRE(color3Type != nullptr);
    REQUIRE(color3Type->getBaseType() == mx::RtTypeDef::BASETYPE_FLOAT);
    REQUIRE(color3Type->getSemantic() == mx::RtTypeDef::SEMANTIC_COLOR);
    const mx::RtTypeDef* color4Type = mx::RtTypeDef::findType(mx::RtType::COLOR4);
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
    const mx::RtTypeDef* fooType = mx::RtTypeDef::registerType(FOO, mx::RtTypeDef::BASETYPE_FLOAT, fooFuncs, mx::RtTypeDef::SEMANTIC_COLOR, 5);
    REQUIRE(fooType != nullptr);
    const mx::RtTypeDef* fooType2 = mx::RtTypeDef::findType(FOO);
    REQUIRE(fooType2 == fooType);

    // Test create/parse/copy values
    // An stage is needed to hold allocated data.
    mx::RtStage stage = mx::RtStage::createNew(MAIN);
    mx::RtObject rootPrim = stage.getRootPrim();

    mx::RtValue fooValue = fooType->createValue(rootPrim);
    REQUIRE(fooValue.asInt() == 7);
    fooType->fromStringValue("bar", fooValue);
    REQUIRE(fooValue.asInt() == 42);

    const mx::RtTypeDef* stringType = mx::RtTypeDef::findType(mx::RtType::STRING);
    mx::RtValue stringValue1 = stringType->createValue(rootPrim);
    mx::RtValue stringValue2 = stringType->createValue(rootPrim);
    stringValue1.asString() = "foobar";
    stringType->copyValue(stringValue1, stringValue2);
    REQUIRE(stringValue2.asString() == "foobar");

    mx::RtValue intValue = integerType->createValue(rootPrim);
    integerType->fromStringValue("12345", intValue);
    REQUIRE(intValue.asInt() == 12345);

    // Make sure we can't use a name already take
    REQUIRE_THROWS(mx::RtTypeDef::registerType(mx::RtType::COLOR3, mx::RtTypeDef::BASETYPE_FLOAT, fooFuncs));

    // Make sure we can't request an unknown type
    REQUIRE(mx::RtTypeDef::findType(mx::RtToken("bar")) == nullptr);

    // Make sure a type is connectable to its own type
    // TODO: Extend to test more types when type auto cast is implemented.
    REQUIRE(floatType->getValidConnectionTypes().count(floatType->getName()));
    REQUIRE(!floatType->getValidConnectionTypes().count(color4Type->getName()));

    // Test aggregate types component index/name
    REQUIRE(color4Type->getComponentIndex(R) == 0);
    REQUIRE(color4Type->getComponentIndex(G) == 1);
    REQUIRE(color4Type->getComponentIndex(B) == 2);
    REQUIRE(color4Type->getComponentIndex(A) == 3);
    REQUIRE(color4Type->getComponentName(0) == R);
    REQUIRE(color4Type->getComponentName(1) == G);
    REQUIRE(color4Type->getComponentName(2) == B);
    REQUIRE(color4Type->getComponentName(3) == A);
    REQUIRE_THROWS(color4Type->getComponentName(7));
}

TEST_CASE("Runtime: Paths", "[runtime]")
{
    mx::RtObject stageObj = mx::RtStage::createNew(ROOT);
    mx::RtStage stage(stageObj);

    mx::RtPath empty;
    REQUIRE(!stage.getPrimAtPath(empty).isValid());

    mx::RtPath root("/");
    REQUIRE(stage.getPrimAtPath(root).isValid());

    mx::RtObject graph = stage.createPrim("/graph", mx::RtNodeGraph::typeName());
    mx::RtObject subgraph = stage.createPrim("/graph/subgraph", mx::RtNodeGraph::typeName());
    mx::RtObject subsubgraph = stage.createPrim("/graph/subgraph/subsubgraph", mx::RtNodeGraph::typeName());

    mx::RtPath path1(graph);
    mx::RtPath path2(subgraph);
    mx::RtPath path3(subsubgraph);

    REQUIRE(stage.getPrimAtPath(path1) == graph);
    REQUIRE(stage.getPrimAtPath(path2) == subgraph);
    REQUIRE(stage.getPrimAtPath(path3) == subsubgraph);
    REQUIRE(path3.asString() == "/graph/subgraph/subsubgraph");

    mx::RtPath path4 = path1;
    REQUIRE(path4 == path1);
    REQUIRE(path4 != path2);
}

TEST_CASE("Runtime: Prims", "[runtime]")
{
    mx::RtObject stageObj = mx::RtStage::createNew(ROOT);
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
    nodeDef.setNodeTypeName(ADD);
    REQUIRE(nodeDef.getNodeTypeName() == ADD);

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

    mx::RtObject fooObj = stage.createPrim("/foo1", FOO);
    REQUIRE(fooObj.isValid());
    REQUIRE(fooObj.getObjType() == mx::RtObjType::PRIM);
    REQUIRE(fooObj.getObjTypeName() == mx::RtPrim::typeName());
    REQUIRE(fooObj.hasApi(mx::RtApiType::PRIM));
}

TEST_CASE("Runtime: Nodes", "[runtime]")
{
    mx::RtStage stage = mx::RtStage::createNew(ROOT);

    // Create a new nodedef object for defining an add node
    mx::RtNodeDef nodedef = stage.createPrim("/ND_add_float", mx::RtNodeDef::typeName());
    nodedef.setNodeTypeName(ADD);

    // Test adding metadata
    mx::RtTypedValue* version = nodedef.addMetadata(VERSION, mx::RtType::FLOAT);
    version->getValue().asFloat() = 1.0f;
    REQUIRE(version->getValue().asFloat() == 1.0);

    // Add attributes to the nodedef
    nodedef.createAttribute(IN1, mx::RtType::FLOAT);
    nodedef.createAttribute(IN2, mx::RtType::FLOAT);
    nodedef.createAttribute(OUT, mx::RtType::FLOAT, mx::RtAttrFlag::OUTPUT);

    // Test the new attributes
    mx::RtAttribute out = nodedef.getAttribute(OUT);
    REQUIRE(out.isValid());
    REQUIRE(out.isOutput());
    REQUIRE(!out.isConnectable());
    REQUIRE(out.getType() == mx::RtType::FLOAT);
    REQUIRE(out.getValue().asFloat() == 0.0f);
    mx::RtAttribute foo = nodedef.getAttribute(FOO);
    REQUIRE(!foo.isValid());
    mx::RtAttribute in1 = nodedef.getAttribute(IN1);
    REQUIRE(in1.isValid());
    REQUIRE(in1.isInput());
    REQUIRE(!in1.isConnectable());
    in1.getValue().asFloat() = 7.0f;
    REQUIRE(in1.getValue().asFloat() == 7.0f);

    // Test deleting an attribute
    nodedef.createAttribute(IN3, mx::RtType::FLOAT);
    mx::RtAttribute in3 = nodedef.getAttribute(IN3);
    REQUIRE(in3);
    nodedef.removeAttribute(in3.getName());
    in3 = nodedef.getAttribute(IN3);
    REQUIRE(!in3);

    // Node creation without a nodedef should throw.
    REQUIRE_THROWS(stage.createPrim("/add1", mx::RtNode::typeName()));

    // Create two new node instances from the add nodedef
    mx::RtObject add1Obj = stage.createPrim("/add1", mx::RtNode::typeName(), nodedef.getObject());
    mx::RtObject add2Obj = stage.createPrim("/add2", mx::RtNode::typeName(), nodedef.getObject());
    REQUIRE(add1Obj.isValid());
    REQUIRE(add2Obj.isValid());

    // Attach the node API to these objects
    mx::RtNode add1(add1Obj);
    mx::RtNode add2(add2Obj);
    REQUIRE(add1.getName() == "add1");
    REQUIRE(add2.getName() == "add2");

    // Get the node inputs
    mx::RtInput add1_in1 = add1.getInput(IN1);
    mx::RtInput add1_in2 = add1.getInput(IN2);
    mx::RtOutput add1_out = add1.getOutput(OUT);
    mx::RtInput add2_in1 = add2.getInput(IN1);
    mx::RtInput add2_in2 = add2.getInput(IN2);
    mx::RtOutput add2_out = add2.getOutput(OUT);
    REQUIRE(add1_in1.isValid());
    REQUIRE(add1_in2.isValid());
    REQUIRE(add1_out.isValid());
    REQUIRE(add2_in1.isValid());
    REQUIRE(add2_in2.isValid());
    REQUIRE(add2_out.isValid());

    // Test setting input metadata
    const mx::RtToken meter("meter");
    const mx::RtToken srgb("srgb");
    add1_in1.setUnit(meter);
    add1_in2.setColorSpace(srgb);
    REQUIRE(add1_in1.getUnit() == meter);
    REQUIRE(add1_in1.getColorSpace() == mx::EMPTY_TOKEN);
    REQUIRE(add1_in2.getUnit() == mx::EMPTY_TOKEN);
    REQUIRE(add1_in2.getColorSpace() == srgb);
    mx::RtTypedValue* fooData = add1_in1.addMetadata(FOO, mx::RtType::FLOAT);
    fooData->getValue().asFloat() = 7.0f;
    REQUIRE(fooData == add1_in1.getMetadata(FOO));
    add1_in1.removeMetadata(FOO);
    REQUIRE(nullptr == add1_in1.getMetadata(FOO));

    // Test port connectability
    REQUIRE(add1_out.isConnectable(add2_in1));
    REQUIRE(add2_in1.isConnectable(add1_out));
    REQUIRE(!add1_out.isConnectable(add1_in1));
    REQUIRE(!add1_out.isConnectable(add2_out));
    REQUIRE(!add2_in1.isConnectable(add1_in1));

    // Make port connections
    add1_out.connect(add2_in1);
    REQUIRE(add1_out.isConnected());
    REQUIRE(add2_in1.isConnected());

    // Try connecting already connected ports
    REQUIRE_THROWS(add1_out.connect(add2_in1));

    // Break port connections
    add1_out.disconnect(add2_in1);
    REQUIRE(!add1_out.isConnected());
    REQUIRE(!add2_in1.isConnected());

    // Make more port connections, now testing
    // the node connect method
    mx::RtNode::connect(add1_out, add2_in1);
    mx::RtNode::connect(add1_out, add2_in2);

    size_t numConnections = 0;
    std::vector<mx::RtObject> dest = { add2_in1.getObject(), add2_in2.getObject() };
    for (mx::RtInput input : add1_out.getConnections())
    {
        REQUIRE(input.getObject() == dest[numConnections++]);
    }
    REQUIRE(numConnections == 2);
    REQUIRE(add2_in1.getConnection() == add1_out);
    REQUIRE(add2_in2.getConnection() == add1_out);

    // Test node creation when name is not unique
    mx::RtNode add3 = stage.createPrim("/add1", mx::RtNode::typeName(), nodedef.getObject());
    REQUIRE(add3.getName() == "add3");

    // Test node creation with generated name
    mx::RtNode add4 = stage.createPrim("/", NONAME, mx::RtNode::typeName(), nodedef.getObject());
    REQUIRE(add4.getName() == "add4");

    // Find object by path
    mx::RtObject prim1 = stage.getPrimAtPath("/add3");
    REQUIRE(prim1.isValid());
    REQUIRE(prim1.hasApi(mx::RtApiType::NODE));
}

TEST_CASE("Runtime: NodeGraphs", "[runtime]")
{
    mx::RtStage stage = mx::RtStage::createNew(ROOT);

    // Create a new nodedef for an add node.
    mx::RtNodeDef addFloat = stage.createPrim("/ND_add_float", mx::RtNodeDef::typeName());
    addFloat.setNodeTypeName(ADD);
    addFloat.createAttribute(IN1, mx::RtType::FLOAT);
    addFloat.createAttribute(IN2, mx::RtType::FLOAT);
    addFloat.createAttribute(OUT, mx::RtType::FLOAT, mx::RtAttrFlag::OUTPUT);

    // Create a nodegraph object.
    mx::RtNodeGraph graph1 = stage.createPrim("/graph1", mx::RtNodeGraph::typeName());
    REQUIRE(graph1.isValid());

    // Create add nodes in the graph.
    mx::RtNode add1 = stage.createPrim("/graph1/add1", mx::RtNode::typeName(), addFloat.getObject());
    mx::RtNode add2 = stage.createPrim("/graph1/add2", mx::RtNode::typeName(), addFloat.getObject());
    REQUIRE(graph1.getChild(add1.getName()));
    REQUIRE(graph1.getChild(add2.getName()));

    // Test deleting a node.
    mx::RtNode add3 = stage.createPrim("/graph1/add3", mx::RtNode::typeName(), addFloat.getObject());
    mx::RtPath add3Path = add3.getPath();
    stage.removePrim(add3Path);
    REQUIRE(!graph1.getChild(add3Path.getName()));
    REQUIRE(!stage.getPrimAtPath(add3Path));

    // Add an interface to the graph.
    graph1.createAttribute(A, mx::RtType::FLOAT);
    graph1.createAttribute(B, mx::RtType::FLOAT);
    graph1.createAttribute(OUT, mx::RtType::FLOAT, mx::RtAttrFlag::OUTPUT);
    REQUIRE(graph1.getAttribute(A));
    REQUIRE(graph1.getAttribute(B));
    REQUIRE(graph1.getAttribute(OUT));
    REQUIRE(graph1.getInput(A));
    REQUIRE(graph1.getInput(B));
    REQUIRE(graph1.getOutput(OUT));
    REQUIRE(graph1.getInputSocket(A));
    REQUIRE(graph1.getInputSocket(B));
    REQUIRE(graph1.getOutputSocket(OUT));
    REQUIRE(graph1.getInputSocket(A).isOutput());
    REQUIRE(graph1.getInputSocket(B).isOutput());
    REQUIRE(graph1.getOutputSocket(OUT).isInput());

    // Test deleting an attribute.
    graph1.createAttribute(X, mx::RtType::FLOAT);
    REQUIRE(graph1.getInput(X));
    REQUIRE(graph1.getInputSocket(X));
    graph1.removeAttribute(X);
    REQUIRE(!graph1.getInput(X));
    REQUIRE(!graph1.getInputSocket(X));

    // Connect the graph nodes to each other and the interface.
    mx::RtNode::connect(graph1.getInputSocket(A), add1.getInput(IN1));
    mx::RtNode::connect(graph1.getInputSocket(B), add1.getInput(IN2));
    mx::RtNode::connect(add1.getOutput(OUT), add2.getInput(IN1));
    mx::RtNode::connect(graph1.getInputSocket(A), add2.getInput(IN2));
    mx::RtNode::connect(add2.getOutput(OUT), graph1.getOutputSocket(OUT));

    REQUIRE(graph1.getInputSocket(A).isConnected());
    REQUIRE(graph1.getInputSocket(B).isConnected());
    REQUIRE(graph1.getOutputSocket(OUT).getConnection() == add2.getOutput(OUT));

    // Test query of parent, root and stage.
    REQUIRE(add1.getParent() == graph1.getObject());
    REQUIRE(add1.getRoot() == stage.getRootPrim());
    REQUIRE(add1.getStage() == stage.getObject());
    REQUIRE(graph1.getParent() == stage.getRootPrim());
}

TEST_CASE("Runtime: FileIo", "[runtime]")
{
    mx::FileSearchPath searchPath;
    searchPath.append(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    {
        // Load in stdlib
        // Create a stage and import the document data.
        mx::RtObject stageObj = mx::RtStage::createNew(MAIN);
        mx::RtFileIo stageIo(stageObj);
        stageIo.readLibraries({ "stdlib" }, searchPath);

        // Get a nodegraph and write a dot file for inspection.
        mx::RtStage stage(stageObj);
        mx::RtNodeGraph graph = stage.getPrimAtPath("/NG_tiledimage_float");
        REQUIRE(graph);
        std::ofstream dotfile;
        dotfile.open(graph.getName().str() + ".dot");
        dotfile << graph.asStringDot();
        dotfile.close();

        // Create a new graph
        mx::RtNodeGraph graph1 = stage.createPrim(mx::RtNodeGraph::typeName());
        REQUIRE(graph1);
        REQUIRE(graph1.getPath() == "/nodegraph1");

        // Get a nodedef and create two node instances in the graph.
        mx::RtObject multiplyColor3 = stage.getPrimAtPath("/ND_multiply_color3");
        REQUIRE(multiplyColor3);
        mx::RtNode mult1 = stage.createPrim(graph1.getPath(), NONAME, mx::RtNode::typeName(), multiplyColor3);
        mx::RtNode mult2 = stage.createPrim(graph1.getPath(), NONAME, mx::RtNode::typeName(), multiplyColor3);
        REQUIRE(mult1);
        REQUIRE(mult2);

        // Get a nodedef and create an instance at stage root.
        mx::RtObject tiledimageFloat = stage.getPrimAtPath("/ND_tiledimage_float");
        REQUIRE(tiledimageFloat);
        mx::RtNode tiledimage1 = stage.createPrim(mx::RtNode::typeName(), tiledimageFloat);
        REQUIRE(tiledimage1);

        // Move it into the graph.
        stage.reparentPrim(tiledimage1.getPath(), graph1.getPath());

        // Save the stage to file for inspection.
        stageIo.write(stage.getName().str() + "_export.mtlx", nullptr);

        // Write out nodegraphs only.
        mx::RtWriteOptions writeOptions;
        writeOptions.writeFilter = [](const mx::RtObject& obj) -> bool
        {
            return (obj.hasApi(mx::RtApiType::NODEGRAPH));
        };
        stageIo.write(stage.getName().str() + "_nodegraph_export.mtlx", &writeOptions);

        // Write out only tiledimage nodes.
        writeOptions.writeFilter = [](const mx::RtObject& obj) -> bool
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
        stageIo.write(stage.getName().str() + "_tiledimage_export.mtlx", &writeOptions);
    }
/*
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
        mx::RtPort texcoord1_out = texcoord1.findPort(OUT);
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
    */
}

/*
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
    graph.addPort(OUT, mx::RtType::FLOAT, mx::RtPortFlag::OUTPUT);
    mx::RtPort graphIn = graph.findPort("in");
    mx::RtPort graphOut = graph.findPort(OUT);
    mx::RtPort graphInSocket = graph.findInputSocket("in");
    mx::RtPort graphOutSocket = graph.findOutputSocket(OUT);
    REQUIRE(graphIn);
    REQUIRE(graphOut);
    REQUIRE(graphInSocket);
    REQUIRE(graphOutSocket);

    // Add nodes to the graph.
    mx::RtNodeDef addNodeDef = mainStage.findElementByName("ND_add_float");
    mx::RtNode add1 = mx::RtNode::createNew(graph.getObject(), addNodeDef.getObject());
    mx::RtNode add2 = mx::RtNode::createNew(graph.getObject(), addNodeDef.getObject());
    mx::RtNode::connect(graphInSocket, add1.findPort(IN1));
    mx::RtNode::connect(add1.findPort(OUT), add2.findPort(IN1));
    mx::RtNode::connect(add2.findPort(OUT), graphOutSocket);
    // Add an unconnected node.
    mx::RtNode::createNew(graph.getObject(), addNodeDef.getObject());

    // Create a node on root level and connect it downstream after the graph.
    mx::RtNode add4 = mx::RtNode::createNew(mainStage.getObject(), addNodeDef.getObject());
    mx::RtNode::connect(graphOut, add4.findPort(IN1));

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
