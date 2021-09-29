//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXFormat/XmlExport.h>

namespace mx = MaterialX;

TEST_CASE("File Path Predicate", "[xmlexport]")
{
    mx::FileSearchPath searchPath("libraries/stdlib");
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, "resources/Materials/TestSuite/stdlib/export/export.mtlx", searchPath);

    mx::XmlExportOptions exportOptions;
    exportOptions.flattenFilenames = true;
    exportOptions.resolvedTexturePath = mx::FileSearchPath(mx::FilePath::getCurrentPath() /
        "resources" /
        "Materials" /
        "TestSuite" /
        "stdlib" /
        "export" );
    mx::FileSearchPath textureSearchPath("resources");
    exportOptions.skipFlattening = [textureSearchPath](const mx::FilePath& filePath) -> bool
    {
        return textureSearchPath.find(filePath) != filePath;
    };
    std::stringstream ss;
    mx::exportToXmlStream(doc, ss, &exportOptions);
    mx::NodePtr node = doc->getNode("image_color3");
    mx::NodePtr node2 = doc->getNode("image_color3_2");
    REQUIRE(node);
    REQUIRE(node2);
    mx::InputPtr input = node->getInput("file");
    mx::InputPtr input2 = node2->getInput("file");
    REQUIRE(input->getValueString() == "Images/grid.png");
    REQUIRE(mx::FileSearchPath(input2->getValueString()).asString() == exportOptions.resolvedTexturePath.find("black_image.png").asString());
}
