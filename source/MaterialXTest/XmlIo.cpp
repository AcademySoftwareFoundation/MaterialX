//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXFormat/XmlIo.h>

namespace mx = MaterialX;

TEST_CASE("Load content", "[xmlio]")
{
    std::string libFilename = "mx_stdlib_defs.mtlx";
    std::string exampleFilenames[] =
    {
        "CustomNode.mtlx",
        "Looks.mtlx",
        "MaterialGraphs.mtlx",
        "PaintMaterials.mtlx",
        "PreShaderComposite.mtlx",
    };
    std::string searchPath = "documents/Libraries/stdlib;documents/Examples";

    // Load the standard library.
    mx::DocumentPtr lib = mx::createDocument();
    mx::readFromXmlFile(lib, libFilename, searchPath);
    REQUIRE(lib->validate());

    for (std::string filename : exampleFilenames)
    {
        // Load the example document.
        mx::DocumentPtr doc = mx::createDocument();
        mx::readFromXmlFile(doc, filename, searchPath);
        REQUIRE(doc->validate());

        // Traverse the document tree
        int valueElementCount = 0;
        for (mx::ElementPtr elem : doc->traverseTree())
        {
            if (elem->isA<mx::ValueElement>())
            {
                valueElementCount++;
            }
        }
        REQUIRE(valueElementCount > 0);

        // Traverse the dataflow graph from each shader input to its source nodes.
        for (mx::MaterialPtr material : doc->getMaterials())
        {
            REQUIRE(!material->getReferencedShaderDefs().empty());
            int edgeCount = 0;
            for (mx::NodeDefPtr shader : material->getReferencedShaderDefs())
            {
                for (mx::InputPtr input : shader->getInputs())
                {
                    for (mx::Edge edge : input->traverseGraph(material))
                    {
                        edgeCount++;
                    }
                }
                for (mx::ParameterPtr param : shader->getParameters())
                {
                    for (mx::Edge edge : param->traverseGraph(material))
                    {
                        edgeCount++;
                    }
                }
            }
            REQUIRE(edgeCount > 0);
        }

        // Serialize to XML.
        std::string xmlString = mx::writeToXmlString(doc, false);

        // Verify that the serialized document is identical.
        mx::DocumentPtr writtenDoc = mx::createDocument();
        mx::readFromXmlString(writtenDoc, xmlString);
        REQUIRE(*writtenDoc == *doc);

        // Serialize to XML with a custom predicate that skips images.
        auto skipImages = [](mx::ElementPtr elem)
        {
            return !elem->isA<mx::Node>("image");
        };
        xmlString = mx::writeToXmlString(doc, false, skipImages);
        
        // Verify that the serialized document contains no images.
        writtenDoc = mx::createDocument();
        mx::readFromXmlString(writtenDoc, xmlString);
        unsigned imageElementCount = 0;
        for (mx::ElementPtr elem : writtenDoc->traverseTree())
        {
            if (elem->isA<mx::Node>("image"))
            {
                imageElementCount++;
            }
        }
        REQUIRE(imageElementCount == 0);

        // Combine document with the standard library.
        mx::DocumentPtr doc2 = doc->copy();
        doc2->importLibrary(lib);
        REQUIRE(doc2->validate());

        // Verify that all referenced nodes are declared.
        for (mx::ElementPtr elem : doc2->traverseTree())
        {
            mx::NodePtr node = elem->asA<mx::Node>();
            if (node)
            {
                REQUIRE(node->getReferencedNodeDef());
            }
        }

        // Flatten subgraph references.
        doc2 = doc->copy();
        for (mx::NodeGraphPtr nodeGraph : doc2->getNodeGraphs())
        {
            nodeGraph->flattenSubgraphs();
        }
        REQUIRE(doc2->validate());

        // Read document without XIncludes.
        doc2 = mx::createDocument();
        mx::readFromXmlFile(doc2, filename, searchPath, false);
        if (*doc2 != *doc)
        {
            writtenDoc = mx::createDocument();
            xmlString = mx::writeToXmlString(doc);
            mx::readFromXmlString(writtenDoc, xmlString);
            REQUIRE(*doc2 == *writtenDoc);
        }
    }

    // Read the same documents more than once.
    // When duplcate names are found an error exception is thrown.
    // Setting to skip duplicates names first avoid trying to
    // create a child with a duplicate name in the first place
    // thus no error exception is thrown.
    mx::DocumentPtr doc3 = mx::createDocument();
    const bool readXincludes = true;
    const bool skipDuplicates = true;
    bool exceptionThrown = false;
    try
    {
        mx::readFromXmlFile(doc3, libFilename, searchPath);
        mx::readFromXmlFile(doc3, libFilename, searchPath, readXincludes, skipDuplicates);
        for (std::string filename : exampleFilenames)
        {
            mx::readFromXmlFile(doc3, filename, searchPath, readXincludes, skipDuplicates);
            mx::readFromXmlFile(doc3, filename, searchPath, readXincludes, skipDuplicates);
        }
    }
    catch (MaterialX::Exception e)
    {
        exceptionThrown = true;
    }
    catch (...)
    {
    }
    REQUIRE(exceptionThrown == false);
}
