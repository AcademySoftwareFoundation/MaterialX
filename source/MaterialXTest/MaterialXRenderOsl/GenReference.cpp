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

const mx::ShaderPort* getShaderPort(const mx::ShaderStage& stage, const std::string& name)
{
    for (const auto& it : stage.getUniformBlocks())
    {
        const mx::VariableBlock& block = *it.second;
        const mx::ShaderPort* port = block.find(name);
        if (port)
        {
            return port;
        }
    }
    for (const auto& it : stage.getInputBlocks())
    {
        const mx::VariableBlock& block = *it.second;
        const mx::ShaderPort* port = block.find(name);
        if (port)
        {
            return port;
        }
    }
    for (const auto& it : stage.getOutputBlocks())
    {
        const mx::VariableBlock& block = *it.second;
        const mx::ShaderPort* port = block.find(name);
        if (port)
        {
            return port;
        }
    }
    return nullptr;
}

TEST_CASE("GenReference: Reference implementation file test", "[genreference]")
{
    const std::string LIBRARY = "stdlib";

    mx::DocumentPtr stdlibDoc = mx::createDocument();
    mx::FileSearchPath searchPath;
    searchPath.append(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    loadLibraries({ "targets", LIBRARY }, searchPath, stdlibDoc);

    const std::string DEFINITION_PREFIX = "ND_";
    const std::string IMPLEMENTATION_PREFIX = "IM_";
    const std::string IMPLEMENTATION_STRING = "impl";

    const mx::StringVec gentarget = { "genglsl", "genosl" };
    const mx::StringVec language = { "glsl", "osl" };
    const std::vector<bool> outputFunction = { false, false };
    const std::vector<bool> outputFile = { false, false };

    for (size_t i = 0; i < gentarget.size(); i++)
    {
        mx::FilePath librariesPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
        mx::FilePath outputPathRel = LIBRARY + "/" + "reference/" + gentarget[i];
        mx::FilePath implPath = LIBRARY + "/" + gentarget[i];

        mx::FilePath outputPath = librariesPath / outputPathRel;

        // Create output directory
        outputPath.getParentPath().createDirectory();
        outputPath.createDirectory();

        // Create an implementation per nodedef
        //
        const std::string logFilename = gentarget[i] + "_impl_file_test.txt";
        const mx::FilePath logPath(logFilename);
        std::ofstream logFile;

        mx::DocumentPtr implDoc = mx::createDocument();
        const std::vector<mx::NodeDefPtr> nodedefs = stdlibDoc->getNodeDefs();
        for (const mx::NodeDefPtr& nodedef : nodedefs)
        {
            bool hasOutputs = !nodedef->getActiveOutputs().empty();
            CHECK(hasOutputs);
            if (!hasOutputs)
            {
                if (!logFile.is_open())
                {
                    logFile.open(logPath);
                }
                logFile << "Cannot create implementation reference for nodedef which has not outputs: '" << nodedef->getName() << std::endl;
            }

            std::string nodeName = nodedef->getName();
            if (nodeName.size() > 3 && nodeName.substr(0, 3) == DEFINITION_PREFIX)
            {
                nodeName = nodeName.substr(3);
            }

            const std::string filename = nodeName + "." + language[i];
            try
            {
                mx::ImplementationPtr impl = implDoc->addImplementation(
                    IMPLEMENTATION_PREFIX + nodeName + "_" + gentarget[i]);
                impl->setNodeDef(nodedef);
                if (outputFile[i])
                {
                    impl->setFile((implPath / filename).asString(mx::FilePath::FormatPosix));
                }
                if (outputFunction[i])
                {
                    impl->setFunction("mx_" + nodeName);
                }
                impl->setTarget(gentarget[i]);
            }
            catch (mx::ExceptionShaderGenError& e)
            {
                if (!logFile.is_open())
                {
                    logFile.open(logPath);
                }
                logFile << "Cannot create implementation reference for node: '" << nodeName << " : ";
                logFile << e.what() << std::endl;
            }
        }

        // Save implementations to disk
        const std::string implFileName = LIBRARY + "_" + language[i] + "_" + IMPLEMENTATION_STRING + ".refmtlx";
        mx::writeToXmlFile(implDoc, outputPath / implFileName);

        if (logFile.is_open())
        {
            logFile.close();
        }
    }
}

TEST_CASE("GenReference: OSL Reference", "[genreference]")
{
    mx::DocumentPtr stdlibDoc = mx::createDocument();
    mx::DocumentPtr implDoc = mx::createDocument();

    mx::FileSearchPath searchPath;
    searchPath.append(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));
    loadLibraries({ "targets", "stdlib" }, searchPath, stdlibDoc);

    mx::FilePath librariesPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    mx::FilePath outputPathRel = "stdlib/reference/osl";
    mx::FilePath outputPath    = librariesPath / outputPathRel;

    // Create output directory
    outputPath.getParentPath().createDirectory();
    outputPath.createDirectory();

    mx::ShaderGeneratorPtr generator = mx::OslShaderGenerator::create();
    mx::GenContext context(generator);
    context.registerSourceCodeSearchPath(librariesPath);
    context.getOptions().fileTextureVerticalFlip = true;
    context.getOptions().addUpstreamDependencies = false;

    bool runCompileTest = !std::string(MATERIALX_OSLC_EXECUTABLE).empty();
    mx::OslRendererPtr oslRenderer = nullptr;
    if (runCompileTest)
    {
        oslRenderer = mx::OslRenderer::create();
        oslRenderer->setOslCompilerExecutable(MATERIALX_OSLC_EXECUTABLE);
        oslRenderer->setOslIncludePath(MATERIALX_OSL_INCLUDE_PATH);
    }

    const mx::FilePath logPath("genosl_reference_generate_test.txt");
    std::ofstream logFile;
    logFile.open(logPath);

    const std::vector<mx::NodeDefPtr> nodedefs = stdlibDoc->getNodeDefs();
    for (const mx::NodeDefPtr& nodedef : nodedefs)
    {
        std::string nodeName = nodedef->getName();
        if (nodeName.size() > 3 && nodeName.substr(0, 3) == "ND_")
        {
            nodeName = nodeName.substr(3);
        }
        if (nodeName == mx::MATERIAL_TYPE_STRING)
        {
            continue;
        }

        mx::NodePtr node = stdlibDoc->addNodeInstance(nodedef, nodeName);
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

            mx::ImplementationPtr impl = implDoc->addImplementation(node->getName());
            impl->setAttribute(mx::NodeDef::NODE_ATTRIBUTE, nodedef->getNodeString());
            impl->setNodeDef(nodedef);
            impl->setFile((outputPathRel / filename).asString(mx::FilePath::FormatPosix));
            impl->setFunction(node->getName());
            impl->setTarget("osl");

            mx::ShaderStage stage = shader->getStage(mx::Stage::PIXEL);
            for (const mx::ValueElementPtr& elem : nodedef->getActiveValueElements())
            {
                const mx::ShaderPort* port = getShaderPort(stage, elem->getName());
                if (port && port->getName() != port->getVariable())
                {
                    if (elem->isA<mx::Input>())
                    {
                        mx::InputPtr input = impl->addInput(elem->getName(), elem->getType());
                        input->setImplementationName(port->getVariable());
                    }
                }
            }
        }
        catch (mx::Exception & e)
        {
            logFile << "Error generating OSL reference for '" << nodeName << "' : " << std::endl;
            logFile << e.what() << std::endl;
        }

        stdlibDoc->removeChild(node->getName());
    }

    mx::writeToXmlFile(implDoc, outputPath / "stdlib_osl_impl.refmtlx");

    logFile.close();
}
