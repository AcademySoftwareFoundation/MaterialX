//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXFormat/XmlIo.h>

namespace mx = MaterialX;

TEST_CASE("Load content", "[xmlio]")
{
    std::string libraryFilenames[] =
    {
        "mx_stdlib_defs.mtlx",
        "mx_stdlib_impl_osl.mtlx"
    };
    std::string exampleFilenames[] =
    {
        "CustomNode.mtlx",
        "Looks.mtlx",
        "MaterialGraphs.mtlx",
        "MultiOutput.mtlx",
        "PaintMaterials.mtlx",
        "PreShaderComposite.mtlx",
    };
    std::string searchPath = "documents/Libraries;documents/Examples";

    // Load the standard library.
    std::vector<mx::DocumentPtr> libs;
    for (std::string filename : libraryFilenames)
    {
        mx::DocumentPtr lib = mx::createDocument();
        mx::readFromXmlFile(lib, filename, searchPath);
        REQUIRE(lib->validate());
        libs.push_back(lib);
    }

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

        // Traverse the dataflow graph from each shader parameter and input
        // to its source nodes.
        for (mx::MaterialPtr material : doc->getMaterials())
        {
            REQUIRE(material->getPrimaryShaderNodeDef());
            int edgeCount = 0;
            for (mx::ParameterPtr param : material->getPrimaryShaderParameters())
            {
                REQUIRE(param->getBoundValue(material));
                for (mx::Edge edge : param->traverseGraph(material))
                {
                    edgeCount++;
                }
            }
            for (mx::InputPtr input : material->getPrimaryShaderInputs())
            {
                REQUIRE((input->getBoundValue(material) || input->getUpstreamElement(material)));
                for (mx::Edge edge : input->traverseGraph(material))
                {
                    edgeCount++;
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
        for (mx::DocumentPtr lib : libs)
        {
            doc2->importLibrary(lib);
        }
        REQUIRE(doc2->validate());

        // Verify that all referenced nodes are declared and implemented.
        for (mx::ElementPtr elem : doc2->traverseTree())
        {
            mx::NodePtr node = elem->asA<mx::Node>();
            if (node)
            {
                REQUIRE(node->getNodeDef());
                REQUIRE(node->getImplementation());
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

        mx::XmlReadOptions readingOptions;
        readingOptions._readXincludes = false;
        mx::readFromXmlFile(doc2, filename, searchPath, &readingOptions);
        if (*doc2 != *doc)
        {
            writtenDoc = mx::createDocument();
            xmlString = mx::writeToXmlString(doc);
            mx::readFromXmlString(writtenDoc, xmlString, &readingOptions);
            REQUIRE(*doc2 == *writtenDoc);
        }
    }

    // Read the same documents more than once.
    // When duplcate names are found an error exception is thrown.
    // Setting to skip duplicates names first avoid trying to
    // create a child with a duplicate name in the first place
    // thus no error exception is thrown.
    mx::DocumentPtr doc3 = mx::createDocument();
    mx::XmlReadOptions readingOptions;
    readingOptions._readXincludes = true;
    bool exceptionThrown = false;
    try
    {
        for (std::string libFilename : libraryFilenames)
        {
            readingOptions._skipDuplicates = false;
            mx::readFromXmlFile(doc3, libFilename, searchPath, &readingOptions);
            readingOptions._skipDuplicates = true;
            mx::readFromXmlFile(doc3, libFilename, searchPath, &readingOptions);
        }
        for (std::string filename : exampleFilenames)
        {
            mx::readFromXmlFile(doc3, filename, searchPath, &readingOptions);
            mx::readFromXmlFile(doc3, filename, searchPath, &readingOptions);
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
