//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXFormat/Environ.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/XmlIo.h>

namespace mx = MaterialX;

TEST_CASE("Load content", "[xmlio]")
{
    mx::FilePath libraryPath("libraries/stdlib");
    mx::FilePath examplesPath("resources/Materials/Examples/Syntax");
    std::string searchPath = libraryPath.asString() +
                             mx::PATH_LIST_SEPARATOR +
                             examplesPath.asString();

    // Read the standard library.
    std::vector<mx::DocumentPtr> libs;
    for (std::string filename : libraryPath.getFilesInDirectory(mx::MTLX_EXTENSION))
    {
        mx::DocumentPtr lib = mx::createDocument();
        mx::readFromXmlFile(lib, filename, searchPath);
        libs.push_back(lib);
    }

    // Read and validate each example document.
    for (std::string filename : examplesPath.getFilesInDirectory(mx::MTLX_EXTENSION))
    {
        mx::DocumentPtr doc = mx::createDocument();
        mx::readFromXmlFile(doc, filename, searchPath);
        for (mx::DocumentPtr lib : libs)
        {
            doc->importLibrary(lib);
        }
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

        // Flatten subgraph references.
        for (mx::NodeGraphPtr nodeGraph : doc->getNodeGraphs())
        {
            if (nodeGraph->getActiveSourceUri() != doc->getSourceUri())
            {
                continue;
            }
            nodeGraph->flattenSubgraphs();
            REQUIRE(nodeGraph->validate());
        }

        // Verify that all referenced types and nodes are declared.
        bool referencesValid = true;
        for (mx::ElementPtr elem : doc->traverseTree())
        {
            if (elem->getActiveSourceUri() != doc->getSourceUri())
            {
                continue;
            }

            mx::TypedElementPtr typedElem = elem->asA<mx::TypedElement>();
            if (typedElem && typedElem->hasType() && !typedElem->isMultiOutputType())
            {
                if (!typedElem->getTypeDef())
                {
                    WARN("[" + typedElem->getActiveSourceUri() + "] TypedElement " + typedElem->getName() + " has no matching TypeDef");
                    referencesValid = false;
                }
            }
            mx::NodePtr node = elem->asA<mx::Node>();
            if (node)
            {
                if (!node->getNodeDef())
                {
                    WARN("[" + node->getActiveSourceUri() + "] Node " + node->getName() + " has no matching NodeDef");
                    referencesValid = false;
                }
            }
        }
        REQUIRE(referencesValid);
    }

    // Read reference document.
    mx::DocumentPtr doc = mx::createDocument();
    std::string filename = "PostShaderComposite.mtlx";
    mx::readFromXmlFile(doc, filename, searchPath);

    // Read document without XIncludes.
    mx::DocumentPtr flatDoc = mx::createDocument();
    mx::XmlReadOptions readOptions;
    readOptions.readXIncludeFunction = nullptr;
    mx::readFromXmlFile(flatDoc, filename, searchPath, &readOptions);
    REQUIRE(*flatDoc != *doc);

    // Read document using environment search path.
    mx::setEnviron(mx::MATERIALX_SEARCH_PATH_ENV_VAR, searchPath);
    mx::DocumentPtr envDoc = mx::createDocument();
    mx::readFromXmlFile(envDoc, filename);
    REQUIRE(*envDoc == *doc);
    mx::removeEnviron(mx::MATERIALX_SEARCH_PATH_ENV_VAR);
    REQUIRE_THROWS_AS(mx::readFromXmlFile(envDoc, filename), mx::ExceptionFileMissing&);

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

    // Import duplicate libraries into document.
    mx::DocumentPtr dupDoc = mx::createDocument();
    for (mx::DocumentPtr lib : libs)
    {
        dupDoc->importLibrary(lib);
        dupDoc->importLibrary(lib);
    }
    REQUIRE(dupDoc->validate());

    // Read a non-existent document.
    mx::DocumentPtr nonExistentDoc = mx::createDocument();
    REQUIRE_THROWS_AS(mx::readFromXmlFile(nonExistentDoc, "NonExistent.mtlx"), mx::ExceptionFileMissing&);

    // Read in include file without specifying search to the parent document
    mx::DocumentPtr parentDoc = mx::createDocument();
    mx::readFromXmlFile(parentDoc,
        "resources/Materials/TestSuite/libraries/metal/brass_wire_mesh.mtlx", searchPath);
    REQUIRE(nullptr != parentDoc->getNodeDef("ND_TestMetal"));
}
