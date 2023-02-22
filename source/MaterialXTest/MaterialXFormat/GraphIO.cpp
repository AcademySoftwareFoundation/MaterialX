//
// TM & (c) 2022 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/GraphIo.h>


namespace mx = MaterialX;

TEST_CASE("GraphIo: Generate Functional Graphs", "[graphio]")
{
    mx::FilePathVec libraries = { "libraries/targets", "libraries/stdlib",
                                  "libraries/pbrlib", "libraries/bxdf" };
    mx::StringVec libraryNames = { "targets", "stdlib",
                                  "pbrlib", "bxdf" };

    mx::DocumentPtr stdlib = mx::createDocument();
    mx::FilePath currentPath = mx::FilePath::getCurrentPath();
    mx::FileSearchPath searchPath(currentPath);
    loadLibraries(libraries, searchPath, stdlib);

    // Create output directory.
    mx::FilePath outputPath = mx::FilePath::getCurrentPath() / mx::FilePath("graphio");
    outputPath.createDirectory();
    for (const std::string& libraryName : libraryNames)
    {
        mx::FilePath libPath = outputPath / libraryName;
        libPath.createDirectory();
    }

    // Create log file.
    const mx::FilePath logPath("graphio_test.txt");
    std::ofstream logFile;
    logFile.open(logPath);

    bool failedGeneration = false;

    mx::GraphIoPtr graphers[2] = { mx::MermaidGraphIo::create(), mx::DotGraphIo::create() };
    mx::StringVec extensions = { "md", "dot" };

    for (const mx::NodeDefPtr& nodedef : stdlib->getNodeDefs())
    {
        std::string nodeName = nodedef->getName();
        std::string nodeNode = nodedef->getNodeString();
        if (nodeName.size() > 3 && nodeName.substr(0, 3) == "ND_")
        {
            nodeName = nodeName.substr(3);
        }

        mx::InterfaceElementPtr interface = nodedef->getImplementation();
        mx::NodeGraphPtr nodegraph = interface ? interface->asA<mx::NodeGraph>() : nullptr;
        if (!nodegraph)
        {
            logFile << "Skip generating for node with graph representation '" << nodeName << "'" << std::endl;
            continue;
        }

        try
        {
            mx::StringVec roots;
            std::vector<bool> writeCategoriesList = { true, false };

            for (size_t i=0; i<2; i++)
            {
                mx::GraphIoGenOptions graphOptions;
                graphOptions.setWriteSubgraphs(false);
                graphOptions.setOrientation(mx::GraphIoGenOptions::Orientation::LEFT_RIGHT);
                graphers[i]->setGenOptions(graphOptions);

                for (auto writeCategories : writeCategoriesList)
                {
                    graphOptions.setWriteCategories(writeCategories);
                    std::string graphString = graphers[i]->write(nodegraph, roots);
                    if (!graphString.empty())
                    {
                        logFile << "Wrote " + (!writeCategories ? " instanced " : mx::EMPTY_STRING) + "graph for node '" << nodeName << "'" << std::endl;

                        mx::FilePath libPath = outputPath;
                        for (const std::string& libraryName : libraryNames)
                        {
                            const std::string uri = nodegraph->getSourceUri();
                            if (std::string::npos != uri.find(libraryName))
                            {
                                libPath = outputPath / libraryName;
                                break;
                            }
                        }

                        const std::string filename = nodeName + (!writeCategories ? "_instance" : mx::EMPTY_STRING) + "." + extensions[i];
                        const std::string filepath = (libPath / filename).asString();
                        std::ofstream file;
                        file.open(filepath);
                        REQUIRE(file.is_open());
                        if (extensions[i] == "md")
                        {
                            std::string result = "```mermaid\n";
                            result += graphString;
                            result += "```\n";
                            file << result;
                        }
                        else
                        {
                            file << graphString;
                        }
                        file.close();
                    }
                    else
                    {
                        logFile << "Failed to write graph for node '" << nodeName << "'" << std::endl;
                        failedGeneration = true;
                    }
                }
            }
        }
        catch (mx::Exception& e)
        {
            logFile << "Error generating graph for '" << nodeName << "' : " << std::endl;
            logFile << e.what() << std::endl;
            failedGeneration = true;
        }
    }

    logFile.close();

    CHECK(failedGeneration == false);
}
