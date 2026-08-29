#include "fuzztest/fuzztest.h"
#include "gtest/gtest.h"
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXCore/Document.h>

namespace mx = MaterialX;

void ParseXmlString(const std::string& xml_string)
{
    mx::DocumentPtr doc = mx::createDocument();
    try
    {
        mx::readFromXmlString(doc, xml_string);
        doc->validate();
    }
    catch (const mx::Exception&)
    {
        // MaterialX exceptions indicate expected parse/validation failures, not bugs.
    }
}

FUZZ_TEST(MaterialXXmlIoFuzz, ParseXmlString)
    .WithDomains(fuzztest::Arbitrary<std::string>().WithMaxSize(1024 * 1024))
    .WithSeeds(fuzztest::ReadFilesFromDirectory(
        "resources/Materials/Examples"));
