//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/File.h>

#include <MaterialXGenShader/Shader.h>

#include <MaterialXGenOgsXml/GlslFragmentGenerator.h>
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>

#include <MaterialXTest/GenShaderUtil.h>
#include <MaterialXTest/GenGlsl.h>

namespace mx = MaterialX;

TEST_CASE("GenShader: OGS XML Generation", "[ogsxml]")
{
    mx::DocumentPtr doc = mx::createDocument();

    const mx::FilePath librariesPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
    loadLibraries({ "stdlib", "pbrlib", "bxdf", "lights" }, librariesPath, doc);

    const mx::FilePath resourcesPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources");
    loadLibraries({ "Materials/TestSuite", "Materials/Examples" }, resourcesPath, doc);

    mx::ShaderGeneratorPtr glslGenerator = mx::GlslFragmentGenerator::create();
    mx::GenContext glslContext(glslGenerator);
    glslContext.registerSourceCodeSearchPath(librariesPath);
    glslContext.getOptions().fileTextureVerticalFlip = true;

    mx::OgsXmlGenerator xmlGenerator;

    mx::StringVec testGraphs = { };
    mx::StringVec testMaterials = { "Tiled_Brass", "Brass_Wire_Mesh" };

    for (auto testGraph : testGraphs)
    {
        mx::NodeGraphPtr graph = doc->getNodeGraph(testGraph);
        if (graph)
        {
            std::vector<mx::OutputPtr> outputs = graph->getOutputs();
            for (auto output : outputs)
            {
                const std::string name = graph->getName() + "_" + output->getName();
                mx::ShaderPtr shader = glslGenerator->generate(name, output, glslContext);
                std::ofstream file(name + ".xml");
                std::string shaderName = output->getNamePath();
                shaderName = MaterialX::createValidName(shaderName);
                xmlGenerator.generate(shaderName, shader.get(), nullptr, file);
            }
        }
    }

    for (auto testMaterial : testMaterials)
    {
        mx::MaterialPtr mtrl = doc->getMaterial(testMaterial);
        if (mtrl)
        {
            std::vector<mx::ShaderRefPtr> shaderRefs = mtrl->getShaderRefs();
            for (auto shaderRef : shaderRefs)
            {
                mx::ShaderPtr shader = glslGenerator->generate(shaderRef->getName(), shaderRef, glslContext);
                std::ofstream file(shaderRef->getName() + ".xml");
                std::string shaderName = shaderRef->getNamePath();
                shaderName = MaterialX::createValidName(shaderName);
                xmlGenerator.generate(shaderName, shader.get(), nullptr, file);
            }
        }
    }
}
