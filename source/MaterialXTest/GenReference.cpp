//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXTest/GenShaderUtil.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/OslSyntax.h>

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

TEST_CASE("GenShader: OSL Reference", "[genshader]")
{
    mx::DocumentPtr stdlibDoc = mx::createDocument();
    mx::DocumentPtr implDoc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    loadLibraries({ "stdlib" }, searchPath, stdlibDoc);

    mx::FilePath librariesPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    mx::FilePath outputPathRel = "stdlib/reference/osl";
    mx::FilePath outputPath    = librariesPath / outputPathRel;

    // Create output directory
    outputPath.getParentPath().createDirectory();
    outputPath.createDirectory();

    mx::ShaderGeneratorPtr generator = mx::OslShaderGenerator::create();
    mx::GenContext context(generator);
    context.registerSourceCodeSearchPath(librariesPath);

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

        mx::NodePtr node = stdlibDoc->addNode(nodedef->getNodeString(), nodeName, nodedef->getType());
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

            mx::ImplementationPtr impl = implDoc->addImplementation("IM_" + nodeName + "_osl");
            impl->setNodeDef(nodedef);
            impl->setFile((outputPathRel / filename).asString(mx::FilePath::FormatPosix));
            impl->setFunction(node->getName());
            impl->setLanguage("osl");

            mx::ShaderStage stage = shader->getStage(mx::Stage::PIXEL);
            for (const mx::ValueElementPtr elem : nodedef->getActiveValueElements())
            {
                const mx::ShaderPort* port = getShaderPort(stage, elem->getName());
                if (port && port->getName() != port->getVariable())
                {
                    if (elem->isA<mx::Input>())
                    {
                        mx::InputPtr input = impl->addInput(elem->getName(), elem->getType());
                        input->setImplementationName(port->getVariable());
                    }
                    else
                    {
                        mx::ParameterPtr param = impl->addParameter(elem->getName(), elem->getType());
                        param->setImplementationName(port->getVariable());
                    }
                }
            }
        }
        catch (mx::ExceptionShaderGenError& e)
        {
            logFile << "Generating node not current supported: '" << nodeName << " : ";
            logFile << e.what() << std::endl;
        }

        stdlibDoc->removeChild(node->getName());

    }

    mx::writeToXmlFile(implDoc, outputPath / "stdlib_osl_impl.mtlx");

    logFile.close();
}
