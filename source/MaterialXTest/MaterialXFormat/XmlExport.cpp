//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXFormat/XmlExport.h>

namespace mx = MaterialX;

TEST_CASE("Export Document", "[xmlio]")
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