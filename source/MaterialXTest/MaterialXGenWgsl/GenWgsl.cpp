//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXTest/MaterialXGenWgsl/GenWgsl.h>

#include <MaterialXGenWgsl/WgslShaderGenerator.h>
#include <MaterialXGenWgsl/WgslSyntax.h>
#include <MaterialXGenWgsl/WgslResourceBindingContext.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/XmlIo.h>

#include <MaterialXCore/Document.h>

#include <fstream>

namespace mx = MaterialX;

TEST_CASE("GenShader: WGSL Syntax Check", "[genwgsl]")
{
    mx::TypeSystemPtr typeSystem = mx::TypeSystem::create();
    mx::SyntaxPtr syntax = mx::WgslSyntax::create(typeSystem);

    REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "f32");
    REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "vec3f");
    REQUIRE(syntax->getTypeName(mx::Type::VECTOR4) == "vec4f");
    REQUIRE(syntax->getTypeName(mx::Type::MATRIX44) == "mat4x4f");
    REQUIRE(syntax->getSourceFileExtension() == ".wgsl");
}

TEST_CASE("GenShader: WGSL Target Registration", "[genwgsl]")
{
    mx::ShaderGeneratorPtr generator = mx::WgslShaderGenerator::create();
    REQUIRE(generator->getTarget() == "genwgsl");
}

TEST_CASE("GenShader: WGSL Shader Generation", "[genwgsl]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    mx::DocumentPtr doc = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, doc);

    mx::FilePath examplePath = searchPath.find("resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx");
    REQUIRE(!examplePath.isEmpty());
    mx::readFromXmlFile(doc, examplePath);

    mx::ShaderGeneratorPtr generator = mx::WgslShaderGenerator::create();
    mx::GenContext context(generator);
    context.registerSourceCodeSearchPath(searchPath);
    context.getOptions().hwSpecularEnvironmentMethod = mx::SPECULAR_ENVIRONMENT_FIS;
    context.getOptions().hwMaxActiveLightSources = 1;

    std::vector<mx::TypedElementPtr> renderables = mx::findRenderableElements(doc);
    REQUIRE(!renderables.empty());

    size_t generated = 0;
    for (const mx::TypedElementPtr& element : renderables)
    {
        const std::string name = mx::createValidName(element->getNamePath());

        mx::ShaderPtr shader;
        REQUIRE_NOTHROW(shader = generator->generate(name, element, context));
        REQUIRE(shader != nullptr);

        const std::string& vertexCode = shader->getSourceCode(mx::Stage::VERTEX);
        const std::string& pixelCode = shader->getSourceCode(mx::Stage::PIXEL);

        REQUIRE(!vertexCode.empty());
        REQUIRE(!pixelCode.empty());
        REQUIRE(vertexCode.find("@vertex") != std::string::npos);
        REQUIRE(vertexCode.find("fn vertexMain") != std::string::npos);
        REQUIRE(pixelCode.find("@fragment") != std::string::npos);
        REQUIRE(pixelCode.find("fn fragmentMain") != std::string::npos);

        std::ofstream(name + ".vertex.wgsl") << vertexCode;
        std::ofstream(name + ".pixel.wgsl") << pixelCode;
        generated++;
    }
    REQUIRE(generated > 0);
}

TEST_CASE("GenShader: WGSL TestSuite Generation", "[genwgsl]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();

    mx::FilePathVec testRootPaths;
    testRootPaths.push_back(searchPath.find("resources/Materials/TestSuite"));
    testRootPaths.push_back(searchPath.find("resources/Materials/Examples"));

    mx::ShaderGeneratorPtr generator = mx::WgslShaderGenerator::create();

    const mx::FilePath logPath("genwgsl_generate_test.txt");
    WgslShaderGeneratorTester tester(generator, testRootPaths, searchPath, logPath, false);

    tester.addUserData(mx::HW::USER_DATA_BINDING_CONTEXT,
                       mx::WgslResourceBindingContext::create(0));

    const mx::GenOptions genOptions;
    mx::FilePath optionsFilePath = searchPath.find("resources/Materials/TestSuite/_options.mtlx");
    tester.validate(genOptions, optionsFilePath);
}
