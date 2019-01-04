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
        "stdlib_defs.mtlx",
        "stdlib_ng.mtlx",
        "stdlib_osl_impl.mtlx"
    };
    std::string exampleFilenames[] =
    {
        "CustomNode.mtlx",
        "Looks.mtlx",
        "MaterialBasic.mtlx",
        "MultiOutput.mtlx",
        "NodeGraphs.mtlx",
        "PaintMaterials.mtlx",
        "PostShaderComposite.mtlx",
        "PreShaderComposite.mtlx",
        "BxDF/alSurface.mtlx",
        "BxDF/Disney_BRDF_2012.mtlx",
        "BxDF/Disney_BSDF_2015.mtlx",
    };
    std::string searchPath = "documents/Libraries/stdlib;documents/Examples";

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
    bool firstExample = true;
    for (std::string filename : exampleFilenames)
    {
        mx::DocumentPtr doc = mx::createDocument();
        mx::readFromXmlFile(doc, filename, searchPath);
        std::string message;
        bool docValid = doc->validate(&message);
        if (!docValid)
        {
            WARN("[" + filename + "] " + message);
        }
        REQUIRE(docValid);

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
        mx::XmlWriteOptions writeOptions;
        writeOptions.writeXIncludeEnable = false;
        std::string xmlString = mx::writeToXmlString(doc, &writeOptions);

        // Verify that the serialized document is identical.
        mx::DocumentPtr writtenDoc = mx::createDocument();
        mx::readFromXmlString(writtenDoc, xmlString);
        REQUIRE(*writtenDoc == *doc);

        // Combine document with the standard library.
        for (mx::DocumentPtr lib : libs)
        {
            doc->importLibrary(lib);
        }
        REQUIRE(doc->validate());

        // Flatten subgraph references.
        for (mx::NodeGraphPtr nodeGraph : doc->getNodeGraphs())
        {
            if (!firstExample && nodeGraph->getActiveSourceUri() != doc->getSourceUri())
            {
                continue;
            }
            nodeGraph->flattenSubgraphs();
            REQUIRE(nodeGraph->validate());
        }

        // Verify that all referenced types and nodes are declared, and that
        // referenced node declarations are implemented.
        bool referencesValid = true;
        for (mx::ElementPtr elem : doc->traverseTree())
        {
            if (!firstExample && elem->getActiveSourceUri() != doc->getSourceUri())
            {
                continue;
            }

            mx::TypedElementPtr typedElem = elem->asA<mx::TypedElement>();
            mx::NodePtr node = elem->asA<mx::Node>();
            if (typedElem && typedElem->hasType() && !typedElem->isMultiOutputType())
            {
                if (!typedElem->getTypeDef())
                {
                    WARN("[" + node->getActiveSourceUri() + "] TypedElement " + node->getName() + " has no matching TypeDef");
                    referencesValid = false;
                }
            }
            if (node)
            {
                if (!node->getNodeDef())
                {
                    WARN("[" + node->getActiveSourceUri() + "] Node " + node->getName() + " has no matching NodeDef");
                    referencesValid = false;
                }
                if (!node->getImplementation())
                {
                    WARN("[" + node->getActiveSourceUri() + "] Node " + node->getName() + " has no matching Implementation");
                    referencesValid = false;
                }
            }
        }
        REQUIRE(referencesValid);

        firstExample = false;
    }

    // Read the same document twice with duplicate elements skipped.
    mx::DocumentPtr doc = mx::createDocument();
    mx::XmlReadOptions readOptions;
    readOptions.skipDuplicateElements = true;
    std::string filename = "PostShaderComposite.mtlx";
    mx::readFromXmlFile(doc, filename, searchPath, &readOptions);
    mx::readFromXmlFile(doc, filename, searchPath, &readOptions);
    REQUIRE(doc->validate());

    // Read document without XIncludes.
    mx::DocumentPtr flatDoc = mx::createDocument();
    readOptions = mx::XmlReadOptions();
    readOptions.readXIncludeFunction = nullptr;
    mx::readFromXmlFile(flatDoc, filename, searchPath, &readOptions);
    REQUIRE(*flatDoc != *doc);

    // Serialize to XML with a custom predicate that skips images.
    auto skipImages = [](mx::ElementPtr elem)
    {
        return !elem->isA<mx::Node>("image");
    };
    mx::XmlWriteOptions writeOptions;
    writeOptions.writeXIncludeEnable = false;
    writeOptions.elementPredicate = skipImages;
    std::string xmlString = mx::writeToXmlString(doc, &writeOptions);
        
    // Reconstruct and verify that the document contains no images.
    mx::DocumentPtr writtenDoc = mx::createDocument();
    mx::readFromXmlString(writtenDoc, xmlString);
    REQUIRE(*writtenDoc != *doc);
    unsigned imageElementCount = 0;
    for (mx::ElementPtr elem : writtenDoc->traverseTree())
    {
        if (elem->isA<mx::Node>("image"))
        {
            imageElementCount++;
        }
    }
    REQUIRE(imageElementCount == 0);

    // Read a non-existent document.
    mx::DocumentPtr nonExistentDoc = mx::createDocument();
    REQUIRE_THROWS_AS(mx::readFromXmlFile(nonExistentDoc, "NonExistent.mtlx"), mx::ExceptionFileMissing&);
}
