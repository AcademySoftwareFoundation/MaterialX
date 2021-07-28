//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXFormat/XmlExport.h>

namespace mx = MaterialX;

TEST_CASE("Export Document", "[xmlexport]")
{
    mx::FileSearchPath searchPath("libraries/stdlib");
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, "resources/Materials/TestSuite/stdlib/looks/looks.mtlx", searchPath);

    mx::XmlExportOptions exportOptions;
    exportOptions.lookGroupToMerge = "lookgroup1";
    std::stringstream ss;
    exportOptions.modifyInPlace = false;
    mx::exportToXmlStream(doc, ss, &exportOptions);

    // Test without overwriting the original
    mx::DocumentPtr exportedDoc = mx::createDocument();
    mx::readFromXmlStream(exportedDoc, ss);

    REQUIRE(exportedDoc->getLookGroups().size() == 0);
    REQUIRE(exportedDoc->getLooks().size() == 1);
    REQUIRE(doc->getLookGroups().size() != 0);
    REQUIRE(doc->getLooks().size() > 1);

    // Test with overwriting the original
    exportOptions.modifyInPlace = true;
    mx::exportToXmlStream(doc, ss, &exportOptions);

    exportedDoc = mx::createDocument();
    mx::readFromXmlStream(exportedDoc, ss);

    REQUIRE(exportedDoc->getLookGroups().size() == doc->getLookGroups().size());
    REQUIRE(exportedDoc->getLooks().size() == doc->getLooks().size());

    // Try exporting again should fail
    REQUIRE_THROWS(mx::exportToXmlStream(doc, ss, &exportOptions));
}

TEST_CASE("File Path Predicate", "[xmlexport]")
{
    mx::FileSearchPath searchPath("libraries/stdlib");
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, "resources/Materials/TestSuite/stdlib/export/export.mtlx", searchPath);

    mx::XmlExportOptions exportOptions;
    exportOptions.mergeLooks = true;
    exportOptions.lookGroupToMerge = "defaultLookGroup";
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
