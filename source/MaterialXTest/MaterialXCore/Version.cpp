//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXCore/Util.h>
#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/Util.h>

// TODO: move to format/ ?

namespace mx = MaterialX;

const char* swizzleDoc1 = R"(
<?xml version="1.0"?>
<materialx version="1.38">
  <constant name="f" type="float">
    <input name="value" type="float" value="0" />
  </constant>
  <constant name="v2" type="vector2">
    <input name="value" type="vector2" value="0, 0" />
  </constant>
  <constant name="v3" type="vector3">
    <input name="value" type="vector3" value="0, 0, 0" />
  </constant>
  <constant name="v4" type="vector4">
    <input name="value" type="vector4" value="0, 0, 0, 1" />
  </constant>

  <swizzle name="xyz_to_xyz" type="vector3">
    <input name="in" type="vector3" nodename="v3" />
    <input name="channels" type="string" value="xyz" />
  </swizzle>

  <swizzle name="xyz_to_rgb" type="color3">
    <input name="in" type="vector3" nodename="v3" />
    <input name="channels" type="string" value="xyz" />
  </swizzle>

  <swizzle name="xy_to_xxx" type="vector3">
    <input name="in" type="vector2" nodename="v2" />
    <input name="channels" type="string" value="xxx" />
  </swizzle>

  <swizzle name="x_to_xxx" type="vector3">
    <input name="in" type="float" nodename="f" />
    <input name="channels" type="string" value="xxx" />
  </swizzle>
</materialx>
)";

TEST_CASE("Handling 1.38 swizzle nodes", "[version]")
{
    mx::DocumentPtr libs = mx::createDocument();
    mx::loadLibraries({ "libraries" }, mx::getDefaultDataSearchPath(), libs);

    auto doc = mx::createDocument();
    doc->importLibrary(libs);
    mx::readFromXmlBuffer(doc, swizzleDoc1);
    doc->validate();

    struct ExpectedNodes
    {
        std::string name;
        std::string category;
        std::string nodeDefName;
    };

    ExpectedNodes expectedNodes[] = {
        { "xyz_to_xyz", "combine3", "ND_combine3_vector3" },
        { "xyz_to_rgb", "convert", "ND_convert_vector3_color3" },
        { "xy_to_xxx", "combine3", "ND_combine3_vector3" },
        { "x_to_xxx", "convert", "ND_convert_float_vector3" },
    };

    for (auto& expected : expectedNodes)
    {
        auto node = doc->getNode(expected.name);
        REQUIRE(node);
        REQUIRE(node->getCategory() == expected.category);
        auto nodeDef = node->getNodeDef();
        REQUIRE(nodeDef);
        REQUIRE(nodeDef->getName() == expected.nodeDefName);
    }
}