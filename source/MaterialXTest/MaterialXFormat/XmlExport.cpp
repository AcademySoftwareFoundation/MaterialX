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
    exportOptions.mergeLooks = true;
    exportOptions.lookGroupToMerge = "lookgroup1";
    std::stringstream ss;
    mx::exportToXmlStream(doc, ss, &exportOptions);

    mx::DocumentPtr exportedDoc = mx::createDocument();
    mx::readFromXmlStream(exportedDoc, ss);

    REQUIRE(exportedDoc->getLookGroups().size() == 0);
    REQUIRE(exportedDoc->getLooks().size() == 1);
}