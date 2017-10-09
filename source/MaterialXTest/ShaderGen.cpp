#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXShaderGen/ShaderGenRegistry.h>
#include <MaterialXShaderGen/ShaderGenerators/ArnoldShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OgsFxShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/OslSyntax.h>
#include <MaterialXShaderGen/NodeImplementations/VDirection.h>
#include <MaterialXShaderGen/NodeImplementations/Swizzle.h>

#include <iostream>
#include <fstream>

namespace mx = MaterialX;

struct GeneratorDescription
{
    std::string _language;
    std::string _target;
    std::string _fileExt;
    std::vector<std::string> _implementationLibrary;
};

TEST_CASE("Registry", "[shadergen]")
{
    mx::ShaderGenRegistry::registerBuiltIn();

    mx::ShaderGeneratorPtr sg1 = mx::ShaderGenRegistry::findShaderGenerator(
        mx::ArnoldShaderGenerator::kLanguage,
        mx::ArnoldShaderGenerator::kTarget);
    REQUIRE(sg1->getTarget() == mx::ArnoldShaderGenerator::kTarget);
    
    mx::ShaderGeneratorPtr sg2 = mx::ShaderGenRegistry::findShaderGenerator(
        mx::ArnoldShaderGenerator::kLanguage);
    REQUIRE(sg2 == nullptr);

    mx::ShaderGenRegistry::unregisterShaderGenerator(
        mx::ArnoldShaderGenerator::kLanguage, 
        mx::ArnoldShaderGenerator::kTarget);

    sg1 = mx::ShaderGenRegistry::findShaderGenerator(
        mx::ArnoldShaderGenerator::kLanguage, 
        mx::ArnoldShaderGenerator::kTarget);
    REQUIRE(sg1 == nullptr);

    mx::NodeImplementationPtr impl1 = mx::ShaderGenRegistry::findNodeImplementation(
        mx::VDirectionFlipOsl::kNode,
        mx::VDirectionFlipOsl::kLanguage,
        mx::VDirectionFlipOsl::kTarget);
    REQUIRE(impl1 != nullptr);

    mx::NodeImplementationPtr impl2 = mx::ShaderGenRegistry::findNodeImplementation(
        mx::VDirectionFlipOsl::kNode,
        mx::VDirectionFlipOsl::kLanguage);
    REQUIRE(impl2 != nullptr);
    REQUIRE(impl2 == impl1);

    mx::NodeImplementationPtr impl3 = mx::ShaderGenRegistry::findNodeImplementation(
        mx::VDirectionFlipOsl::kNode);
    REQUIRE(impl3 == nullptr);

    mx::NodeImplementationPtr impl4 = mx::ShaderGenRegistry::findNodeImplementation(
        mx::VDirectionFlipGlsl::kNode,
        mx::VDirectionFlipGlsl::kLanguage,
        mx::VDirectionFlipGlsl::kTarget);
    REQUIRE(impl4 != nullptr);
    REQUIRE(impl4 != impl2);

    mx::ShaderGenRegistry::unregisterBuiltIn();

    impl4 = mx::ShaderGenRegistry::findNodeImplementation(
        mx::VDirectionFlipGlsl::kNode,
        mx::VDirectionFlipGlsl::kLanguage,
        mx::VDirectionFlipGlsl::kTarget);
    REQUIRE(impl4 == nullptr);
}

TEST_CASE("OslSyntax", "[shadergen]")
{
    mx::SyntaxPtr syntax = std::make_shared<mx::OslSyntax>();

    REQUIRE(syntax->getTypeName("float") == "float");
    REQUIRE(syntax->getTypeName("color3") == "color");
    REQUIRE(syntax->getTypeName("vector3") == "vector");

    REQUIRE(syntax->getTypeName("BSDF") == "BSDF");
    REQUIRE(syntax->getOutputTypeName("BSDF") == "output BSDF");

    std::string dv = syntax->getTypeDefault("float");
    REQUIRE(dv == "0.0");
    dv = syntax->getTypeDefault("color3", false);
    REQUIRE(dv == "color(0.0, 0.0, 0.0)");
    dv = syntax->getTypeDefault("color3", true);
    REQUIRE(dv == "color(0.0, 0.0, 0.0)");
}

TEST_CASE("Swizzling", "[shadergen]")
{
    mx::ScopedShaderGenInit shaderGenInit;

    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenRegistry::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx"
    };
    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    mx::ArnoldShaderGenerator sg;
    mx::SyntaxPtr syntax = sg.getSyntax();

    // Test swizzle syntax
    std::string var1 = syntax->getSwizzledVariable("foo", "color3", "color3", "bgr");
    REQUIRE(var1 == "color(foo[2], foo[1], foo[0])");
    std::string var2 = syntax->getSwizzledVariable("foo", "color4", "float", "rr01");
    REQUIRE(var2 == "pack(foo, foo, 0, 1)");
    std::string var3 = syntax->getSwizzledVariable("foo", "color2", "vector2", "xy");
    REQUIRE(var3 == "color2(foo.x, foo.y)");

    // Create a simple test graph
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("foo");
    mx::NodePtr add = nodeGraph->addNode("add", "", "color3");
    mx::NodePtr constant = nodeGraph->addNode("constant", "bar", "color3");
    mx::NodePtr swizzle = nodeGraph->addNode("swizzle", "swizzle", "color3");
    swizzle->setConnectedNode("in", constant);
    swizzle->setParameterValue("channels", std::string("rrr"));
    add->setConnectedNode("in1", swizzle);

    // Test swizzle node custom implementation
    mx::Swizzle swizzleNode;

    mx::Shader test1("test1");
    swizzleNode.emitFunctionCall(mx::SgNode(swizzle, sg.getLanguage(), sg.getTarget()), sg, test1);
    REQUIRE(test1.getSourceCode() == "color foo_swizzle = color(foo_bar[0], foo_bar[0], foo_bar[0]);\n");

    swizzle->setParameterValue("channels", std::string("b0b"));

    mx::Shader test2("test2");
    swizzleNode.emitFunctionCall(mx::SgNode(swizzle, sg.getLanguage(), sg.getTarget()), sg, test2);
    REQUIRE(test2.getSourceCode() == "color foo_swizzle = color(foo_bar[2], 0, foo_bar[2]);\n");
}

TEST_CASE("Simple Nodegraph Shader Generation", "[shadergen]")
{
    mx::ScopedShaderGenInit shaderGenInit;

    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenRegistry::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    // Create a simple node graph
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("test1");
    mx::NodePtr constant = nodeGraph->addNode("constant", "constant1", "color3");
    constant->setParameterValue("value", mx::Color3(1, 0, 0));
    mx::NodePtr image = nodeGraph->addNode("image", "image1", "color3");
    image->setParameterValue("file", std::string("/foo/bar.exr"), "filename");
    mx::NodePtr multiply = nodeGraph->addNode("multiply", "multiply1", "color3");
    multiply->setConnectedNode("in1", constant);
    multiply->setConnectedNode("in2", image);

    // Connected to output.
    mx::OutputPtr output1 = nodeGraph->addOutput();
    output1->setConnectedNode(multiply);

    // Write out a .dot file for visualization
    std::ofstream file;
    std::string dot = mx::printGraphDot(nodeGraph);
    file.open(nodeGraph->getName() + ".dot");
    file << dot;
    file.close();

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        // language,  target,    file ext,  implementation library
        { "osl",      "arnold",  "osl",     {"documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx"} },
        { "glsl",     "ogsfx",   "ogsfx",   {"documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx"} }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc._implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }

        // Find the shader generator
        mx::ShaderGeneratorPtr sg = mx::ShaderGenRegistry::findShaderGenerator(desc._language, desc._target);
        REQUIRE(sg != nullptr);

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = sg->generate(nodeGraph->getName(), output1);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "_graphoutput." + desc._fileExt);
        file << shader->getSourceCode();
        file.close();

        // Test shader generation from a node
        shader = sg->generate(nodeGraph->getName(), multiply);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "_node." + desc._fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}

TEST_CASE("Subgraph Shader Generation", "[shadergen]")
{
    mx::ScopedShaderGenInit shaderGenInit;

    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries/stdlib");
    mx::ShaderGenRegistry::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load example file
    searchPath += ";";
    searchPath += mx::FilePath::getCurrentPath() / mx::FilePath("documents/Examples");
    std::vector<std::string> filenames =
    {
        "SubGraphs.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename, searchPath, true);
    }

    // Get a node graph from the example file
    mx::NodeGraphPtr nodeGraph = doc->getNodeGraph("subgraph_ex1");
    mx::OutputPtr output = nodeGraph->getOutput("out");

    // Write out a .dot file for visualization
    std::ofstream file;
    std::string dot = mx::printGraphDot(nodeGraph);
    file.open(nodeGraph->getName() + ".dot");
    file << dot;
    file.close();

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        // language,  target,    file ext,  implementation library
        { "osl",      "arnold",  "osl",     { "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx" } },
        { "glsl",     "ogsfx",   "ogsfx",   { "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx" } }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc._implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }

        // Find the shader generator
        mx::ShaderGeneratorPtr sg = mx::ShaderGenRegistry::findShaderGenerator(desc._language, desc._target);
        REQUIRE(sg != nullptr);

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = sg->generate(nodeGraph->getName(), output);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Match against blessed versions
        file.open(shader->getName() + "." + desc._fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}

TEST_CASE("Material Shader Generation", "[shadergen]")
{
    mx::ScopedShaderGenInit shaderGenInit;

    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenRegistry::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/adsk/adsk_defs.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    // Create a node graph with the following structure:
    //
    // [image1] [constant]     
    //        \ /                    
    //    [multiply]          [image2]         [adskCellNoise2d]
    //             \____________  |  ____________/
    //                          [mix]
    //                            |
    //                         [output]
    //
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
    mx::NodePtr image1 = nodeGraph->addNode("image");
    image1->addParameter("file", "filename");
    mx::NodePtr image2 = nodeGraph->addNode("image");
    image2->addParameter("file", "filename");
    mx::NodePtr noise = nodeGraph->addNode("adskCellNoise2d", mx::EMPTY_STRING, "float");
    mx::NodePtr constant = nodeGraph->addNode("constant");
    mx::NodePtr multiply = nodeGraph->addNode("multiply");
    mx::NodePtr mix = nodeGraph->addNode("mix");
    mx::OutputPtr output = nodeGraph->addOutput();
    multiply->setConnectedNode("in1", image1);
    multiply->setConnectedNode("in2", constant);
    mix->setConnectedNode("fg", multiply);
    mix->setConnectedNode("bg", image2);
    mix->setConnectedNode("mask", noise);
    output->setConnectedNode(mix);

    // Create a material with a shader ref connecting to the graph
    mx::MaterialPtr material = doc->addMaterial();
    mx::ShaderRefPtr shaderRef = material->addShaderRef("adskSurface1", "adskSurface");
    mx::BindInputPtr base = shaderRef->addBindInput("base", "float");
    mx::BindInputPtr baseColor = shaderRef->addBindInput("base_color", "color3");
    base->setValue(0.8f);
    baseColor->setConnectedOutput(output);

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { "osl", "arnold", "osl",
            {
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_osl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/adsk_impl_shadergen_osl.mtlx"
            }
        },
        { "glsl", "ogsfx", "ogsfx",
            { 
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx",
                "documents/Libraries/adsk/impl/shadergen/adsk_impl_shadergen_glsl.mtlx"
            }
        }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc._implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }
        // Find the shader generator
        mx::ShaderGeneratorPtr sg = mx::ShaderGenRegistry::findShaderGenerator(desc._language, desc._target);
        REQUIRE(sg != nullptr);

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = sg->generate(shaderRef->getName(), shaderRef);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Match against blessed versions
        std::ofstream file;
        file.open(shader->getName() + "_shaderref." + desc._fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}

TEST_CASE("BSDF Test", "[shadergen]")
{
    mx::ScopedShaderGenInit shaderGenInit;

    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenRegistry::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/sx/sx_defs.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("BsdfTest");

    mx::NodePtr diffuse_tex = nodeGraph->addNode("image");
    mx::NodePtr sss         = nodeGraph->addNode("translucentbsdf", "sss", "BSDF");
    mx::NodePtr diffuse     = nodeGraph->addNode("diffusebsdf", "diffuse", "BSDF");
    mx::NodePtr mix         = nodeGraph->addNode("mixbsdf", "mix", "BSDF");
    mx::NodePtr specular    = nodeGraph->addNode("coatingbsdf", "specular", "BSDF");
    mx::OutputPtr output    = nodeGraph->addOutput("out", "BSDF");

    diffuse->setConnectedNode("reflectance", diffuse_tex);
    mix->setConnectedNode("fg", sss);
    mix->setConnectedNode("bg", diffuse);
    specular->setConnectedNode("base", mix);
    output->setConnectedNode(specular);

    mx::ParameterPtr diffuse_file = diffuse_tex->addParameter("file", "filename");
    diffuse_file->setPublicName("diffuse_file");
    diffuse_file->setValueString("");
    mx::ParameterPtr diffuse_color = diffuse_tex->addParameter("default", "color3");
    diffuse_color->setPublicName("diffuse_color");
    diffuse_color->setValueString("0.8, 0.8, 0.8");

    mx::InputPtr sss_color = sss->addInput("transmittance", "color3");
    sss_color->setPublicName("sss_color");
    sss_color->setValueString("0.3, 0.3, 0.3");

    mx::InputPtr sss_weight = mix->addInput("mask", "float");
    sss_weight->setPublicName("sss_weight");
    sss_weight->setValueString("0.0");

    mx::InputPtr specular_color = specular->addInput("reflectance", "color3");
    specular_color->setPublicName("specular_color");
    specular_color->setValueString("1.0, 1.0, 1.0");

    mx::InputPtr specular_roughness = specular->addInput("roughness", "float");
    specular_roughness->setPublicName("specular_roughness");
    specular_roughness->setValueString("0.2");

    mx::InputPtr specular_ior = specular->addInput("ior", "float");
    specular_ior->setPublicName("specular_ior");
    specular_ior->setValueString("1.52");

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { "glsl", "ogsfx", "ogsfx",
            {
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx",
                "documents/Libraries/sx/impl/shadergen/sx_impl_shadergen_glsl.mtlx"
            }
        }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc._implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }
        // Find the shader generator
        mx::ShaderGeneratorPtr sg = mx::ShaderGenRegistry::findShaderGenerator(desc._language, desc._target);
        REQUIRE(sg != nullptr);

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = sg->generate(nodeGraph->getName(), output);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Match against blessed versions
        std::ofstream file;
        file.open(shader->getName() + "." + desc._fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}

TEST_CASE("Surface Layer Test", "[shadergen]")
{
    mx::ScopedShaderGenInit shaderGenInit;

    std::string searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGenRegistry::registerSourceCodeSearchPath(searchPath);

    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/sx/sx_defs.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("SurfaceLayerTest");
    mx::NodePtr image1 = nodeGraph->addNode("image");
    image1->addParameter("file", "filename");
    mx::NodePtr image2 = nodeGraph->addNode("image");
    image2->addParameter("file", "filename");
    mx::NodePtr image3 = nodeGraph->addNode("image");
    image3->addParameter("file", "filename");

    mx::NodePtr layer1_diffuse  = nodeGraph->addNode("diffusebsdf", "layer1_diffuse", "BSDF");
    mx::NodePtr layer1_specular = nodeGraph->addNode("coatingbsdf", "layer1_specular", "BSDF");
    mx::NodePtr layer1          = nodeGraph->addNode("surfacelayer", "layer1", "surfaceshader");
    layer1_specular->setConnectedNode("base", layer1_diffuse);
    layer1->setConnectedNode("bsdf", layer1_specular);

    mx::NodePtr layer2_diffuse  = nodeGraph->addNode("diffusebsdf", "layer2_diffuse", "BSDF");
    mx::NodePtr layer2_emission = nodeGraph->addNode("uniformedf", "layer2_emission", "EDF");
    mx::NodePtr layer2_mask     = nodeGraph->addNode("image");
    mx::NodePtr layer2          = nodeGraph->addNode("surfacelayer", "layer2", "surfaceshader");
    layer2->setConnectedNode("bsdf", layer2_diffuse);
    layer2->setConnectedNode("edf", layer2_emission);
    layer2->setConnectedNode("opacity", layer2_mask);
    layer2->setConnectedNode("base", layer1);

    mx::OutputPtr output = nodeGraph->addOutput("out", "surfaceshader");
    output->setConnectedNode(layer2);

    mx::InputPtr layer1_diffuse_ = layer1_diffuse->addInput("reflectance", "color3");
    layer1_diffuse_->setPublicName("layer1_diffuse");
    layer1_diffuse_->setValueString("0.2, 0.9, 0.2");

    mx::InputPtr layer1_specular_ = layer1_specular->addInput("reflectance", "color3");
    layer1_specular_->setPublicName("layer1_specular");
    layer1_specular_->setValueString("1.0, 1.0, 1.0");

    mx::InputPtr layer2_diffuse_ = layer2_diffuse->addInput("reflectance", "color3");
    layer2_diffuse_->setPublicName("layer2_diffuse");
    layer2_diffuse_->setValueString("0.2, 0.2, 0.9");

    mx::InputPtr layer2_emission_ = layer2_emission->addInput("intensity", "color3");
    layer2_emission_->setPublicName("layer2_emission");
    layer2_emission_->setValueString("0.0, 0.0, 0.0");

    mx::ParameterPtr layer2_mask_file = layer2_mask->addParameter("file", "filename");
    layer2_mask_file->setPublicName("layer2_mask");
    layer2_mask_file->setValueString("");

    // Setup the shader generators
    std::vector<GeneratorDescription> generatorDescriptions =
    {
        { "glsl", "ogsfx", "ogsfx",
            {
                "documents/Libraries/stdlib/impl/shadergen/mx_stdlib_impl_shadergen_glsl.mtlx",
                "documents/Libraries/sx/impl/shadergen/sx_impl_shadergen_glsl.mtlx"
            }
        }
    };

    for (auto desc : generatorDescriptions)
    {
        // Load in the implementation libraries
        for (const std::string& libfile : desc._implementationLibrary)
        {
            mx::readFromXmlFile(doc, libfile);
        }
        // Find the shader generator
        mx::ShaderGeneratorPtr sg = mx::ShaderGenRegistry::findShaderGenerator(desc._language, desc._target);
        REQUIRE(sg != nullptr);

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = sg->generate(nodeGraph->getName(), output);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Match against blessed versions
        std::ofstream file;
        file.open(shader->getName() + "." + desc._fileExt);
        file << shader->getSourceCode();
        file.close();
    }
}
