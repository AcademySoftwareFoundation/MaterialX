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
#include <MaterialXFormat/Util.h>

#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtValue.h>
#include <MaterialXRuntime/RtStage.h>
#include <MaterialXRuntime/RtPrim.h>
#include <MaterialXRuntime/RtRelationship.h>
#include <MaterialXRuntime/RtAttribute.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtTypeDef.h>
#include <MaterialXRuntime/RtNameResolver.h>
#include <MaterialXRuntime/RtNode.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtBackdrop.h>
#include <MaterialXRuntime/RtGeneric.h>
#include <MaterialXRuntime/RtPath.h>
#include <MaterialXRuntime/RtFileIo.h>
#include <MaterialXRuntime/RtTraversal.h>
#include <MaterialXRuntime/RtLook.h>
#include <MaterialXRuntime/RtCollection.h>
#include <MaterialXRuntime/RtMessage.h>

#include <MaterialXRuntime/Commands/PrimCommands.h>
#include <MaterialXRuntime/Commands/AttributeCommands.h>
#include <MaterialXRuntime/Commands/UndoCommands.h>

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
    const mx::RtToken IN("in");
    const mx::RtToken REFLECTIVITY("reflectivity");
    const mx::RtToken SURFACESHADER("surfaceshader");
    const mx::RtToken FOO("foo");
    const mx::RtToken FOO1("foo1");
    const mx::RtToken BAR("bar");
    const mx::RtToken VERSION("version");
    const mx::RtToken ROOT("root");
    const mx::RtToken MAIN("main");
    const mx::RtToken LIBS("libs");
    const mx::RtToken NONAME("");
    const mx::RtToken STDLIB("stdlib");
    const mx::RtToken PBRLIB("pbrlib");
    const mx::RtToken BXDFLIB("bxdf");

    bool compareFiles(const mx::FilePath& filename1, const mx::FilePath& filename2)
    {
        std::ifstream file1(filename1.asString());
        std::string str1((std::istreambuf_iterator<char>(file1)), std::istreambuf_iterator<char>());
        std::ifstream file2(filename2.asString());
        std::string str2((std::istreambuf_iterator<char>(file2)), std::istreambuf_iterator<char>());
        return str1 == str2;
    }
}

TEST_CASE("Runtime: Material Element Upgrade", "[runtime]")
{
    mx::RtScopedApiHandle api;
    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    api->setSearchPath(searchPath);
    api->loadLibrary(STDLIB);
    api->loadLibrary(PBRLIB);
    api->loadLibrary(BXDFLIB);
    mx::FileSearchPath testSearchPath(mx::FilePath::getCurrentPath() /
        "resources" /
        "Materials" /
        "TestSuite" /
        "stdlib" /
        "upgrade" );
    mx::RtStagePtr defaultStage = api->createStage(mx::RtToken("defaultStage"));
    mx::RtFileIo fileIo(defaultStage);
    mx::RtReadOptions options;
    options.applyFutureUpdates = true;
    fileIo.read("material_element_to_surface_material.mtlx", testSearchPath, &options);
    mx::RtPrim mixNodeGraphPrim = defaultStage->getPrimAtPath("NG_aiMixColor31");
    REQUIRE(mixNodeGraphPrim);
    mx::RtNodeGraph mixNodeGraph(mixNodeGraphPrim);
    REQUIRE(mixNodeGraph);
    mx::RtOutput mixNodeGraphOutput = mixNodeGraph.getOutput(mx::RtToken("out"));
    REQUIRE(mixNodeGraphOutput);
    mx::RtConnectionIterator iter = mixNodeGraphOutput.getConnections();
    while (!iter.isDone())
    {
        REQUIRE((*iter).getName() == "base_color");
        break;
    }
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
    mx::RtScopedApiHandle api;

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
    mx::RtStagePtr stage = api->createStage(MAIN);
    mx::RtPrim rootPrim = stage->getRootPrim();

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
    mx::RtValue::fromString(mx::RtType::BOOLEAN, "1", value);
    REQUIRE(value.asBool());
    mx::RtValue::fromString(mx::RtType::BOOLEAN, "0", value);
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
    mx::RtScopedApiHandle api;

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
    mx::RtStagePtr stage = api->createStage(MAIN);
    mx::RtPrim rootPrim = stage->getRootPrim();

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
    std::string value;
    integerType->toStringValue(intValue, value);
    REQUIRE(value == "12345");

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

namespace MaterialX
{

// Test adding reference count to a class
class Foo : public mx::RtRefCounted<Foo>
{
public:
    Foo() : v(0)
    {
        construct++;
    }
    ~Foo()
    {
        deconstruct++;
    }
    int v;
    static int construct;
    static int deconstruct;
    RT_FRIEND_REF_PTR_FUNCTIONS(Foo)
};
int Foo::construct = 0;
int Foo::deconstruct = 0;
RT_DECLARE_REF_PTR_TYPE(Foo, FooPtr)
RT_DEFINE_REF_PTR_FUNCTIONS(Foo)

} // namespace MaterialX

TEST_CASE("Runtime: RefCount", "[runtime]")
{
    mx::FooPtr foo1 = mx::FooPtr(new mx::Foo());
    mx::FooPtr foo2 = mx::FooPtr(new mx::Foo());
    REQUIRE(mx::Foo::construct == 2);
    REQUIRE(mx::Foo::deconstruct == 0);
    foo1->v = 1;
    foo2->v = 2;
    REQUIRE(foo1->refCount() == 1);
    REQUIRE(foo2->refCount() == 1);
    mx::FooPtr foo3 = foo1;
    REQUIRE(foo3 == foo1);
    REQUIRE(foo3->v == 1);
    REQUIRE(foo1->refCount() == 2);

    foo3.reset(foo2.get());
    REQUIRE(foo1->refCount() == 1);
    REQUIRE(foo2->refCount() == 2);
    foo3.reset();
    REQUIRE(foo1->refCount() == 1);
    REQUIRE(foo2->refCount() == 1);

    {
        mx::FooPtr foo4 = foo2;
        REQUIRE(foo4 == foo2);
        REQUIRE(foo4->v == 2);
        REQUIRE(foo2->refCount() == 2);

        mx::FooPtr foo5 = mx::FooPtr(new mx::Foo());
        foo5->v = 7;
        foo1.reset(foo5.get());
        REQUIRE(mx::Foo::construct == 3);
        REQUIRE(mx::Foo::deconstruct == 1);
    }
    REQUIRE(mx::Foo::construct == 3);
    REQUIRE(mx::Foo::deconstruct == 1);

    REQUIRE(foo1->refCount() == 1);
    REQUIRE(foo2->refCount() == 1);
    REQUIRE(foo1->v == 7);
    REQUIRE(foo2->v == 2);

    foo1.reset();
    foo2.reset();
    REQUIRE(mx::Foo::construct == 3);
    REQUIRE(mx::Foo::deconstruct == 3);
}

TEST_CASE("Runtime: Paths", "[runtime]")
{
    mx::RtScopedApiHandle api;

    mx::RtStagePtr stage = api->createStage(ROOT);

    mx::RtPath empty;
    REQUIRE(!stage->getPrimAtPath(empty).isValid());

    mx::RtPath root("/");
    REQUIRE(stage->getPrimAtPath(root).isValid());

    mx::RtObject graph = stage->createPrim("/graph", mx::RtNodeGraph::typeName());
    mx::RtObject subgraph = stage->createPrim("/graph/subgraph", mx::RtNodeGraph::typeName());
    mx::RtObject subsubgraph = stage->createPrim("/graph/subgraph/subsubgraph", mx::RtNodeGraph::typeName());

    mx::RtPath path1(graph);
    mx::RtPath path2(subgraph);
    mx::RtPath path3(subsubgraph);

    REQUIRE(stage->getPrimAtPath(path1) == graph);
    REQUIRE(stage->getPrimAtPath(path2) == subgraph);
    REQUIRE(stage->getPrimAtPath(path3) == subsubgraph);
    REQUIRE(path3.asString() == "/graph/subgraph/subsubgraph");

    mx::RtPath path4 = path1;
    REQUIRE(path4 == path1);
    REQUIRE(path4 != path2);
}

TEST_CASE("Runtime: Prims", "[runtime]")
{
    mx::RtScopedApiHandle api;

    mx::RtStagePtr stage = api->createStage(ROOT);
    REQUIRE(stage);

    // Test creating a prim of each type

    mx::RtPrim nodedefPrim = stage->createPrim("/ND_foo_float", mx::RtNodeDef::typeName());
    REQUIRE(nodedefPrim);
    REQUIRE(nodedefPrim.isA<mx::RtPrim>());
    REQUIRE(nodedefPrim.isValid());
    REQUIRE(nodedefPrim.getTypeName() == mx::RtNodeDef::typeName());
    REQUIRE(nodedefPrim.hasApi<mx::RtNodeDef>());

    mx::RtNodeDef nodedef(nodedefPrim);
    REQUIRE(nodedef);
    REQUIRE(nodedef.isValid());
    REQUIRE(nodedef.getTypeInfo().getShortTypeName() == mx::RtNodeDef::typeName());
    REQUIRE(nodedef.getName() == mx::RtToken("ND_foo_float"));
    nodedef.setNode(FOO);
    REQUIRE(nodedef.getNode() == FOO);
    REQUIRE_THROWS(stage->createPrim(nodedef.getName()));
    nodedef.registerMasterPrim();
    REQUIRE(stage->createPrim(nodedef.getName()));

    mx::RtPrim nodePrim = stage->createPrim(nodedefPrim.getName());
    REQUIRE(nodePrim);
    REQUIRE(nodePrim.hasApi<mx::RtNode>());
    mx::RtNode node(nodePrim);
    REQUIRE(node);
    REQUIRE(node.getTypeInfo().getShortTypeName() == mx::RtNode::typeName());
    REQUIRE(node.getName() == mx::RtToken("foo1"));
    REQUIRE(node.getNodeDef() == nodedefPrim);

    mx::RtPrim graphPrim = stage->createPrim(mx::RtNodeGraph::typeName());
    REQUIRE(graphPrim);
    REQUIRE(graphPrim.hasApi<mx::RtNodeGraph>());
    mx::RtNodeGraph graph(graphPrim);
    REQUIRE(graph);
    REQUIRE(graph.getName() == mx::RtToken("nodegraph1"));
    REQUIRE(graph.getTypeInfo().getShortTypeName() == mx::RtNodeGraph::typeName());
    REQUIRE(graph.getTypeInfo().numBaseClasses() == 1);
    REQUIRE(graph.getTypeInfo().getBaseClassType(0) == mx::RtNode::typeName());
    REQUIRE(graph.getTypeInfo().isCompatible(mx::RtNode::typeName()));
    REQUIRE(graphPrim.hasApi<mx::RtNode>());
    mx::RtNode graphNode(graphPrim);
    REQUIRE(graphNode);
    REQUIRE(graphNode.getName() == graph.getName());

    mx::RtPrim backdropPrim = stage->createPrim(mx::RtBackdrop::typeName());
    REQUIRE(backdropPrim);
    REQUIRE(backdropPrim.hasApi<mx::RtBackdrop>());
    mx::RtBackdrop backdrop(backdropPrim);
    REQUIRE(backdrop);
    REQUIRE(backdrop.getTypeInfo().getShortTypeName() == mx::RtBackdrop::typeName());
    backdrop.getContains().addTarget(node.getPrim());
    backdrop.getContains().addTarget(graph.getPrim());
    REQUIRE(backdrop.getContains().hasTargets());
    backdrop.getContains().clearTargets();
    REQUIRE(!backdrop.getContains().hasTargets());
    backdrop.getNote().getValue().asString() = "These aren't the Droids you're looking for";
    REQUIRE(backdrop.getNote().getValue().asString() == "These aren't the Droids you're looking for");
    REQUIRE(backdropPrim.getRelationship(backdrop.getContains().getName()) == backdrop.getContains());
    bool found = false;
    for (auto rel: backdropPrim.getRelationships()) {
        if (rel.getName() == backdrop.getContains().getName()) {
            found = true;
            break;
        }
    }
    REQUIRE(found);

    mx::RtPrim genericPrim = stage->createPrim("/generic1", mx::RtGeneric::typeName());
    REQUIRE(genericPrim);
    REQUIRE(genericPrim.hasApi<mx::RtGeneric>());
    mx::RtGeneric generic(genericPrim);
    REQUIRE(generic);
    REQUIRE(generic.getTypeInfo().getShortTypeName() == mx::RtGeneric::typeName());
    mx::RtToken kind("mykindofprim");
    generic.setKind(kind);
    REQUIRE(generic.getKind() == kind);

    // Test object casting
    REQUIRE(backdropPrim.isA<mx::RtObject>());
    REQUIRE(backdropPrim.isA<mx::RtPrim>());
    mx::RtObject obj1 = backdrop.getNote();
    mx::RtObject obj2 = backdrop.getContains();
    REQUIRE(obj1.isA<mx::RtAttribute>());
    REQUIRE(!obj1.isA<mx::RtRelationship>());
    REQUIRE(obj2.isA<mx::RtRelationship>());
    REQUIRE(!obj2.isA<mx::RtAttribute>());
    mx::RtAttribute attr1 = obj1.asA<mx::RtAttribute>();
    mx::RtAttribute attr2 = obj2.asA<mx::RtAttribute>();
    REQUIRE(attr1);
    REQUIRE(!attr2);

    // Test object life-time management
    mx::RtInput graph_in = graph.createInput(IN, mx::RtType::FLOAT);
    mx::RtOutput graph_out = graph.createOutput(OUT, mx::RtType::FLOAT);
    mx::RtPrim node1 = stage->createPrim(graph.getPath(), mx::RtToken("node1"), nodedefPrim.getName());
    mx::RtPrim node2 = stage->createPrim(graph.getPath(), mx::RtToken("node1"), nodedefPrim.getName());
    REQUIRE(node1.isValid());
    stage->removePrim(node1.getPath());
    REQUIRE(!node1.isValid());
    REQUIRE(graph_in.isValid());
    graph.removeInput(graph_in.getName());
    REQUIRE(!graph_in.isValid());
    stage->removePrim(graph.getPath());
    REQUIRE(!graph_in.isValid());
    REQUIRE(!node2.isValid());
    REQUIRE(!graph.isValid());
    REQUIRE_THROWS(graph.getName());
    REQUIRE_THROWS(graph_in.isConnected());
}

TEST_CASE("Runtime: Nodes", "[runtime]")
{
    mx::RtScopedApiHandle api;

    mx::RtStagePtr stage = api->createStage(ROOT);

    // Create a new nodedef object for defining an add node
    mx::RtNodeDef nodedef = stage->createPrim("/ND_add_float", mx::RtNodeDef::typeName());
    nodedef.setNode(ADD);

    // Test adding metadata
    mx::RtTypedValue* version = nodedef.addMetadata(VERSION, mx::RtType::FLOAT);
    version->getValue().asFloat() = 1.0f;
    REQUIRE(version->getValue().asFloat() == 1.0);

    // Add attributes to the nodedef
    nodedef.createInput(IN1, mx::RtType::FLOAT);
    nodedef.createInput(IN2, mx::RtType::FLOAT);
    nodedef.createOutput(OUT, mx::RtType::FLOAT);

    // Test the new attributes
    mx::RtOutput out = nodedef.getOutput(OUT);
    REQUIRE(out);
    REQUIRE(out.getType() == mx::RtType::FLOAT);
    REQUIRE(out.getValue().asFloat() == 0.0f);
    mx::RtInput foo = nodedef.getInput(FOO);
    REQUIRE(!foo);
    mx::RtInput in1 = nodedef.getInput(IN1);
    REQUIRE(in1);
    in1.getValue().asFloat() = 7.0f;
    REQUIRE(in1.getValue().asFloat() == 7.0f);

    // Test deleting an input
    nodedef.createInput(IN3, mx::RtType::FLOAT);
    mx::RtInput in3 = nodedef.getInput(IN3);
    REQUIRE(in3);
    nodedef.removeInput(in3.getName());
    in3 = nodedef.getInput(IN3);
    REQUIRE(!in3);

    // Node creation without a valid nodedef should throw.
    REQUIRE_THROWS(stage->createPrim("/add1", FOO));

    // Register as a master prim so we can create node instance from it.
    nodedef.registerMasterPrim();

    // Create two new node instances from the add nodedef
    mx::RtPrim add1Prim = stage->createPrim("/add1", nodedef.getName());
    mx::RtPrim add2Prim = stage->createPrim("/add2", nodedef.getName());
    REQUIRE(add1Prim);
    REQUIRE(add1Prim);

    // Attach the node API schema to these objects
    mx::RtNode add1(add1Prim);
    mx::RtNode add2(add2Prim);
    REQUIRE(add1.getName() == "add1");
    REQUIRE(add2.getName() == "add2");

    // Get the node inputs
    mx::RtInput add1_in1 = add1.getInput(IN1);
    mx::RtInput add1_in2 = add1.getInput(IN2);
    mx::RtOutput add1_out = add1.getOutput(OUT);
    mx::RtInput add2_in1 = add2.getInput(IN1);
    mx::RtInput add2_in2 = add2.getInput(IN2);
    mx::RtOutput add2_out = add2.getOutput(OUT);
    REQUIRE(add1_in1);
    REQUIRE(add1_in2);
    REQUIRE(add1_out);
    REQUIRE(add2_in1);
    REQUIRE(add2_in2);
    REQUIRE(add2_out);

    // Test invalid input and output.
    mx::RtInput invalid_in = add2.getInput(OUT);
    mx::RtOutput invalid_out = add2.getOutput(IN1);
    REQUIRE(!invalid_in);
    REQUIRE(!invalid_out);

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

    // Make more port connections
    add1_out.connect(add2_in1);
    add1_out.connect(add2_in2);

    size_t numConnections = 0;
    std::vector<mx::RtObject> dest = { add2_in1, add2_in2 };
    for (mx::RtObject input : add1_out.getConnections())
    {
        REQUIRE(input == dest[numConnections++]);
    }
    REQUIRE(numConnections == 2);
    REQUIRE(add2_in1.getConnection() == add1_out);
    REQUIRE(add2_in2.getConnection() == add1_out);

    // Test node creation when name is not unique
    mx::RtNode add3 = stage->createPrim("/add1", nodedef.getName());
    REQUIRE(add3.getName() == "add3");

    // Test node creation with generated name
    mx::RtNode add4 = stage->createPrim("/", NONAME, nodedef.getName());
    REQUIRE(add4.getName() == "add");

    // Find object by path
    mx::RtPrim prim1 = stage->getPrimAtPath("/add3");
    REQUIRE(prim1);
    REQUIRE(mx::RtNode(prim1).isValid());
}

TEST_CASE("Runtime: NodeGraphs", "[runtime]")
{
    mx::RtScopedApiHandle api;

    mx::RtStagePtr stage = api->createStage(ROOT);

    // Create a new nodedef for an add node.
    mx::RtNodeDef addFloat = stage->createPrim("/ND_add_float", mx::RtNodeDef::typeName());
    addFloat.setNode(ADD);
    addFloat.createInput(IN1, mx::RtType::FLOAT);
    addFloat.createInput(IN2, mx::RtType::FLOAT);
    addFloat.createOutput(OUT, mx::RtType::FLOAT);
    addFloat.registerMasterPrim();

    // Create a nodegraph object.
    mx::RtNodeGraph graph1 = stage->createPrim("/graph1", mx::RtNodeGraph::typeName());
    REQUIRE(graph1.isValid());

    // Create add nodes in the graph.
    mx::RtNode add1 = stage->createPrim("/graph1/add1", addFloat.getName());
    mx::RtNode add2 = stage->createPrim("/graph1/add2", addFloat.getName());
    REQUIRE(graph1.getNode(add1.getName()));
    REQUIRE(graph1.getNode(add2.getName()));

    // Test deleting a node.
    mx::RtNode add3 = stage->createPrim("/graph1/add3", addFloat.getName());
    mx::RtPath add3Path = add3.getPath();
    stage->removePrim(add3Path);
    REQUIRE(!graph1.getNode(add3Path.getName()));
    REQUIRE(!stage->getPrimAtPath(add3Path));

    // Add an interface to the graph.
    mx::RtInput Ainput = graph1.createInput(A, mx::RtType::FLOAT);
    Ainput.setValueString("0.3");
    mx::RtInput Binput = graph1.createInput(B, mx::RtType::FLOAT);
    Binput.setValueString("0.1");
    graph1.createOutput(OUT, mx::RtType::FLOAT);
    REQUIRE(graph1.getPrim().getAttribute(A));
    REQUIRE(graph1.getPrim().getAttribute(B));
    REQUIRE(graph1.getPrim().getAttribute(OUT));
    REQUIRE(graph1.getInput(A));
    REQUIRE(graph1.getInput(B));
    REQUIRE(graph1.getOutput(OUT));
    REQUIRE(graph1.getInputSocket(A));
    REQUIRE(graph1.getInputSocket(B));
    REQUIRE(graph1.getOutputSocket(OUT));

    // Test renaming an input.
    graph1.createInput(FOO, mx::RtType::FLOAT);
    REQUIRE(graph1.getInput(FOO));
    REQUIRE(graph1.getInputSocket(FOO));
    graph1.renameInput(FOO, X);
    REQUIRE(!graph1.getInput(FOO));
    REQUIRE(!graph1.getInputSocket(FOO));
    REQUIRE(graph1.getInput(X));
    REQUIRE(graph1.getInputSocket(X));

    // Test deleting an input.
    graph1.removeInput(X);
    REQUIRE(!graph1.getInput(X));
    REQUIRE(!graph1.getInputSocket(X));

    // Connect the graph nodes to each other and the interface.
    graph1.getInputSocket(A).connect(add1.getInput(IN1));
    graph1.getInputSocket(B).connect(add1.getInput(IN2));
    add1.getOutput(OUT).connect(add2.getInput(IN1));
    graph1.getInputSocket(A).connect(add2.getInput(IN2));
    add2.getOutput(OUT).connect(graph1.getOutputSocket(OUT));

    REQUIRE(graph1.getInputSocket(A).isConnected());
    REQUIRE(graph1.getInputSocket(B).isConnected());
    REQUIRE(graph1.getOutputSocket(OUT).getConnection() == add2.getOutput(OUT));

    // Test query of parent, root and stage.
    REQUIRE(add1.getPrim().getParent() == graph1.getPrim());
    REQUIRE(add1.getPrim().getRoot() == stage->getRootPrim());
    REQUIRE(add1.getPrim().getStage().lock() == stage);
    REQUIRE(graph1.getPrim().getParent() == stage->getRootPrim());

    // Test creating a nodedef from a nodegraph
    const mx::RtToken NG_ADDGRAPH("NG_addgraph");
    const mx::RtToken ND_ADDGRAPH("ND_addgraph");
    const mx::RtToken ADDGRAPH("addgraph");
    const mx::RtToken MATH_GROUP("math");
    const mx::RtToken ADDGRAPH_VERSION("3.4");
    const mx::RtToken ADDGRAPH_TARGET("mytarget");
    bool isDefaultVersion = false;
    stage->renamePrim(graph1.getPath(), NG_ADDGRAPH);
    mx::RtPrim addgraphPrim = stage->createNodeDef(graph1, ND_ADDGRAPH, ADDGRAPH, ADDGRAPH_VERSION, isDefaultVersion, MATH_GROUP);
    mx::RtNodeDef addgraphDef(addgraphPrim);    

    REQUIRE(graph1.getDefinition() == ND_ADDGRAPH);
    REQUIRE(graph1.getVersion() == ADDGRAPH_VERSION);
    REQUIRE(addgraphDef.isMasterPrim());
    REQUIRE(addgraphDef.numInputs() == 2);
    REQUIRE(addgraphDef.numOutputs() == 1);
    REQUIRE(addgraphDef.getOutput().getName() == OUT);
    REQUIRE(addgraphDef.getName() == ND_ADDGRAPH);
    REQUIRE(addgraphDef.getNode() == ADDGRAPH);
    REQUIRE(addgraphDef.getNodeGroup() == MATH_GROUP);
    REQUIRE(addgraphDef.getVersion() == ADDGRAPH_VERSION);
    REQUIRE_FALSE(addgraphDef.getIsDefaultVersion());
    addgraphDef.setTarget(ADDGRAPH_TARGET);
    REQUIRE(addgraphDef.getTarget() == ADDGRAPH_TARGET);

    // Check implementation search based on nodegraph.
    mx::RtPrim addGraphImpl = stage->getImplementation(addgraphDef);
    // Exact version check
    {
        REQUIRE(addGraphImpl.getPath() == graph1.getPath());
    }
    // Bad version check
    {
        graph1.setVersion(mx::RtToken("badVersion")); 
        addGraphImpl = stage->getImplementation(addgraphDef);
        REQUIRE_FALSE(addGraphImpl.isValid());
        graph1.setVersion(ADDGRAPH_VERSION);
    }

    // Check instance creation:
    mx::RtPrim agPrim = stage->createPrim("addgraph1", ND_ADDGRAPH);
    REQUIRE(agPrim.isValid());
    mx::RtNode agNode(agPrim);
    {
        // 1. Metadata like version should be copiedbut not target or node.
        mx::RtTypedValue* agVersion = agNode.getMetadata(mx::RtNodeDef::VERSION);
        REQUIRE(agVersion->getValueString() == ADDGRAPH_VERSION);
        mx::RtTypedValue* agTarget = agNode.getMetadata(mx::RtNodeDef::TARGET);
        REQUIRE(!agTarget);
        mx::RtTypedValue* agNodeValue = agNode.getMetadata(mx::RtNodeDef::NODE);
        REQUIRE(!agNodeValue);
    }

    // 2. The assocaited nodedef can be found.
    {
        mx::RtPrim agNodeDefinition = agNode.getNodeDef();
        REQUIRE(agNodeDefinition.getPath() == addgraphDef.getPath());
    }

    // Check export to MTLX document:
    mx::RtFileIo stageIo(stage);
    mx::RtTokenVec names;
    names.push_back(ND_ADDGRAPH);
    stageIo.writeDefinitions("ND_addgraph.mtlx", names);

    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, "ND_addgraph.mtlx");
    doc->validate();
    mx::NodeDefPtr nodeDef = doc->getNodeDef(ND_ADDGRAPH.str());
    {
        // 1. Check nodedef
        REQUIRE(nodeDef);
        REQUIRE(nodeDef->getVersionString() == ADDGRAPH_VERSION.str());
        REQUIRE(nodeDef->getTarget() == ADDGRAPH_TARGET.str());
        std::vector<mx::InputPtr> inputs = nodeDef->getInputs();
        bool inputCheck =
            (inputs.size() == 2) &&
            (inputs[0]->getName() == "a") &&
            (inputs[0]->getType() == "float") &&
            (inputs[0]->getValueString() == "0.3") &&
            (inputs[1]->getName() == "b") &&
            (inputs[1]->getType() == "float") &&
            (inputs[1]->getValueString() == "0.1");
        REQUIRE(inputCheck);
        mx::OutputPtr out = nodeDef->getOutput("out");
        bool outputCheck = out &&
            (out->getName() == "out") &&
            (out->getType() == "float") &&
            (out->getValueString() == "0");
        REQUIRE(outputCheck);
    }

    // 2. Check the nodegraph for the nodedef
    {
        mx::InterfaceElementPtr inter = nodeDef->getImplementation();
        mx::NodeGraphPtr nodeGraph = inter->asA<mx::NodeGraph>();
        REQUIRE((nodeGraph && nodeGraph->getOutputs().size() == 1));
        for (mx::TreeIterator it = nodeGraph->traverseTree().begin(); it != mx::TreeIterator::end(); ++it)
        {
            mx::ValueElementPtr input = it.getElement()->asA<mx::ValueElement>();
            if (input)
            {
                if (input->getName() == "in1" && input->getAttribute("nodename").empty())
                {
                    REQUIRE((input->getInterfaceName() == "a"));
                }
                else if (input->getName() == "in2")
                {
                    if (input->getParent())
                    {
                        bool interfaceNameMatched = true;
                        if (input->getParent()->getName() == "add2")
                            interfaceNameMatched = (input->getInterfaceName() == "a");
                        else
                            interfaceNameMatched = (input->getInterfaceName() == "b");
                        REQUIRE(interfaceNameMatched);
                    }
                }
            }
        }
    }

    // 3. Check instance creation
    {
        stageIo.write("addgraph_example.mtlx");
        doc = mx::createDocument();
        mx::readFromXmlFile(doc, "addgraph_example.mtlx");
        doc->validate();
        mx::ElementPtr agInstance = doc->getChild("addgraph1");
        REQUIRE(agInstance);
        bool instanceVersionSaved = agInstance->getAttribute(mx::RtNodeDef::VERSION.str()) == ADDGRAPH_VERSION;
        REQUIRE(instanceVersionSaved);
        bool instanceTargetNotSaved = agInstance->getAttribute(mx::RtNodeDef::TARGET.str()) == mx::EMPTY_STRING;
        REQUIRE(instanceTargetNotSaved);
        bool instanceNodeNotSaved = agInstance->getAttribute(mx::RtNodeDef::NODE.str()) == mx::EMPTY_STRING;
        REQUIRE(instanceNodeNotSaved);
    }
}

TEST_CASE("Runtime: FileIo", "[runtime]")
{
    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    {
        mx::RtScopedApiHandle api;

        // Load in stdlib
        api->setSearchPath(searchPath);
        api->loadLibrary(STDLIB);

        // Create a stage.
        mx::RtStagePtr stage = api->createStage(MAIN);

        // Get a nodegraph from the library and write a dot file for inspection.
        mx::RtNodeGraph graph = api->getLibrary()->getPrimAtPath("/NG_tiledimage_float");
        REQUIRE(graph);
        std::ofstream dotfile;
        dotfile.open(graph.getName().str() + ".dot");
        dotfile << graph.asStringDot();
        dotfile.close();

        // Create a new graph
        mx::RtNodeGraph graph1 = stage->createPrim(mx::RtNodeGraph::typeName());
        REQUIRE(graph1);
        REQUIRE(graph1.getPath() == "/nodegraph1");

        // Get a nodedef and create two node instances in the graph.
        const mx::RtToken multiplyColor3("ND_multiply_color3");
        mx::RtNode mult1 = stage->createPrim(graph1.getPath(), NONAME, multiplyColor3);
        mx::RtNode mult2 = stage->createPrim(graph1.getPath(), NONAME, multiplyColor3);
        REQUIRE(mult1);
        REQUIRE(mult2);

        // Create a node instance at stage root.
        const mx::RtToken tiledimageFloat("ND_tiledimage_float");
        mx::RtNode tiledimage1 = stage->createPrim(tiledimageFloat);
        REQUIRE(tiledimage1);

        // Move it into the graph.
        stage->reparentPrim(tiledimage1.getPath(), graph1.getPath());

        // Save the stage to file for inspection.
        mx::RtFileIo stageIo(stage);
        stageIo.write(stage->getName().str() + "_export.mtlx", nullptr);

        // Write out nodegraphs only.
        mx::RtWriteOptions writeOptions;

        writeOptions.writeFilter = mx::RtSchemaPredicate<mx::RtNodeGraph>();
        stageIo.write(stage->getName().str() + "_nodegraph_export.mtlx", &writeOptions);
    }

    {
        mx::RtScopedApiHandle api;

        // Load in stdlib
        api->setSearchPath(searchPath);
        api->loadLibrary(STDLIB);

        // Create a new working space stage
        mx::RtStagePtr stage = api->createStage(MAIN);

        // Create a small node network.
        const mx::RtToken tiledimageDef("ND_tiledimage_color3");
        const mx::RtToken texcoordDef("ND_texcoord_vector2");
        mx::RtNode tiledimage1 = stage->createPrim(tiledimageDef);
        mx::RtNode texcoord1 = stage->createPrim(texcoordDef);
        REQUIRE(tiledimage1);
        REQUIRE(texcoord1);
        mx::RtInput tiledimage1_texcoord = tiledimage1.getInput(mx::RtToken("texcoord"));
        mx::RtInput tiledimage1_file = tiledimage1.getInput(mx::RtToken("file"));
        mx::RtInput texcoord1_index = texcoord1.getInput(mx::RtToken("index"));
        mx::RtOutput texcoord1_out = texcoord1.getOutput(OUT);
        REQUIRE(tiledimage1_texcoord);
        REQUIRE(tiledimage1_file);
        REQUIRE(texcoord1_index);
        REQUIRE(texcoord1_out);
        tiledimage1_file.getValue().asString() = "myimagetexture.png";
        texcoord1_out.connect(tiledimage1_texcoord);
        texcoord1_index.getValue().asInt() = 2;

        const mx::FilePath fileExport("file_export.mtlx");
        const mx::FilePath streamExport("stream_export.mtlx");

        mx::RtFileIo fileIO(stage);
        fileIO.write(fileExport);

        // Test write to stream.
        std::stringstream stream1;
        fileIO.write(stream1);
        REQUIRE(!stream1.str().empty());

        // Test read from stream.
        mx::RtStagePtr streamStage = api->createStage(MAIN);
        mx::RtFileIo streamFileIO(streamStage);
        streamFileIO.read(stream1);
        streamFileIO.write(streamExport);

        // Compare file contents.
        REQUIRE(compareFiles(fileExport, streamExport));
    }
}

TEST_CASE("Runtime: FileIo no validatioon", "[runtime]")
{
    mx::RtScopedApiHandle api;

    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    api->setSearchPath(searchPath);
    api->loadLibrary(STDLIB);
    api->loadLibrary(PBRLIB);

    mx::FileSearchPath bxdfPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries") / mx::FilePath("bxdf"));
    mx::RtReadOptions options;
    options.validateDocument = false;

    mx::RtStagePtr stage = api->createStage(MAIN);
    mx::RtFileIo fileIo(stage);
    REQUIRE_THROWS(fileIo.read("standard_surface.mtlx", bxdfPath));
    REQUIRE_NOTHROW(fileIo.read("standard_surface.mtlx", bxdfPath, &options));

    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, "standard_surface.mtlx", bxdfPath);
    std::stringstream stream;
    mx::writeToXmlStream(doc, stream);
    stage = api->createStage(MAIN);
    mx::RtFileIo fileIo2(stage);
    REQUIRE_THROWS(fileIo2.read(stream));
    std::stringstream stream2;
    mx::writeToXmlStream(doc, stream2);
    REQUIRE_NOTHROW(fileIo2.read(stream2, &options));
}

TEST_CASE("Runtime: DefaultLook", "[runtime]")
{
    mx::RtScopedApiHandle api;

    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    api->setSearchPath(searchPath);
    api->loadLibrary(STDLIB);
    api->loadLibrary(PBRLIB);
    api->loadLibrary(BXDFLIB);

    mx::RtStagePtr defaultStage = api->createStage(mx::RtToken("defaultStage"));
    defaultStage->addReference(api->getLibrary());

    mx::FileSearchPath lookSearchPath(mx::FilePath::getCurrentPath() /
                                      "resources" /
                                      "LookDev");
    mx::RtFileIo fileIo(defaultStage);
    fileIo.read("defaultLook.mtlx", lookSearchPath);
    fileIo.read("emptyLook.mtlx", lookSearchPath);
}

TEST_CASE("Runtime: Conflict resolution", "[runtime]")
{
    mx::RtScopedApiHandle api;

    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    api->setSearchPath(searchPath);
    api->loadLibrary(STDLIB);
    api->loadLibrary(PBRLIB);
    api->loadLibrary(BXDFLIB);

    mx::RtStagePtr defaultStage = api->createStage(mx::RtToken("defaultStage"));
    defaultStage->addReference(api->getLibrary());

    mx::FileSearchPath lookSearchPath(mx::FilePath::getCurrentPath() /
                                      "resources" /
                                      "LookDev");
    mx::RtFileIo fileIo(defaultStage);
    fileIo.read("defaultLook.mtlx", lookSearchPath);
    int numBefore = 0;
    auto stageTraverser = defaultStage->traverse();
    while (!stageTraverser.isDone()) {
        ++numBefore;
        ++stageTraverser;
    }

    fileIo.read("defaultLook.mtlx", lookSearchPath);
    int numAfter = 0;
    stageTraverser = defaultStage->traverse();
    while (!stageTraverser.isDone()) {
        ++numAfter;
        ++stageTraverser;
    }
    std::cout << std::endl;
    // Everything duplicated except the nodedef ND_default_shader and its
    // nodegraph implementation NG_default_shader
    REQUIRE(numBefore * 2 - 2 == numAfter);

    // And all duplicates correctly connected:
    mx::RtPrim lg1 = defaultStage->getPrimAtPath("/defaultLookGroup1");
    REQUIRE(lg1);
    mx::RtLookGroup lookgroup1(lg1);

    mx::RtConnectionIterator iter = lookgroup1.getLooks().getTargets();
    REQUIRE(!iter.isDone());
    mx::RtPrim lk1 = (*iter).asA<mx::RtPrim>();
    REQUIRE(lk1);
    REQUIRE(lk1.getName().str() == "defaultLook1");
    ++iter;
    REQUIRE(iter.isDone());

    mx::RtLook look1(lk1);
    iter = look1.getMaterialAssigns();
    REQUIRE(!iter.isDone());
    mx::RtPrim ma1 = (*iter).asA<mx::RtPrim>();
    REQUIRE(ma1);
    REQUIRE(ma1.getName().str() == "defaultMaterialAssign1");
    ++iter;
    REQUIRE(iter.isDone());

    mx::RtMaterialAssign materialassign1(ma1);
    iter = materialassign1.getMaterial();
    REQUIRE(!iter.isDone());
    mx::RtPrim mat1 = (*iter).asA<mx::RtPrim>();
    REQUIRE(mat1);
    REQUIRE(mat1.getName().str() == "defaultMaterial1");
    ++iter;
    REQUIRE(iter.isDone());
    iter = materialassign1.getCollection();
    REQUIRE(!iter.isDone());
    mx::RtPrim co1 = (*iter).asA<mx::RtPrim>();
    REQUIRE(co1);
    REQUIRE(co1.getName().str() == "defaultCollection1");
    ++iter;
    REQUIRE(iter.isDone());

    mx::RtNode material1(mat1);
    mx::RtInput surfInput = material1.getInput(SURFACESHADER);
    REQUIRE(surfInput.isConnected());
    mx::RtPrim sh1 = surfInput.getConnection().getParent();
    REQUIRE(sh1);
    REQUIRE(sh1.getName().str() == "defaultShader1");
}

TEST_CASE("Runtime: FileIo NodeGraph", "[runtime]")
{
    mx::RtScopedApiHandle api;

    // Load in stdlib
    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    api->setSearchPath(searchPath);
    api->loadLibrary(STDLIB);

    // Create a main stage
    mx::RtStagePtr stage = api->createStage(MAIN);

    // Create a nodegraph.
    mx::RtNodeGraph graph = stage->createPrim(mx::RtNodeGraph::typeName());
    graph.createInput(IN, mx::RtType::FLOAT);
    graph.createOutput(OUT, mx::RtType::FLOAT);
    mx::RtInput graphIn = graph.getInput(IN);
    mx::RtOutput graphOut = graph.getOutput(OUT);
    mx::RtOutput graphInSocket = graph.getInputSocket(IN);
    mx::RtInput graphOutSocket = graph.getOutputSocket(OUT);
    REQUIRE(graphIn);
    REQUIRE(graphOut);
    REQUIRE(graphInSocket);
    REQUIRE(graphOutSocket);

    // Add nodes to the graph.
    mx::RtNodeDef addNodeDef = api->getLibrary()->getPrimAtPath("/ND_add_float");
    mx::RtNode add1 = stage->createPrim(graph.getPath(), NONAME, addNodeDef.getName());
    mx::RtNode add2 = stage->createPrim(graph.getPath(), NONAME, addNodeDef.getName());
    graphInSocket.connect(add1.getInput(IN1));
    add1.getOutput(OUT).connect(add2.getInput(IN1));
    add2.getOutput(OUT).connect(graphOutSocket);

    // Add an unconnected node.
    stage->createPrim(graph.getPath(), NONAME, addNodeDef.getName());

    // Create a node on root level and connect it downstream after the graph.
    mx::RtNode add3 = stage->createPrim(addNodeDef.getName());
    graphOut.connect(add3.getInput(IN1));

    mx::RtWriteOptions options;
    options.writeIncludes = false;

    // Save the stage to file for inspection.
    const mx::FilePath filename = graph.getName().str() + "_export.mtlx";
    mx::RtFileIo fileIo(stage);
    fileIo.write(filename, &options);

    // Read the saved file to another stage
    mx::RtStagePtr anotherStage = api->createStage(mx::RtToken("another"));
    fileIo.setStage(anotherStage);
    fileIo.read(filename, searchPath);

    // Save the re-read stage to file for inspection.
    const mx::FilePath filename2 = graph.getName().str() + "_another_export.mtlx";
    fileIo.write(filename2, &options);

    // Compare file contents.
    REQUIRE(compareFiles(filename, filename2));
}

TEST_CASE("Runtime: Rename", "[runtime]")
{
    mx::RtScopedApiHandle api;

    // Load in stdlib
    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    api->setSearchPath(searchPath);
    api->loadLibrary(STDLIB);

    // Create a main stage
    mx::RtStagePtr stage = api->createStage(MAIN);

    // Create some nodes.
    const mx::RtToken nodedefName("ND_add_float");
    mx::RtNode node1 = stage->createPrim(nodedefName);
    mx::RtNode node2 = stage->createPrim(nodedefName);
    REQUIRE(node1);
    REQUIRE(node2);
    REQUIRE(node1.getName() == "add");
    REQUIRE(node2.getName() == "add1");

    // Test node renaming
    mx::RtToken newName1 = stage->renamePrim(node1.getPath(), mx::RtToken("foo"));
    mx::RtToken newName2 = stage->renamePrim(node2.getPath(), mx::RtToken("foo"));
    REQUIRE(node1.getName() == newName1);
    REQUIRE(node2.getName() == newName2);
    REQUIRE(node1.getName() == "foo");
    REQUIRE(node2.getName() == "foo1");

    stage->renamePrim(node1.getPath(), mx::RtToken("add"));
    REQUIRE(node1.getName() == "add");
    stage->renamePrim(node2.getPath(), mx::RtToken("foo"));
    REQUIRE(node2.getName() == "foo");
    stage->renamePrim(node2.getPath(), mx::RtToken("add1"));
    REQUIRE(node2.getName() == "add1");

    // Test that a rename to existing "add" results in the
    // old name "add1" since that is still a unique name.
    stage->renamePrim(node2.getPath(), mx::RtToken("add"));
    REQUIRE(node2.getName() == "add1");
}

TEST_CASE("Runtime: Stage References", "[runtime]")
{
    mx::RtScopedApiHandle api;

    // Load in stdlib
    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    api->setSearchPath(searchPath);
    api->loadLibrary(STDLIB);
    api->loadLibrary(PBRLIB);

    // Create a main stage
    mx::RtStagePtr stage = api->createStage(MAIN);

    // Test access and usage of contents from the library.
    mx::RtPrim nodedef = api->getLibrary()->getPrimAtPath("/ND_complex_ior");
    REQUIRE(nodedef.isValid());
    mx::RtNode node1 = stage->createPrim("/nodeA", nodedef.getName());
    REQUIRE(node1.isValid());

    // Add a reference to all the loaded libraries.
    stage->addReference(api->getLibrary());

    // Write the main stage to a new document,
    // writing only the non-referenced content.
    mx::RtWriteOptions options;
    options.writeIncludes = true;
    mx::RtFileIo mainFileIO(stage);
    mainFileIO.write("refs_as_includes.mtlx", &options);

    // Test removal of referenced content.
    REQUIRE_THROWS(stage->removePrim(nodedef.getPath()));

    // Test removal of non-referenced content.
    stage->removePrim(node1.getPath());
    REQUIRE(!node1.isValid());

    // Test renaming of referenced stage.
    mx::RtStagePtr fooStage = api->createStage(FOO);
    stage->addReference(fooStage);
    REQUIRE(stage->getReference(FOO) == fooStage);
    REQUIRE(fooStage->getName() == FOO);
    api->renameStage(FOO, BAR);
    REQUIRE(fooStage->getName() == BAR);
    REQUIRE(stage->getReference(BAR) == fooStage);
}

TEST_CASE("Runtime: Traversal", "[runtime]")
{
    mx::RtScopedApiHandle api;

    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    api->setSearchPath(searchPath);

    // Load in the standard libraries.
    api->loadLibrary(STDLIB);
    api->loadLibrary(PBRLIB);

    // Count elements traversing the loaded libraries.
    size_t nodeCount = 0, nodeDefCount = 0, nodeGraphCount = 0;
    for (mx::RtPrim prim : api->getLibrary()->traverse())
    {
        const mx::RtToken& typeName = prim.getTypeName();
        if (typeName == mx::RtNode::typeName())
        {
            nodeCount++;
        }
        else if (typeName == mx::RtNodeDef::typeName())
        {
            nodeDefCount++;
        }
        else if (typeName == mx::RtNodeGraph::typeName())
        {
            nodeGraphCount++;
        }
    }

    // Loading the same libraries into a MaterialX document
    // and tests the same element counts.
    mx::DocumentPtr doc = mx::createDocument();
    loadLibraries({ "stdlib", "pbrlib" }, searchPath, doc);
    const size_t libNodeCount = doc->getNodes().size();
    const size_t libNodeDefCount = doc->getNodeDefs().size();
    const size_t libNodeGraphCount = doc->getNodeGraphs().size();
    REQUIRE(nodeCount == libNodeCount);
    REQUIRE(nodeDefCount == libNodeDefCount);
    REQUIRE(nodeGraphCount == libNodeGraphCount);

    // Create a main stage
    mx::RtStagePtr stage = api->createStage(MAIN);

    const mx::RtToken nodedefName("ND_subtract_vector3");
    mx::RtObject nodeObj = stage->createPrim(nodedefName);
    REQUIRE(nodeObj);

    // Traverse using a filter to return only node objects.
    mx::RtSchemaPredicate<mx::RtNode> nodeFilter;
    nodeCount = 0, nodeDefCount = 0, nodeGraphCount = 0;
    for (auto it = stage->traverse(nodeFilter); !it.isDone(); ++it)
    {
        const mx::RtToken& typeName = (*it).getTypeName();
        if (typeName == mx::RtNode::typeName())
        {
            nodeCount++;
        }
        else if (typeName == mx::RtNodeDef::typeName())
        {
            nodeDefCount++;
        }
        else if (typeName == mx::RtNodeGraph::typeName())
        {
            nodeGraphCount++;
        }
    }
    REQUIRE(nodeCount == 1);
    REQUIRE(nodeDefCount == 0);
    REQUIRE(nodeGraphCount == 0);

    // Traverse nodes on a nodegraph
    mx::RtNodeGraph nodegraph = api->getLibrary()->getPrimAtPath("/NG_tiledimage_float");
    REQUIRE(nodegraph);
    nodeCount = 0;
    for (auto it = nodegraph.getNodes(); !it.isDone(); ++it)
    {
        nodeCount++;
    }
    REQUIRE(nodeCount == 5);

    // Filter for finding input attributes.
    mx::RtObjTypePredicate<mx::RtInput> inputFilter;

    // Travers a nodedef finding all its inputs.
    mx::RtNodeDef generalized_schlick_brdf = api->getLibrary()->getPrimAtPath("/ND_generalized_schlick_brdf");
    REQUIRE(generalized_schlick_brdf);
    size_t inputCount = 0;
    for (auto it = generalized_schlick_brdf.getPrim().getAttributes(inputFilter); !it.isDone(); ++it)
    {
        inputCount++;
    }
    REQUIRE(inputCount == 9);

    // Filter for finding nodedefs of BSDF nodes.
    auto bsdfFilter = [](const mx::RtObject& obj) -> bool
    {
        mx::RtNodeDef nodedef(obj.asA<mx::RtPrim>());
        if (nodedef.isValid())
        {
            mx::RtOutput out = nodedef.getOutput(OUT);
            return (out && out.getType() == mx::RtType::BSDF);
        }
        return false;
    };

    // Travers to find all nodedefs for BSDF nodes.
    size_t bsdfCount = 0;
    for (auto it = api->getLibrary()->traverse(bsdfFilter); !it.isDone(); ++it)
    {
        bsdfCount++;
    }
    size_t libBsdfCount = 0;
    for (auto it : doc->getNodeDefs())
    {
        if (it->getOutputCount() == 1 && it->getOutput(OUT.str())->getType() == mx::RtType::BSDF.str())
        {
            libBsdfCount++;
        }
    }
    REQUIRE(bsdfCount == libBsdfCount);
}

TEST_CASE("Runtime: Looks", "[runtime]")
{
    mx::RtScopedApiHandle api;

    mx::RtStagePtr stage = api->createStage(ROOT);

    //
    // Test collections
    //
    mx::RtPrim p1 = stage->createPrim("collection1", mx::RtCollection::typeName());
    mx::RtCollection col1(p1);
    mx::RtAttribute igeom = col1.getIncludeGeom();
    igeom.setValueString("foo");
    mx::RtAttribute egeom = col1.getExcludeGeom();
    egeom.setValueString("bar");
    REQUIRE(igeom.getValueString() == "foo");
    REQUIRE(egeom.getValueString() == "bar");

    mx::RtPrim p2 = stage->createPrim("child1", mx::RtCollection::typeName());
    mx::RtPrim p3 = stage->createPrim("child2", mx::RtCollection::typeName());
    col1.addCollection(p2);
    col1.addCollection(p3);
    mx::RtRelationship rel = col1.getIncludeCollection();
    REQUIRE(rel.targetCount() == 2);
    col1.removeCollection(p3);
    REQUIRE(rel.targetCount() == 1);
    col1.addCollection(p3);

    //
    // Test materialassign
    //
    mx::RtPrim pa = stage->createPrim("matassign1", mx::RtMaterialAssign::typeName());
    mx::RtMaterialAssign assign1(pa);
    assign1.getCollection().addTarget(p1);
    mx::RtConnectionIterator iter = assign1.getCollection().getTargets();
    while (iter.isDone())
    {
        REQUIRE((*iter).getName() == "collection1");
        break;
    }

    // Load in library so we can create a material
    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    api->setSearchPath(searchPath);
    api->loadLibrary(STDLIB);
    api->loadLibrary(PBRLIB);
    const mx::RtToken matDef("ND_surfacematerial");
    mx::RtPrim sm1 = stage->createPrim(mx::RtPath("/surfacematerial1"), matDef);
    assign1.getMaterial().addTarget(sm1);
    mx::RtConnectionIterator iter2 = assign1.getMaterial().getTargets();
    while (!iter2.isDone())
    {
        REQUIRE((*iter2).getName() == "surfacematerial1");
        break;
    }

    assign1.getExclusive().setValue(mx::RtValue(true));
    REQUIRE(assign1.getExclusive().getValue().asBool() == true);

    assign1.getGeom().setValueString("/mygeom");
    REQUIRE(assign1.getGeom().getValueString() == "/mygeom");

    //
    // Test look
    //
    mx::RtPrim lo1 = stage->createPrim("look1", mx::RtLook::typeName());
    mx::RtLook look1(lo1);

    mx::RtPrim pa2 = stage->createPrim("matassign2", mx::RtMaterialAssign::typeName());
    mx::RtMaterialAssign assign2(pa2);
    assign2.getCollection().addTarget(p2);
    mx::RtPrim sm2 = stage->createPrim(mx::RtPath("/surfacematerial2"), matDef);
    assign2.getMaterial().addTarget(sm2);
    assign2.getExclusive().getValue().asBool() = false;
    look1.addMaterialAssign(pa);
    look1.addMaterialAssign(pa2);
    REQUIRE_THROWS(look1.addMaterialAssign(col1.getPrim()));

    mx::RtConnectionIterator iter3 = look1.getMaterialAssigns().getTargets();
    REQUIRE(look1.getMaterialAssigns().targetCount() == 2);
    while (!iter3.isDone())
    {
        REQUIRE((*iter3).getName() == "matassign1");
        break;
    }
    look1.removeMaterialAssign(pa2);
    REQUIRE(look1.getMaterialAssigns().targetCount() == 1);
    look1.addMaterialAssign(pa2);

    mx::RtPrim lo2 = stage->createPrim("look2", mx::RtLook::typeName());
    mx::RtLook look2(lo2);
    look2.getInherit().addTarget(lo1);
    REQUIRE(look2.getInherit().targetCount() == 1);

    //
    // Test lookgroup
    //
    mx::RtPrim lg1 = stage->createPrim("lookgroup1", mx::RtLookGroup::typeName());
    mx::RtLookGroup lookgroup1(lg1);
    lookgroup1.addLook(lo1);
    lookgroup1.addLook(lo2);
    REQUIRE(lookgroup1.getLooks().targetCount() == 2);
    lookgroup1.removeLook(lo1);
    REQUIRE(lookgroup1.getLooks().targetCount() == 1);

    lookgroup1.getActiveLook().setValueString("look1");
    REQUIRE(lookgroup1.getActiveLook().getValueString() == "look1");

    lookgroup1.addLook(lo1);

    // Test file I/O
    bool useOptions = false;
    for (int i = 0; i < 2; ++i) {

        mx::RtWriteOptions writeOptions;
        writeOptions.materialWriteOp = mx::RtWriteOptions::MaterialWriteOp::WRITE_LOOKS;
        mx::RtReadOptions readOptions;
        readOptions.readLookInformation = true;
        // Do not upgrade on reload:
        mx::DocumentPtr doc = mx::createDocument();
        readOptions.applyFutureUpdates = false;

        mx::RtFileIo stageIo(stage);
        stageIo.write("rtLookExport.mtlx", useOptions ? &writeOptions : nullptr);
        std::stringstream stream1;
        stageIo.write(stream1, useOptions ? &writeOptions : nullptr);

        mx::RtStagePtr stage2 = api->createStage(ROOT);
        mx::RtFileIo stageIo2(stage2);
        stageIo2.read("rtLookExport.mtlx", mx::FileSearchPath(), useOptions ? &readOptions : nullptr);
        stageIo2.write("rtLookExport_2.mtlx", useOptions ? &writeOptions : nullptr);
        std::stringstream stream2;
        stageIo2.write(stream2, useOptions ? &writeOptions : nullptr);
        REQUIRE(stream1.str() == stream2.str());

        // Make sure we get an identical runtime after reloading:

        //
        // Check collections
        //
        p1 = stage2->getPrimAtPath("/collection1");
        REQUIRE(p1);
        REQUIRE(p1.getTypeInfo()->getShortTypeName() == mx::RtCollection::typeName());
        col1 = p1;
        REQUIRE(col1.getIncludeGeom().getValueString() == "foo");
        REQUIRE(col1.getExcludeGeom().getValueString() == "bar");

        p2 = stage2->getPrimAtPath("/child1");
        REQUIRE(p2);
        REQUIRE(p2.getTypeInfo()->getShortTypeName() == mx::RtCollection::typeName());
        p3 = stage2->getPrimAtPath("/child2");
        REQUIRE(p3);
        REQUIRE(p3.getTypeInfo()->getShortTypeName() == mx::RtCollection::typeName());
        rel = col1.getIncludeCollection();
        REQUIRE(rel.targetCount() == 2);
        iter = rel.getTargets();
        REQUIRE(!iter.isDone());
        REQUIRE(*iter == p2);
        ++iter;
        REQUIRE(!iter.isDone());
        REQUIRE(*iter == p3);
        ++iter;
        REQUIRE(iter.isDone());

        //
        // Check materialassign
        //
        pa = stage2->getPrimAtPath("/matassign1");
        REQUIRE(pa);
        REQUIRE(pa.getTypeInfo()->getShortTypeName() == mx::RtMaterialAssign::typeName());
        assign1 = pa;
        REQUIRE(assign1);
        iter = assign1.getCollection().getTargets();
        REQUIRE(!iter.isDone());
        REQUIRE((*iter) == p1);
        ++iter;
        REQUIRE(iter.isDone());

        // Check material
        sm1 = stage2->getPrimAtPath("/surfacematerial1");
        REQUIRE(sm1);
        iter2 = assign1.getMaterial().getTargets();
        REQUIRE(!iter2.isDone());
        REQUIRE((*iter2) == sm1);
        ++iter2;
        REQUIRE(iter2.isDone());
        REQUIRE(assign1.getExclusive().getValue().asBool() == true);
        REQUIRE(assign1.getGeom().getValueString() == "/mygeom");

        //
        // Check look
        //
        lo1 = stage2->getPrimAtPath("/look1");
        REQUIRE(lo1);
        REQUIRE(lo1.getTypeInfo()->getShortTypeName() == mx::RtLook::typeName());
        look1 = lo1;
        REQUIRE(look1);

        pa2 = stage2->getPrimAtPath("matassign2");
        REQUIRE(pa2);
        REQUIRE(pa2.getTypeInfo()->getShortTypeName() == mx::RtMaterialAssign::typeName());
        assign2 = pa2;
        REQUIRE(assign2);
        iter = assign2.getCollection().getTargets();
        REQUIRE(!iter.isDone());
        REQUIRE((*iter) == p2);
        ++iter;
        REQUIRE(iter.isDone());
        sm2 = stage2->getPrimAtPath("/surfacematerial2");
        iter = assign2.getMaterial().getTargets();
        REQUIRE(!iter.isDone());
        REQUIRE((*iter) == sm2);
        ++iter;
        REQUIRE(iter.isDone());
        REQUIRE(assign2.getExclusive().getValue().asBool() == false);
        iter = look1.getMaterialAssigns().getTargets();
        REQUIRE(!iter.isDone());
        REQUIRE((*iter) == pa);
        ++iter;
        REQUIRE(!iter.isDone());
        REQUIRE((*iter) == pa2);
        ++iter;
        REQUIRE(iter.isDone());

        lo2 = stage2->getPrimAtPath("/look2");
        REQUIRE(lo2);
        REQUIRE(lo2.getTypeInfo()->getShortTypeName() == mx::RtLook::typeName());
        look2 = lo2;
        REQUIRE(look2);
        iter = look2.getInherit().getTargets();
        REQUIRE(!iter.isDone());
        REQUIRE((*iter) == lo1);
        ++iter;
        REQUIRE(iter.isDone());

        //
        // Check lookgroup
        //
        lg1 = stage2->getPrimAtPath("/lookgroup1");
        REQUIRE(lg1);
        REQUIRE(lg1.getTypeInfo()->getShortTypeName() == mx::RtLookGroup::typeName());
        lookgroup1 = lg1;
        REQUIRE(lookgroup1);

        iter = lookgroup1.getLooks().getTargets();
        REQUIRE(!iter.isDone());
        REQUIRE((*iter) == lo2);
        ++iter;
        REQUIRE(!iter.isDone());
        REQUIRE((*iter) == lo1);
        ++iter;
        REQUIRE(iter.isDone());
        REQUIRE(lookgroup1.getActiveLook().getValueString() == "look1");

        // Try again, with options.
        useOptions = true;
    }

    // Look group relations
    mx::RtPrim lg2 = stage->createPrim("parent_lookgroup", mx::RtLookGroup::typeName());
    mx::RtLookGroup lookgroup2(lg2);
    mx::RtPrim lg3 = stage->createPrim("child_lookgroup", mx::RtLookGroup::typeName());
    mx::RtLookGroup lookgroup3(lg3);
    lookgroup2.addLook(lg3);
    lookgroup2.addLook(lo2);
    REQUIRE_THROWS(lookgroup2.addLook(assign2.getPrim()));

    iter = lookgroup2.getLooks().getTargets();
    REQUIRE(!iter.isDone());
    REQUIRE((*iter) == lg3);
    ++iter;
    REQUIRE(!iter.isDone());
    REQUIRE((*iter) == lo2);
    lookgroup2.getActiveLook().setValueString("child_lookgroup");
    REQUIRE(lookgroup2.getActiveLook().getValueString() == "child_lookgroup");
}

TEST_CASE("Runtime: FileIo downgrade", "[runtime]")
{
    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    {
        mx::RtScopedApiHandle api;

        // Load in stdlib
        api->setSearchPath(searchPath);
        api->loadLibrary(STDLIB);
        api->loadLibrary(PBRLIB);
        api->loadLibrary(BXDFLIB);

        // Create a stage.
        mx::RtStagePtr stage = api->createStage(MAIN);

        // Create a new graph
        mx::RtLook lk1 = stage->createPrim("lk1", mx::RtLook::typeName());

        mx::RtPrim materialassign1 = stage->createPrim("ma1", mx::RtMaterialAssign::typeName());
        mx::RtMaterialAssign ma1(materialassign1);
        lk1.addMaterialAssign(materialassign1);

        ma1.getCollection().addTarget(stage->createPrim("co1", mx::RtCollection::typeName()));
        const mx::RtToken smName("ND_surfacematerial");
        const mx::RtToken smSurface("surfaceshader");
        mx::RtPrim sm1 = stage->createPrim(mx::RtPath("/sm1"), smName);
        ma1.getMaterial().addTarget(sm1);

        const mx::RtToken ssName("ND_standard_surface_surfaceshader");
        mx::RtPrim ss1 = stage->createPrim(mx::RtPath("/ss1"), ssName);
        ss1.getOutput(OUT).connect(sm1.getInput(smSurface));
        const mx::RtToken ssBase("base");
        const mx::RtToken ssEmission("emission");
        const mx::RtToken ssBaseColor("base_color");
        const mx::RtToken ssSheenColor("sheen_color");
        const mx::RtToken ssCoatColor("coat_color");
        const mx::RtToken ssEmissionColor("emission_color");

        ss1.getAttribute(ssBase).getValue().asFloat() = 0.5;
        ss1.getAttribute(ssEmission).getValue().asFloat() = 0.5;

        // Create a node and connect it twice to the Standard Surface.
        mx::RtNode ml1 = stage->createPrim("ml1", mx::RtToken("ND_multiply_color3"));
        mx::RtOutput ml1Out = ml1.getOutput(OUT);
        ml1Out.connect(ss1.getInput(ssBaseColor));
        ml1Out.connect(ss1.getInput(ssEmissionColor));

        // Create an empty nodegraph and connect it twice to the Standard Surface
        mx::RtNodeGraph ng1 = stage->createPrim("ng1", mx::RtNodeGraph::typeName());
        mx::RtOutput ng1Out = ng1.createOutput(OUT, mx::RtType::COLOR3);
        ng1Out.connect(ss1.getInput(ssSheenColor));
        ng1Out.connect(ss1.getInput(ssCoatColor));

        // Save and downgrade to v1.37:
        mx::RtFileIo stageIo(stage);
        mx::RtWriteOptions wops;
        wops.writeIncludes = false;
        wops.materialWriteOp =
            mx::RtWriteOptions::MaterialWriteOp::WRITE_MATERIALS_AS_ELEMENTS |
            mx::RtWriteOptions::MaterialWriteOp::WRITE_LOOKS;

        std::stringstream stream;
        stageIo.write(stream, &wops);

        // What are we looking for:
        //
        // The 1.37 syntax for <bindinput> elements allows two and only two
        // combinations:
        //
        //  <bindinput nodegraph="foo" [output="bar"] />
        //     for connecting to the output of a nodegraph and
        //  <bindinput output="baz" />
        //     for connecting to a raw <output> element.
        //
        // You will notice the absence of an option to connect to the output of
        // a <node> element since there is no "nodename" attribute on a
        // <bindinput>.
        //
        // So we expect the two connections to the freestanding <multiply> node to
        // be done via an intermediate <output> element added to the graph,
        // while the connection to the "ng1" <nodegraph> should be done natively.
        //
        // <?xml version="1.0"?>
        // <materialx version="1.37">
        //   <multiply name="ml1" type="color3" />
        //   <nodegraph name="ng1">
        //     <output name="out" type="color3" />
        //   </nodegraph>
        //   <collection name="co1" excludegeom="" includegeom="" includecollection="" />
        //   <look name="lk1" inherit="">
        //     <materialassign name="ma1" exclusive="true" collection="co1" material="sm1" geom="" />
        //   </look>
        //   <material name="sm1">
        //     <shaderref name="ss1" node="standard_surface">
        //       <bindinput name="base" type="float" value="0.5" />
        //       <bindinput name="base_color" type="color3" output="OUT_ml1_out" />
        //       <bindinput name="sheen_color" type="color3" nodegraph="ng1" />
        //       <bindinput name="coat_color" type="color3" nodegraph="ng1" />
        //       <bindinput name="emission" type="float" value="0.5" />
        //       <bindinput name="emission_color" type="color3" output="OUT_ml1_out" />
        //     </shaderref>
        //   </material>
        //   <output name="OUT_ml1_out" type="color3" nodename="ml1" output="out" />
        // </materialx>
        mx::DocumentPtr doc = mx::createDocument();
        mx::XmlReadOptions readOptions;
        // Last version with material and shaderref:
        readOptions.applyFutureUpdates = false;
        mx::readFromXmlString(doc, stream.str(), &readOptions);

        auto xmlMat = doc->getMaterial("sm1");
        REQUIRE(xmlMat);

        auto xmlSR = xmlMat->getShaderRef("ss1");
        REQUIRE(xmlSR);

        auto xmlBI = xmlSR->getBindInput(ssBase);
        REQUIRE(xmlBI);
        REQUIRE(xmlBI->getAttribute(mx::ValueElement::VALUE_ATTRIBUTE) == "0.5");
        REQUIRE(!xmlBI->hasAttribute(mx::PortElement::NODE_GRAPH_ATTRIBUTE));
        REQUIRE(!xmlBI->hasAttribute(mx::PortElement::OUTPUT_ATTRIBUTE));

        xmlBI = xmlSR->getBindInput(ssEmission);
        REQUIRE(xmlBI);
        REQUIRE(xmlBI->getAttribute(mx::ValueElement::VALUE_ATTRIBUTE) == "0.5");
        REQUIRE(!xmlBI->hasAttribute(mx::PortElement::NODE_GRAPH_ATTRIBUTE));
        REQUIRE(!xmlBI->hasAttribute(mx::PortElement::OUTPUT_ATTRIBUTE));

        xmlBI = xmlSR->getBindInput(ssBaseColor);
        REQUIRE(xmlBI);
        REQUIRE(!xmlBI->hasAttribute(mx::ValueElement::VALUE_ATTRIBUTE));
        REQUIRE(!xmlBI->hasAttribute(mx::PortElement::NODE_GRAPH_ATTRIBUTE));
        auto const& outputName = xmlBI->getAttribute(mx::PortElement::OUTPUT_ATTRIBUTE);
        auto xmlOUT = doc->getOutput(outputName);
        REQUIRE(xmlOUT);
        REQUIRE(xmlOUT->getAttribute(mx::PortElement::NODE_NAME_ATTRIBUTE) == "ml1");

        xmlBI = xmlSR->getBindInput(ssEmissionColor);
        REQUIRE(xmlBI);
        REQUIRE(!xmlBI->hasAttribute(mx::ValueElement::VALUE_ATTRIBUTE));
        REQUIRE(!xmlBI->hasAttribute(mx::PortElement::NODE_GRAPH_ATTRIBUTE));
        REQUIRE(xmlBI->getAttribute(mx::PortElement::OUTPUT_ATTRIBUTE) == outputName);

        xmlBI = xmlSR->getBindInput(ssSheenColor);
        REQUIRE(xmlBI);
        REQUIRE(!xmlBI->hasAttribute(mx::ValueElement::VALUE_ATTRIBUTE));
        REQUIRE(xmlBI->getAttribute(mx::PortElement::NODE_GRAPH_ATTRIBUTE) == "ng1");

        xmlBI = xmlSR->getBindInput(ssCoatColor);
        REQUIRE(xmlBI);
        REQUIRE(!xmlBI->hasAttribute(mx::ValueElement::VALUE_ATTRIBUTE));
        REQUIRE(xmlBI->getAttribute(mx::PortElement::NODE_GRAPH_ATTRIBUTE) == "ng1");
    }
}

mx::RtToken toTestResolver(const mx::RtToken& str, const mx::RtToken& type)
{
    mx::StringResolverPtr resolver = mx::StringResolver::create();
    mx::RtToken resolvedName = mx::RtToken(resolver->resolve(str, type).c_str());
    resolvedName = resolvedName.str() + "_toTestResolver";
    return resolvedName;
}

mx::RtToken fromTestResolver(const mx::RtToken& str, const mx::RtToken& type)
{
    mx::StringResolverPtr resolver = mx::StringResolver::create();
    mx::RtToken resolvedName = mx::RtToken(resolver->resolve(str, type).c_str());
    resolvedName = resolvedName.str() + "_fromTestResolver";
    return resolvedName;
}

TEST_CASE("Runtime: NameResolvers", "[runtime]")
{
    mx::RtNameResolverRegistryPtr registry = mx::RtNameResolverRegistry::createNew();
    REQUIRE(registry);

    mx::RtNameResolverInfo geomInfo;
    geomInfo.identifier = mx::RtToken("geom_resolver");
    geomInfo.elementType = mx::RtNameResolverInfo::GEOMNAME_TYPE;
    geomInfo.toFunction = nullptr;
    geomInfo.fromFunction = nullptr;
    mx::RtToken pipe("|");
    mx::RtToken slash("/");
    geomInfo.toSubstitutions.emplace(pipe, slash);
    geomInfo.fromSubstitutions.emplace(slash, pipe);
    registry->registerNameResolvers(geomInfo);

    mx::RtNameResolverInfo imageInfo;
    imageInfo.identifier = mx::RtToken("image_resolver");
    imageInfo.elementType = mx::RtNameResolverInfo::FILENAME_TYPE;
    imageInfo.toFunction = toTestResolver;
    imageInfo.fromFunction = fromTestResolver;
    registry->registerNameResolvers(imageInfo);
    
    mx::RtToken mayaPathToGeom("|path|to|geom");
    mx::RtToken mxPathToGeom("/path/to/geom");
    mx::RtToken result1 = registry->resolveIdentifier(mayaPathToGeom, mx::RtNameResolverInfo::GEOMNAME_TYPE, true);
    REQUIRE(result1.str() == mxPathToGeom.str());
    mx::RtToken result2 = registry->resolveIdentifier(result1, mx::RtNameResolverInfo::GEOMNAME_TYPE, false);
    REQUIRE(result2.str() == mayaPathToGeom.str());
    
    mx::RtToken pathToGeom("test");
    mx::RtToken result3 = registry->resolveIdentifier(pathToGeom, mx::RtNameResolverInfo::FILENAME_TYPE, true);
    REQUIRE(result3.str() == "test_toTestResolver");
    mx::RtToken result4 = registry->resolveIdentifier(pathToGeom, mx::RtNameResolverInfo::FILENAME_TYPE, false);
    REQUIRE(result4.str() == "test_fromTestResolver");
}

TEST_CASE("Runtime: libraries", "[runtime]")
{
    mx::RtScopedApiHandle api;

    // Load in all libraries required for materials
    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    api->setSearchPath(searchPath);
    api->loadLibrary(STDLIB);
    api->loadLibrary(PBRLIB);
    api->loadLibrary(BXDFLIB);

    // Set and test search paths
    api->clearSearchPath();
    REQUIRE(api->getSearchPath().isEmpty());
    api->setSearchPath(searchPath);
    REQUIRE(api->getSearchPath().asString() == searchPath.asString());

    REQUIRE(api->getTextureSearchPath().isEmpty());
    mx::FileSearchPath texturePath(mx::FilePath::getCurrentPath() / mx::FilePath("resources/Images"));
    api->setTextureSearchPath(texturePath);
    REQUIRE(api->getTextureSearchPath().find("brass_color.jpg").exists());

    REQUIRE(api->getImplementationSearchPath().isEmpty());
    mx::FileSearchPath implPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/stdlib/genglsl"));
    api->setImplementationSearchPath(implPath);
    REQUIRE(api->getImplementationSearchPath().find("stdlib_genglsl_unit_impl.mtlx").exists());    
}

TEST_CASE("Runtime: units", "[runtime]")
{
    mx::RtScopedApiHandle api;

    // Load in all libraries required for materials
    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    api->setSearchPath(searchPath);
    api->loadLibrary(STDLIB);
    // Load in stdlib twice on purpose to ensure no exception is thrown when trying to add a duplicate unit 
    // definition 
    api->loadLibrary(STDLIB);
    api->loadLibrary(PBRLIB);
    api->loadLibrary(BXDFLIB);

    // Read in test document with units
    mx::FileSearchPath testSearchPath(mx::FilePath::getCurrentPath() /
        "resources" /
        "Materials" /
        "TestSuite" /
        "stdlib" /
        "units");
    mx::StringVec tests{ "distance_units.mtlx",
                         "image_unit.mtlx",
                         "standard_surface_unit.mtlx",
                         "texture_units.mtlx",
                         "tiledimage_unit.mtlx" };
    mx::RtReadOptions options;
    options.applyFutureUpdates = true;
    for (auto test : tests)
    {
        mx::RtStagePtr stage = api->createStage(mx::RtToken("stage: " + test));
        mx::RtFileIo fileIo(stage);

        // Test read will take into account units read in via library load
        fileIo.read(test, testSearchPath, &options);

        // Test that read and write of files with units works.
        std::stringstream inStream;
        mx::DocumentPtr inDoc = mx::createDocument();
        mx::XmlReadOptions readOptions;
        readOptions.applyFutureUpdates = true;
        mx::readFromXmlFile(inDoc, test, testSearchPath, &readOptions);

        mx::DocumentPtr outDoc = mx::createDocument();
        std::stringstream outStream;
        fileIo.write(outStream);
        mx::readFromXmlStream(outDoc, outStream);

        for (mx::ElementPtr elem : inDoc->traverseTree())
        {
            mx::ValueElementPtr val = elem->asA<mx::ValueElement>();
            if (val)
            {
                const std::string& unit = val->getUnit();
                const std::string& unitType = val->getUnitType();
                if (!unit.empty() || !unitType.empty())
                {
                    const std::string& path = val->getNamePath();
                    mx::ElementPtr outElem = outDoc->getDescendant(path);
                    mx::ValueElementPtr outVal = outElem->asA<mx::ValueElement>();
                    if (outVal)
                    {
                        REQUIRE((outVal->getUnit() == unit && outVal->getUnitType() == unitType));
                    }
                }
            }
        }
    }
}

TEST_CASE("Runtime: Commands", "[runtime]")
{
    mx::RtScopedApiHandle api;

    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    api->setSearchPath(searchPath);
    api->loadLibrary(STDLIB);

    mx::RtStagePtr stage = api->createStage(MAIN);

    //
    // Register callbacks for tracking data model changes
    //
    auto createPrimCB = [](const mx::RtStagePtr&, const mx::RtPrim&, void* userData)
    {
        ++(*reinterpret_cast<size_t*>(userData));
    };
    auto removePrimCB = [](const mx::RtStagePtr&, const mx::RtPrim&, void* userData)
    {
        ++(*reinterpret_cast<size_t*>(userData));
    };
    auto renamePrimCB = [](const mx::RtStagePtr&, const mx::RtPrim&, const mx::RtToken&, void* userData)
    {
        ++(*reinterpret_cast<size_t*>(userData));
    };
    auto reparentPrimCB = [](const mx::RtStagePtr&, const mx::RtPrim&, const mx::RtPath&, void* userData)
    {
        ++(*reinterpret_cast<size_t*>(userData));
    };
    auto setAttrCB = [](const mx::RtAttribute&, const mx::RtValue&, void* userData)
    {
        ++(*reinterpret_cast<size_t*>(userData));
    };
    auto makeConnectionCB = [](const mx::RtOutput&, const mx::RtInput&, void* userData)
    {
        ++(*reinterpret_cast<size_t*>(userData));
    };
    auto breakConnectionCB = [](const mx::RtOutput&, const mx::RtInput&, void* userData)
    {
        ++(*reinterpret_cast<size_t*>(userData));
    };

    size_t createPrimCount = 0;
    size_t removePrimCount = 0;
    size_t renamePrimCount = 0;
    size_t reparentPrimCount = 0;
    size_t setAttrCount = 0;
    size_t makeConnectionCount = 0;
    size_t breakConnectionCount = 0;

    mx::RtCallbackId createPrimCB_id = mx::RtMessage::addCreatePrimCallback(createPrimCB, &createPrimCount);
    mx::RtCallbackId removePrimCB_id = mx::RtMessage::addRemovePrimCallback(removePrimCB, &removePrimCount);
    mx::RtCallbackId renamePrimCB_id = mx::RtMessage::addRenamePrimCallback(renamePrimCB, &renamePrimCount);
    mx::RtCallbackId reparentPrimCB_id = mx::RtMessage::addReparentPrimCallback(reparentPrimCB, &reparentPrimCount);
    mx::RtCallbackId setAttrCB_id = mx::RtMessage::addSetAttributeCallback(setAttrCB, &setAttrCount);
    mx::RtCallbackId makeConnectionCB_id = mx::RtMessage::addMakeConnectionCallback(makeConnectionCB, &makeConnectionCount);
    mx::RtCallbackId breakConnectionCB_id = mx::RtMessage::addBreakConnectionCallback(breakConnectionCB, &breakConnectionCount);

    mx::RtCommandResult result;

    //
    // Test prim creation
    //

    createPrimCount = 0;
    removePrimCount = 0;

    mx::RtCommand::createPrim(stage, mx::RtNodeGraph::typeName(), result);
    REQUIRE(result);
    REQUIRE(result.getObject().isA<mx::RtPrim>());
    mx::RtPrim graph1(result.getObject().asA<mx::RtPrim>());
    mx::RtNodeGraph ng1(graph1);
    ng1.createInput(IN1, mx::RtType::FLOAT);
    ng1.createInput(IN2, mx::RtType::FLOAT);
    ng1.createOutput(OUT, mx::RtType::FLOAT);

    const mx::RtToken addFloatNode("ND_add_float");

    mx::RtCommand::createPrim(stage, addFloatNode, graph1.getPath(), mx::EMPTY_TOKEN, result);
    REQUIRE(result);
    REQUIRE(result.getObject().isA<mx::RtPrim>());
    mx::RtPrim add1(result.getObject().asA<mx::RtPrim>());

    mx::RtCommand::createPrim(stage, addFloatNode, graph1.getPath(), mx::EMPTY_TOKEN, result);
    REQUIRE(result);
    REQUIRE(result.getObject().isA<mx::RtPrim>());
    mx::RtPrim add2(result.getObject().asA<mx::RtPrim>());

    mx::RtPath nodePath(graph1.getPath());
    nodePath.push(FOO);
    mx::RtCommand::createPrim(stage, addFloatNode, nodePath, result);
    REQUIRE(result);
    REQUIRE(result.getObject().isA<mx::RtPrim>());
    mx::RtPrim foo(result.getObject().asA<mx::RtPrim>());
    REQUIRE(foo.getPath() == nodePath);

    mx::RtCommand::undo(result);
    REQUIRE(result);
    REQUIRE(!foo.isValid());
    mx::RtCommand::redo(result);
    REQUIRE(result);
    REQUIRE(foo.isValid());

    REQUIRE(createPrimCount == 5);
    REQUIRE(removePrimCount == 1);

    //
    // Test rename command
    //

    renamePrimCount = 0;

    mx::RtCommand::renamePrim(stage, foo.getPath(), BAR, result);
    REQUIRE(result);
    REQUIRE(foo.getName() == BAR);

    mx::RtCommand::undo(result);
    REQUIRE(result);
    REQUIRE(foo.getName() == FOO);
    mx::RtCommand::redo(result);
    REQUIRE(result);
    REQUIRE(foo.getName() == BAR);

    REQUIRE(renamePrimCount == 3);

    //
    // Test setting attribute values
    //
    setAttrCount = 0;

    mx::RtAttribute in1 = add2.getAttribute(IN1);
    mx::RtAttribute in2 = add2.getAttribute(IN2);
    in1.getValue().asFloat() = 1.0f;
    in2.getValue().asFloat() = 1.0f;

    mx::RtCommand::setAttribute(in1, 3.0f, result);
    REQUIRE(result);
    REQUIRE(in1.getValue().asFloat() == 3.0f);

    mx::RtCommand::setAttribute(in2, 7.0f, result);
    REQUIRE(result);
    REQUIRE(in2.getValue().asFloat() == 7.0f);

    mx::RtCommand::undo(result);
    REQUIRE(result);
    REQUIRE(in1.getValue().asFloat() == 3.0f);
    REQUIRE(in2.getValue().asFloat() == 1.0f);

    mx::RtCommand::undo(result);
    REQUIRE(result);
    REQUIRE(in1.getValue().asFloat() == 1.0f);
    REQUIRE(in2.getValue().asFloat() == 1.0f);

    mx::RtCommand::redo(result);
    REQUIRE(result);
    REQUIRE(in1.getValue().asFloat() == 3.0f);
    REQUIRE(in2.getValue().asFloat() == 1.0f);

    mx::RtCommand::redo(result);
    REQUIRE(result);
    REQUIRE(in1.getValue().asFloat() == 3.0f);
    REQUIRE(in2.getValue().asFloat() == 7.0f);

    REQUIRE(setAttrCount == 6);

    //
    // Test making and breaking connections
    //
    makeConnectionCount = 0;
    breakConnectionCount = 0;

    mx::RtCommand::makeConnection(add1.getOutput(OUT), add2.getInput(IN1), result);
    REQUIRE(result);
    REQUIRE(add2.getInput(IN1).getConnection() == add1.getOutput(OUT));

    mx::RtCommand::undo(result);
    REQUIRE(result);
    REQUIRE(add2.getInput(IN1).getConnection() != add1.getOutput(OUT));

    mx::RtCommand::redo(result);
    REQUIRE(result);
    REQUIRE(add2.getInput(IN1).getConnection() == add1.getOutput(OUT));

    mx::RtCommand::makeConnection(ng1.getInputSocket(IN1), add1.getInput(IN1), result);
    REQUIRE(result);
    mx::RtCommand::makeConnection(ng1.getInputSocket(IN2), add1.getInput(IN2), result);
    REQUIRE(result);
    mx::RtCommand::makeConnection(add2.getOutput(OUT), ng1.getOutputSocket(OUT), result);
    REQUIRE(result);
    mx::RtCommand::breakConnection(add2.getOutput(OUT), ng1.getOutputSocket(OUT), result);
    REQUIRE(result);
    mx::RtCommand::undo(result);
    REQUIRE(result);

    REQUIRE(makeConnectionCount == 6);
    REQUIRE(breakConnectionCount == 2);

    //
    // Test prim copy
    //

    createPrimCount = 0;
    removePrimCount = 0;

    mx::RtCommand::copyPrim(stage, add1, add1.getParent().getPath(), result);
    REQUIRE(result);
    REQUIRE(result.getObject().isA<mx::RtPrim>());
    mx::RtPrim add1Copy(result.getObject().asA<mx::RtPrim>());
    REQUIRE(add1Copy.getParent().getPath() == add1.getParent().getPath());
    REQUIRE(add1Copy.numInputs() == add1.numInputs());
    REQUIRE(add1Copy.numOutputs() == add1.numOutputs());

    mx::RtCommand::undo(result);
    REQUIRE(result);
    REQUIRE(!add1Copy.isValid());
    mx::RtCommand::redo(result);
    REQUIRE(result);
    REQUIRE(add1Copy.isValid());

    mx::RtCommand::copyPrim(stage, graph1, result);
    REQUIRE(result);
    REQUIRE(result.getObject().isA<mx::RtPrim>());
    mx::RtPrim graph1Copy(result.getObject().asA<mx::RtPrim>());
    REQUIRE(graph1Copy.getParent().getPath() == mx::RtPath("/"));
    REQUIRE(graph1Copy.numChildren() == graph1.numChildren());

    mx::RtNodeGraph ng1Copy(graph1Copy);
    REQUIRE(ng1Copy.asStringDot() == ng1.asStringDot());

    REQUIRE(createPrimCount == 3);
    REQUIRE(removePrimCount == 1);


    //
    // Test node deletion
    //
    makeConnectionCount = 0;
    breakConnectionCount = 0;

    mx::RtCommand::removePrim(stage, add2.getPath(), result);
    REQUIRE(result);
    REQUIRE(!add2.isValid());
    REQUIRE(!add1.getOutput(OUT).isConnected());
    REQUIRE(!ng1.getOutputSocket(OUT).isConnected());

    mx::RtCommand::undo(result);
    REQUIRE(result);
    REQUIRE(add2.isValid());
    REQUIRE(add1.getOutput(OUT).isConnected());
    REQUIRE(add2.getInput(IN1).getConnection() == add1.getOutput(OUT));
    REQUIRE(ng1.getOutputSocket(OUT).getConnection() == add2.getOutput(OUT));

    REQUIRE(makeConnectionCount == 2);
    REQUIRE(breakConnectionCount == 2);

    //
    // Test node reparenting
    //

    reparentPrimCount = 0;

    mx::RtCommand::renamePrim(stage, foo.getPath(), FOO, result);
    mx::RtCommand::createPrim(stage, mx::RtNodeGraph::typeName(), result);
    REQUIRE(result);
    mx::RtPrim graph2(result.getObject().asA<mx::RtPrim>());

    mx::RtCommand::reparentPrim(stage, foo.getPath(), graph2.getPath(), result);
    REQUIRE(result);
    REQUIRE(foo.getParent() == graph2);

    mx::RtCommand::undo(result);
    REQUIRE(result);
    REQUIRE(foo.getParent() == graph1);

    mx::RtCommand::redo(result);
    REQUIRE(result);
    REQUIRE(foo.getParent() == graph2);

    REQUIRE(reparentPrimCount == 3);

    //
    // Test reparenting undo/redo when the reparenting will update the prim name
    //

    mx::RtCommand::createPrim(stage, addFloatNode, graph1.getPath(), FOO, result);
    REQUIRE(result);
    mx::RtPrim foo2(result.getObject().asA<mx::RtPrim>());
    REQUIRE(foo2.getName() == FOO);
    REQUIRE(foo2.getParent() == graph1);

    mx::RtCommand::reparentPrim(stage, foo2.getPath(), graph2.getPath(), result);
    REQUIRE(result);
    REQUIRE(foo2.getParent() == graph2);
    REQUIRE(foo2.getName() == FOO1); // renamed inside graph2 now
    REQUIRE(!graph1.getChild(FOO));
    REQUIRE(graph2.getChild(FOO1));

    mx::RtCommand::undo(result);
    REQUIRE(result);
    REQUIRE(foo2.getParent() == graph1);
    REQUIRE(foo2.getName() == FOO); // original name restored
    REQUIRE(graph1.getChild(FOO));
    REQUIRE(!graph2.getChild(FOO1));

    mx::RtCommand::redo(result);
    REQUIRE(result);
    REQUIRE(foo2.getParent() == graph2);
    REQUIRE(foo2.getName() == FOO1); // renamed inside graph2 again
    REQUIRE(!graph1.getChild(FOO));
    REQUIRE(graph2.getChild(FOO1));

    //
    // Test flush undo queue
    //

    mx::RtCommand::createPrim(stage, addFloatNode, result);
    REQUIRE(result);
    mx::RtPrim node(result.getObject().asA<mx::RtPrim>());
    REQUIRE(node);

    mx::RtCommand::flushUndoQueue();
    mx::RtCommand::undo(result);
    REQUIRE(!result); // undo should fail
    REQUIRE(node);    // node should remain created

    //
    // Remove the callbacks
    //

    mx::RtMessage::removeCallback(createPrimCB_id);
    mx::RtMessage::removeCallback(removePrimCB_id);
    mx::RtMessage::removeCallback(renamePrimCB_id);
    mx::RtMessage::removeCallback(reparentPrimCB_id);
    mx::RtMessage::removeCallback(setAttrCB_id);
    mx::RtMessage::removeCallback(makeConnectionCB_id);
    mx::RtMessage::removeCallback(breakConnectionCB_id);

    //
    // Test failing to create special case prims
    //

    // Look management nodes must be created at the root level
    mx::RtCommandResult colResult;
    mx::RtCommand::createPrim(stage, mx::RtToken("collection"), mx::RtPath("/test/"), mx::EMPTY_TOKEN, colResult);
    REQUIRE(!colResult);

    mx::RtCommandResult matAssignResult;
    mx::RtCommand::createPrim(stage, mx::RtToken("materialassign"), mx::RtPath("/test/"), mx::EMPTY_TOKEN, matAssignResult);
    REQUIRE(!matAssignResult);

    mx::RtCommandResult lookResult;
    mx::RtCommand::createPrim(stage, mx::RtToken("look"), mx::RtPath("/test/"), mx::EMPTY_TOKEN, lookResult);
    REQUIRE(!lookResult);

    mx::RtCommandResult lookGroupResult;
    mx::RtCommand::createPrim(stage, mx::RtToken("lookgroup"), mx::RtPath("/test/"), mx::EMPTY_TOKEN, lookGroupResult);
    REQUIRE(!lookResult);

    // Unknown node types cannot be created. Must be a look managment node, a material node, or have a nodedef.
    mx::RtCommandResult unknownResult;
    mx::RtCommand::createPrim(stage, mx::RtToken("unknown"), mx::RtPath("/"), mx::EMPTY_TOKEN, unknownResult);
    REQUIRE(!unknownResult);
}

TEST_CASE("Runtime: graph output connection", "[runtime]")
{
    mx::RtScopedApiHandle api;

    // Load in all libraries required for materials
    mx::FileSearchPath searchPath(mx::FilePath::getCurrentPath() /
                                  mx::FilePath("libraries"));
    api->setSearchPath(searchPath);
    api->loadLibrary(STDLIB);
    api->loadLibrary(PBRLIB);
    api->loadLibrary(BXDFLIB);

    const std::string mtlxDoc =
        "<?xml version=\"1.0\"?>\n"
        "<materialx version=\"1.38\">\n"
        "  <nodegraph name=\"Compound\">\n"
        "    <output name=\"in\" type=\"color2\" />\n"
        "  </nodegraph>\n"
        "  <clamp name=\"clamp\" type=\"color2\">\n"
        "    <input name=\"in\" type=\"color2\" nodegraph=\"Compound\" />\n"
        "  </clamp>\n"
        "</materialx>";
    mx::RtStagePtr defaultStage = api->createStage(mx::RtToken("defaultStage"));
    mx::RtFileIo   fileIo(defaultStage);
    mx::RtReadOptions options;
    options.applyFutureUpdates = true;
    std::stringstream ss;
    ss << mtlxDoc;
    REQUIRE_NOTHROW(fileIo.read(ss, &options));
}

using TestLoggerPtr = std::shared_ptr<class TestLogger>;
class TestLogger : public mx::RtLogger
{
protected:
    TestLogger() :
        mx::RtLogger()
    {
    }

    void logImpl(mx::RtLogger::MessageType type, const mx::RtToken& msg) override
    {
        if (type == mx::RtLogger::MessageType::ERROR)
        {
            result = "Error: ";
        }
        else if (type == mx::RtLogger::MessageType::WARNING)
        {
            result = "Warning: ";
        }
        else if (type == mx::RtLogger::MessageType::INFO)
        {
            result = "Info: ";
        }

        result += msg.str();
    }

public:
    static TestLoggerPtr get()
    {
        return TestLoggerPtr(new TestLogger());
    }

    std::string result = "";
};

TEST_CASE("Runtime: logging", "[runtime]")
{
    TestLoggerPtr logger = TestLogger::get();
    mx::RtApi& api = mx::RtApi::get();
    api.registerLogger(logger);
    mx::RtToken testMsg("Test");
    api.log(mx::RtLogger::MessageType::ERROR, testMsg);
    REQUIRE("Error: Test" == logger->result);
    api.log(mx::RtLogger::MessageType::WARNING, testMsg);
    REQUIRE("Warning: Test" == logger->result);
    api.log(mx::RtLogger::MessageType::INFO, testMsg);
    REQUIRE("Info: Test" == logger->result);

    logger->enable(mx::RtLogger::MessageType::WARNING, false);
    REQUIRE(logger->isEnabled(mx::RtLogger::MessageType::ERROR));
    REQUIRE(!logger->isEnabled(mx::RtLogger::MessageType::WARNING));
    REQUIRE(logger->isEnabled(mx::RtLogger::MessageType::INFO));
    api.log(mx::RtLogger::MessageType::ERROR, testMsg);
    REQUIRE("Error: Test" == logger->result);
    api.log(mx::RtLogger::MessageType::WARNING, testMsg);
    REQUIRE("Error: Test" == logger->result);
    api.log(mx::RtLogger::MessageType::INFO, testMsg);
    REQUIRE("Info: Test" == logger->result);
}

TEST_CASE("Runtime: duplicate name", "[runtime]")
{
    mx::RtScopedApiHandle api;

    mx::RtStagePtr stage = api->createStage(ROOT);

    // Create a new nodedef for an add node.
    mx::RtNodeDef addFloat = stage->createPrim("/ND_add_float", mx::RtNodeDef::typeName());
    addFloat.setNode(ADD);
    addFloat.createInput(IN1, mx::RtType::FLOAT);
    addFloat.createInput(IN2, mx::RtType::FLOAT);
    addFloat.createOutput(OUT, mx::RtType::FLOAT);
    addFloat.registerMasterPrim();

    // Create a nodegraph object.
    mx::RtNodeGraph graph1 = stage->createPrim("/graph1", mx::RtNodeGraph::typeName());
    REQUIRE(graph1.isValid());

    // Create add node in the graph.
    mx::RtNode add1Node = stage->createPrim("/graph1/add1", addFloat.getName());
    REQUIRE(graph1.getNode(add1Node.getName()));

    // Add an interface to the graph.
    mx::RtInput Ainput = graph1.createInput(A, mx::RtType::FLOAT);
    graph1.createOutput(OUT, mx::RtType::FLOAT);
    REQUIRE(graph1.getPrim().getAttribute(A));
    REQUIRE(graph1.getPrim().getAttribute(OUT));
    REQUIRE(graph1.getInput(A));
    REQUIRE(graph1.getOutput(OUT));
    REQUIRE(graph1.getInputSocket(A));
    REQUIRE(graph1.getOutputSocket(OUT));

    mx::RtToken add1("add1");
    mx::RtToken add2("add2");
    mx::RtToken add3("add3");
    mx::RtToken add4("add4");
    mx::RtToken add5("add5");

    auto duplicateCount = [graph1](const mx::RtToken& name)
    {
        unsigned int count = 0;
        for (auto inputIttr = graph1.getInputs(); !inputIttr.isDone(); ++inputIttr)
        {
            auto input = *inputIttr;
            if (input.getName() == name)
            {
                count++;
            }
        }
        for (auto outputIttr = graph1.getOutputs(); !outputIttr.isDone(); ++outputIttr)
        {
            auto output = *outputIttr;
            if (output.getName() == name)
            {
                count++;
            }
        }
        for (auto nodeIttr = graph1.getNodes(); !nodeIttr.isDone(); ++nodeIttr)
        {
            auto node = *nodeIttr;
            if (node.getName() == name)
            {
                count++;
            }
        }

        return count;
    };

    // Test renaming the input to the name of the node
    REQUIRE(graph1.renameInput(A, add1) == add2);
    REQUIRE(!graph1.getInput(A));
    REQUIRE(!graph1.getInputSocket(A));
    REQUIRE(!graph1.getInput(add1));
    REQUIRE(!graph1.getInputSocket(add1));
    REQUIRE(graph1.getInput(add2));
    REQUIRE(graph1.getInputSocket(add2));
    REQUIRE(duplicateCount(add2) == 1);

    // Test renaming the output to the name of the node
    REQUIRE(graph1.renameOutput(OUT, add1) == add3);
    REQUIRE(!graph1.getOutput(OUT));
    REQUIRE(!graph1.getOutputSocket(OUT));
    REQUIRE(!graph1.getOutput(add1));
    REQUIRE(!graph1.getOutputSocket(add1));
    REQUIRE(graph1.getOutput(add3));
    REQUIRE(graph1.getOutputSocket(add3));
    REQUIRE(duplicateCount(add3) == 1);

    // Add an interface to the graph with the same name as the add node.
    mx::RtInput input2 = graph1.createInput(add1, mx::RtType::FLOAT);
    REQUIRE(graph1.getInput(add4));
    REQUIRE(graph1.getInputSocket(add4));
    REQUIRE(duplicateCount(add4) == 1);

    // Add an output to the graph with the same name as the add node.
    mx::RtOutput output2 = graph1.createOutput(add1, mx::RtType::FLOAT);
    REQUIRE(graph1.getOutput(add5));
    REQUIRE(graph1.getOutputSocket(add5));
    REQUIRE(duplicateCount(add5) == 1);
}

#endif // MATERIALX_BUILD_RUNTIME
