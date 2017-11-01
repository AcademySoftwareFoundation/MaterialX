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

    // Read the standard library.
    std::vector<mx::DocumentPtr> libs;
    for (std::string filename : libraryFilenames)
    {
        mx::DocumentPtr lib = mx::createDocument();
        mx::readFromXmlFile(lib, filename, searchPath);
        REQUIRE(lib->validate());
        libs.push_back(lib);
    }

    // Read and validate each example document.
    for (std::string filename : exampleFilenames)
    {
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
        mx::XmlReadOptions readOptions;
        readOptions.readXIncludes = false;
        mx::readFromXmlFile(doc2, filename, searchPath, &readOptions);
        if (*doc2 != *doc)
        {
            writtenDoc = mx::createDocument();
            xmlString = mx::writeToXmlString(doc);
            mx::readFromXmlString(writtenDoc, xmlString, &readOptions);
            REQUIRE(*doc2 == *writtenDoc);
        }
    }

    // Read the same document twice with duplicate elements skipped.
    mx::DocumentPtr doc = mx::createDocument();
    mx::XmlReadOptions readOptions;
    readOptions.skipDuplicateElements = true;
    std::string filename = "PaintMaterials.mtlx";
    mx::readFromXmlFile(doc, filename, searchPath, &readOptions);
    mx::readFromXmlFile(doc, filename, searchPath, &readOptions);
    REQUIRE(doc->validate());

    // Read a non-existent document.
    mx::DocumentPtr doc2 = mx::createDocument();
    REQUIRE_THROWS_AS(mx::readFromXmlFile(doc2, "NonExistent.mtlx"), mx::ExceptionFileMissing);
}
