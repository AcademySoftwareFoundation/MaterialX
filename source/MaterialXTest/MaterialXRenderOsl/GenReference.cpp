//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <MaterialXGenShader/ShaderStage.h>

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslSyntax.h>

#include <MaterialXRenderOsl/OslRenderer.h>

namespace mx = MaterialX;

TEST_CASE("GenReference: OSL Reference", "[genreference]")
{
    mx::DocumentPtr stdlib = mx::createDocument();
    mx::FilePath currentPath = mx::FilePath::getCurrentPath();
    mx::FileSearchPath searchPath(currentPath);
    loadLibraries({ "libraries/targets", "libraries/stdlib" }, searchPath, stdlib);

    // Create renderer if requested.
    bool runCompileTest = !std::string(MATERIALX_OSL_BINARY_OSLC).empty();
    mx::OslRendererPtr oslRenderer = nullptr;
    if (runCompileTest)
    {
        oslRenderer = mx::OslRenderer::create();
        oslRenderer->setOslCompilerExecutable(MATERIALX_OSL_BINARY_OSLC);
        mx::FileSearchPath oslIncludePaths; 
        oslIncludePaths.append(mx::FilePath(MATERIALX_OSL_INCLUDE_PATH));
        // Add in library include path for compile testing as the generated
        // shader's includes are not added with absolute paths.
        oslIncludePaths.append(currentPath / mx::FilePath("libraries/stdlib/genosl/include"));
        oslRenderer->setOslIncludePath(oslIncludePaths);
    }

    // Create shader generator.
    mx::ShaderGeneratorPtr generator = mx::OslShaderGenerator::create();
    mx::GenContext context(generator);
    context.getOptions().addUpstreamDependencies = false;
    context.registerSourceCodeSearchPath(currentPath);
    context.getOptions().fileTextureVerticalFlip = true;

    // Create output directory.
    mx::FilePath outputPath = mx::FilePath::getCurrentPath() / mx::FilePath("reference/osl");
    outputPath.getParentPath().createDirectory();
    outputPath.createDirectory();

    // Create log file.
    const mx::FilePath logPath("genosl_reference_generate_test.txt");
    std::ofstream logFile;
    logFile.open(logPath);

    // Generate reference shaders.
    // Ignore the following nodes:
    const mx::StringSet ignoreNodeList = { "surfacematerial", "volumematerial",
                                           "constant_filename", "arrayappend",
                                           "dot_filename"};

    bool failedGeneration = false;
    for (const mx::NodeDefPtr& nodedef : stdlib->getNodeDefs())
    {
        std::string nodeName = nodedef->getName();
        std::string nodeNode = nodedef->getNodeString();
        if (nodeName.size() > 3 && nodeName.substr(0, 3) == "ND_")
        {
            nodeName = nodeName.substr(3);
        }
        if (nodeName == mx::MATERIAL_TYPE_STRING || 
            ignoreNodeList.find(nodeName) != ignoreNodeList.end() ||
            ignoreNodeList.find(nodeNode) != ignoreNodeList.end())
        {
            logFile << "Skip generating reference for'" << nodeName << "'" << std::endl;
            continue;
        }

        mx::InterfaceElementPtr interface = nodedef->getImplementation();
        if (!interface)
        {
            logFile << "Skip generating reference for unimplemented node '" << nodeName << "'" << std::endl;
            continue;
        }

        mx::NodePtr node = stdlib->addNodeInstance(nodedef, nodeName);
        REQUIRE(node);

        const std::string filename = nodeName + ".osl";
        try
        {
            mx::ShaderPtr shader = generator->generate(node->getName(), node, context);

            std::ofstream file;
            const std::string filepath = (outputPath / filename).asString();
            file.open(filepath);
            REQUIRE(file.is_open());
            file << shader->getSourceCode();
            file.close();

            if (oslRenderer)
            {
                oslRenderer->compileOSL(filepath);
            }
        }
        catch (mx::ExceptionRenderError& e)
        {
            logFile << "Error compiling OSL reference for '" << nodeName << "' : " << std::endl;
            logFile << e.what() << std::endl;
            for (const std::string& error : e.errorLog())
            {
                logFile << error << std::endl;
            }
            failedGeneration = true;
        }
        catch (mx::Exception& e)
        {
            logFile << "Error generating OSL reference for '" << nodeName << "' : " << std::endl;
            logFile << e.what() << std::endl;
            failedGeneration = true;
        }

        stdlib->removeChild(node->getName());
    }

    logFile.close();

    CHECK(failedGeneration == false);
}
