#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Observer.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslSyntax.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/OgsFxShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/OgsFx/OgsFxSyntax.h>
#include <MaterialXShaderGen/ShaderGenerators/Osl/Arnold/ArnoldShaderGenerator.h>
#include <MaterialXShaderGen/ShaderGenerators/Osl/OslSyntax.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Swizzle.h>
#include <MaterialXShaderGen/TypeDesc.h>
#include <MaterialXShaderGen/Util.h>
#include <MaterialXShaderGen/HwShader.h>
#include <MaterialXShaderGen/HwLightHandler.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>

namespace mx = MaterialX;

void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc)
{
    for (const std::string& library : libraryNames)
    {
        mx::FilePath path = searchPath / library;
        mx::StringVec filenames;
        mx::getDocumentsInDirectory(path, filenames);

        for (const std::string& filename : filenames)
        {
            mx::FilePath file = path / filename;
            mx::readFromXmlFile(doc, file);
        }
    }
    REQUIRE(doc->getNodeDefs().size() > 0);
}

void loadExamples(const mx::StringVec& exampleNames, const mx::FilePath& examplesPath, const mx::FilePath searchPath, mx::DocumentPtr doc)
{
    try
    {
        for (const std::string& filename : exampleNames)
        {
            mx::FilePath file = examplesPath / filename;
            mx::readFromXmlFile(doc, file, searchPath);
        }
    }
    catch (mx::Exception e)
    {
    }
}


//
// Get source content, source path and resolved paths for
// an implementation
//
bool getShaderSource(mx::ShaderGeneratorPtr generator,
                    const mx::ImplementationPtr implementation,
                    mx::FilePath& sourcePath,
                    mx::FilePath& resolvedPath,
                    std::string& sourceContents) 
{
    if (implementation)
    {
        sourcePath = implementation->getFile();
        resolvedPath = generator->findSourceCode(sourcePath);
        if (mx::readFile(resolvedPath.asString(), sourceContents))
        {
            return true;
        }
    }
    return false;
}

// Check if a nodedef requires an implementation check
// Untyped nodes do not
bool requiresImplementation(const mx::NodeDefPtr nodeDef) 
{
    if (!nodeDef)
    {
        return false;
    }
    static std::string TYPE_NONE("none");
    const std::string typeAttribute = nodeDef->getAttribute(mx::TypedElement::TYPE_ATTRIBUTE);
    return !typeAttribute.empty() && typeAttribute != TYPE_NONE;
}

void createLightCompoundExample(mx::DocumentPtr document)
{
    const std::string nodeName = "lightcompound";
    const std::string nodeDefName = "ND_" + nodeName;

    // Make sure it doesn't exists already
    if (!document->getNodeDef(nodeDefName))
    {
        // Create an interface for the light with position, color and intensity
        mx::NodeDefPtr nodeDef = document->addNodeDef(nodeDefName, "lightshader", nodeName);
        nodeDef->addInput("position", "vector3");
        nodeDef->addInput("color", "color3");
        nodeDef->addInput("intensity", "float");

        // Create a graph implementing the light using EDF's
        mx::NodeGraphPtr nodeGraph = document->addNodeGraph("IMP_" + nodeName);
        mx::OutputPtr output = nodeGraph->addOutput("out", "lightshader");

        // Add EDF node and connect the EDF's intensity to the 'color' input
        mx::NodePtr edf = nodeGraph->addNode("uniformedf", "edf1", "EDF");
        mx::InputPtr edf_intensity = edf->addInput("intensity", "color3");
        edf_intensity->setInterfaceName("color");

        // Add the light constructor node connect it's intensity to the 'intensity' input
        mx::NodePtr light = nodeGraph->addNode("light", "light1", "lightshader");
        mx::InputPtr light_intensity = light->addInput("intensity", "float");
        light_intensity->setInterfaceName("intensity");

        // Connect the EDF to the light constructor
        light->setConnectedNode("edf", edf);

        // Connect the light to the graph output
        output->setConnectedNode(light);

        // Make this graph become the implementation of our nodedef
        nodeGraph->setAttribute("nodedef", nodeDef->getName());
    }
}

void createExampleMaterials(mx::DocumentPtr doc, std::vector<mx::MaterialPtr>& materials)
{
    // Example1: Create a material from 'standard_surface' with support for normal mapping
    {
        mx::MaterialPtr material = doc->addMaterial("example1");
        mx::ShaderRefPtr shaderRef = material->addShaderRef("example1_surface", "standard_surface");

        // Create nodes to handle normal mapping and bind it to the shader's normal input
        mx::NodePtr texcoord1 = doc->addNode("texcoord", "texcoord1", "vector2");
        mx::NodePtr uvscale = doc->addNode("multiply", "uvscale", "vector2");
        uvscale->setConnectedNode("in1", texcoord1);
        uvscale->setInputValue("in2", mx::Vector2(2.0, 2.0));

        mx::NodePtr normalTex = doc->addNode("image", "normalTex", "vector3");
        normalTex->setParameterValue("file", std::string(""), "filename");
        normalTex->setParameterValue("default", mx::Vector3(0.5f, 0.5f, 1.0f));
        normalTex->setConnectedNode("texcoord", uvscale);
        mx::NodePtr normalMap1 = doc->addNode("normalmap", "normalmap1", "vector3");
        normalMap1->setConnectedNode("in", normalTex);

        mx::OutputPtr outNormal = doc->addOutput("outNormal", "vector3");
        outNormal->setConnectedNode(normalMap1);

        mx::BindInputPtr normalInput = shaderRef->addBindInput("normal", "vector3");
        normalInput->setConnectedOutput(outNormal);

        // Bind a couple of other shader parameter values
        mx::BindInputPtr specular_roughness_input = shaderRef->addBindInput("specular_roughness", "float");
        specular_roughness_input->setValue(0.2f);
        mx::BindInputPtr specular_IOR_input = shaderRef->addBindInput("specular_IOR", "float");
        specular_IOR_input->setValue(2.0f);
    }

    // Example2: Create a surface shader by a graph of BSDF nodes with diffuse + specular
    {
        // Create a nodedef interface for the surface shader
        mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_testshader2", "surfaceshader", "testshader2");
        nodeDef->addInput("diffuse_reflectance", "color3");
        nodeDef->addInput("diffuse_roughness", "float");
        nodeDef->addInput("specular_reflectance", "color3");
        nodeDef->addInput("specular_roughness", "float");
        nodeDef->addInput("specular_anisotropy", "float");
        nodeDef->addInput("specular_IOR", "float");

        // Create the shader graph implementing the surface shader
        mx::NodeGraphPtr shaderGraph = doc->addNodeGraph("IMP_testshader2");

        // Get a normal facing our view direction
        mx::NodePtr shadingnormal = shaderGraph->addNode("shadingnormal", "shadingnormal1", "vector3");

        // A diffuse lobe
        mx::NodePtr diffuse = shaderGraph->addNode("diffusebsdf", "diffuse", "BSDF");
        diffuse->setConnectedNode("normal", shadingnormal);
        mx::InputPtr diffuse_reflectance = diffuse->addInput("reflectance", "color3");
        diffuse_reflectance->setInterfaceName("diffuse_reflectance");
        mx::InputPtr diffuse_roughness = diffuse->addInput("roughness", "float");
        diffuse_roughness->setInterfaceName("diffuse_roughness");

        // A specular lobe
        mx::NodePtr specular = shaderGraph->addNode("coatingbsdf", "coating", "BSDF");
        specular->setConnectedNode("normal", shadingnormal);
        specular->setConnectedNode("base", diffuse);
        mx::InputPtr specular_reflectance = specular->addInput("reflectance", "color3");
        specular_reflectance->setInterfaceName("specular_reflectance");
        mx::InputPtr specular_roughness = specular->addInput("roughness", "float");
        specular_roughness->setInterfaceName("specular_roughness");
        mx::InputPtr specular_anisotropy = specular->addInput("anisotropy", "float");
        specular_anisotropy->setInterfaceName("specular_anisotropy");
        mx::InputPtr specular_IOR = specular->addInput("ior", "float");
        specular_IOR->setInterfaceName("specular_IOR");

        // Create a surface shader construction node and connect the final BSDF
        mx::NodePtr surface = shaderGraph->addNode("surface", "surface1", "surfaceshader");
        surface->setConnectedNode("bsdf", specular);

        // Connect it as the graph output
        mx::OutputPtr output = shaderGraph->addOutput("out", "surfaceshader");
        output->setConnectedNode(surface);

        // Set this graph to be the implementation of the shader nodedef
        shaderGraph->setAttribute("nodedef", nodeDef->getName());

        // Create a material with the above shader node as the shader ref
        mx::MaterialPtr material = doc->addMaterial("example2");
        mx::ShaderRefPtr shaderRef = material->addShaderRef("example2_surface", "testshader2");

        // Bind a couple of shader parameter values
        mx::BindInputPtr diffuse_reflectance_input = shaderRef->addBindInput("diffuse_reflectance", "color3");
        diffuse_reflectance_input->setValue(mx::Color3(0.8f, 0.2f, 0.8f));
        mx::BindInputPtr specular_reflectance_input = shaderRef->addBindInput("specular_reflectance", "color3");
        specular_reflectance_input->setValue(mx::Color3(1.0f, 1.0f, 1.0f));
        mx::BindInputPtr specular_roughness_input = shaderRef->addBindInput("specular_roughness", "float");
        specular_roughness_input->setValue(0.2f);
        mx::BindInputPtr specular_IOR_input = shaderRef->addBindInput("specular_IOR", "float");
        specular_IOR_input->setValue(2.0f);
    }

    // Example3: Create a metal surface shader by a graph, using both the complex and
    // the artistic refraction index description, to test that they give equal results.
    {
        // Create a nodedef interface for the surface shader
        mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_testshader3", "surfaceshader", "testshader3");
        nodeDef->addInput("reflectivity", "color3");
        nodeDef->addInput("edgetint", "color3");
        nodeDef->addInput("ior_n", "vector3");
        nodeDef->addInput("ior_k", "vector3");
        nodeDef->addInput("artistic_vs_complex", "float");
        nodeDef->addInput("roughness", "float");
        nodeDef->addInput("anisotropy", "float");

        // Create the shader graph implementing the surface shader
        mx::NodeGraphPtr shaderGraph = doc->addNodeGraph("IMP_testshader3");

        // Get a normal facing our view direction
        mx::NodePtr shadingnormal = shaderGraph->addNode("shadingnormal", "shadingnormal1", "vector3");

        // A metal lobe with artistic fresnel
        mx::NodePtr metal1 = shaderGraph->addNode("metalbsdf", "metal1", "BSDF");
        metal1->setConnectedNode("normal", shadingnormal);
        mx::InputPtr reflectivity = metal1->addInput("reflectivity", "color3");
        reflectivity->setInterfaceName("reflectivity");
        mx::InputPtr edgetint = metal1->addInput("edgetint", "color3");
        edgetint->setInterfaceName("edgetint");
        mx::InputPtr roughness1 = metal1->addInput("roughness", "float");
        roughness1->setInterfaceName("roughness");
        mx::InputPtr anisotropy1 = metal1->addInput("anisotropy", "float");
        anisotropy1->setInterfaceName("anisotropy");

        // A metal lobe with complex fresnel
        mx::NodePtr metal2 = shaderGraph->addNode("metalbsdf", "metal2", "BSDF");
        mx::NodePtr reflectivity1 = shaderGraph->addNode("reflectivity", "reflectivity1", "color3");
        mx::InputPtr reflectivity1_ior_n = reflectivity1->addInput("ior_n", "vector3");
        reflectivity1_ior_n->setInterfaceName("ior_n");
        mx::InputPtr reflectivity1_ior_k = reflectivity1->addInput("ior_k", "vector3");
        reflectivity1_ior_k->setInterfaceName("ior_k");
        mx::NodePtr edgetint1 = shaderGraph->addNode("edgetint", "edgetint1", "color3");
        mx::InputPtr edgetint1_ior_n = edgetint1->addInput("ior_n", "vector3");
        edgetint1_ior_n->setInterfaceName("ior_n");
        edgetint1->setConnectedNode("reflectivity", reflectivity1);
        metal2->setConnectedNode("normal", shadingnormal);
        metal2->setConnectedNode("reflectivity", reflectivity1);
        metal2->setConnectedNode("edgetint", edgetint1);
        mx::InputPtr roughness2 = metal2->addInput("roughness", "float");
        roughness2->setInterfaceName("roughness");
        mx::InputPtr anisotropy2 = metal2->addInput("anisotropy", "float");
        anisotropy2->setInterfaceName("anisotropy");

        mx::NodePtr mix = shaderGraph->addNode("mixbsdf", "mix", "BSDF");
        mx::InputPtr weight = mix->addInput("weight", "float");
        weight->setInterfaceName("artistic_vs_complex");
        mix->setConnectedNode("in1", metal2);
        mix->setConnectedNode("in2", metal1);
        
        // Create a surface shader construction node and connect the final BSDF
        mx::NodePtr surface = shaderGraph->addNode("surface", "surface1", "surfaceshader");
        surface->setConnectedNode("bsdf", mix);

        // Connect it as the graph output
        mx::OutputPtr output = shaderGraph->addOutput("out", "surfaceshader");
        output->setConnectedNode(surface);

        // Set this graph to be the implementation of the shader nodedef
        shaderGraph->setAttribute("nodedef", nodeDef->getName());

        // Create a material with the above shader node as the shader ref
        mx::MaterialPtr material = doc->addMaterial("example3");
        mx::ShaderRefPtr shaderRef = material->addShaderRef("example3_surface", "testshader3");

        // Bind values setting both reflectivity/edgetint and ior_n/ior_k to represent gold,
        // so both metals should give the same result.
        mx::BindInputPtr reflectivity_input = shaderRef->addBindInput("reflectivity", "color3");
        reflectivity_input->setValue(mx::Color3(0.944f, 0.776f, 0.373f));
        mx::BindInputPtr edgetint_input = shaderRef->addBindInput("edgetint", "color3");
        edgetint_input->setValue(mx::Color3(0.998f, 0.981f, 0.751f));
        mx::BindInputPtr ior_n_input = shaderRef->addBindInput("ior_n", "vector3");
        ior_n_input->setValue(mx::Vector3(0.183f, 0.422f, 1.373f));
        mx::BindInputPtr ior_k_input = shaderRef->addBindInput("ior_k", "vector3");
        ior_k_input->setValue(mx::Vector3(3.424f, 2.346f, 1.771f));
        mx::BindInputPtr roughness_input = shaderRef->addBindInput("roughness", "float");
        roughness_input->setValue(0.2f);
    }

    // Example4: Create a material from 'standard_surface' using <inputmap> nodes for mappable inputs
    {
        // Shader parameter listing (<name>, <type>, <mappable
        using StringPair = std::pair<std::string, std::string>;
        const std::vector< std::pair<StringPair, bool> > shaderParams =
        {
            { {"base", "float"}, false },
            { { "base_color", "color3" }, true },
            { { "diffuse_roughness", "float" }, false },
            { { "specular", "float" }, true },
            { { "specular_color", "color3" }, true },
            { { "specular_roughness", "float" }, true },
            { { "specular_IOR", "float" }, false },
            { { "specular_anisotropy", "float" }, false },
            { { "metalness","float" }, true },
            { { "transmission", "float" }, true },
            { { "transmission_color", "color3" }, true },
            { { "transmission_extra_roughness", "float" }, true },
            { { "subsurface", "float" }, false },
            { { "subsurface_color", "color3" }, false },
            { { "coat", "float" }, true },
            { { "coat_color", "color3" }, true },
            { { "coat_roughness", "float" }, true },
            { { "coat_IOR", "float" }, false },
        };

        mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_testshader4", "surfaceshader", "testshader4");
        mx::NodeGraphPtr shaderGraph = doc->addNodeGraph("IMP_testshader4");
        shaderGraph->setAttribute("nodedef", nodeDef->getName());

        mx::NodePtr standardSurface = shaderGraph->addNode("standard_surface", "standardsurface1", "surfaceshader");
        for (auto shaderParam : shaderParams)
        {
            const std::string& name = shaderParam.first.first;
            const std::string& type = shaderParam.first.second;
            const bool mappable = shaderParam.second;

            if (mappable)
            {
                // Texturable, add an input for both a value and an image map
                nodeDef->addInput(name, type);
                nodeDef->addInput(name + "_map", "filename");

                // Create the inputmap node and connect it to the shader interface
                mx::NodePtr texInput = shaderGraph->addNode("inputmap", name, type);
                mx::InputPtr file = texInput->addInput("file", "filename");
                file->setInterfaceName(name + "_map");
                mx::InputPtr value = texInput->addInput("value", type);
                value->setInterfaceName(name);

                // Connect it to the surface shader
                mx::InputPtr input = standardSurface->addInput(name, type);
                input->setConnectedNode(texInput);
            }
            else
            {
                // Non-Texturable, add a single value input
                nodeDef->addInput(name, type);
                mx::InputPtr input = standardSurface->addInput(name, type);
                input->setInterfaceName(name);
            }
        }

        nodeDef->addInput("normalmap", "filename");
        nodeDef->addInput("coat_normalmap", "filename");

        mx::NodePtr specularNormalTex = shaderGraph->addNode("image", "normalTex", "vector3");
        mx::ParameterPtr specularNormalTexFile = specularNormalTex->addParameter("file", "filename");
        specularNormalTexFile->setInterfaceName("normalmap");
        specularNormalTex->setParameterValue("default", mx::Vector3(0.5f, 0.5f, 1.0f));
        mx::NodePtr specularNormalMap = shaderGraph->addNode("normalmap", "normalmap1", "vector3");
        specularNormalMap->setConnectedNode("in", specularNormalTex);

        mx::NodePtr coatNormalTex = shaderGraph->addNode("image", "coatTex", "vector3");
        mx::ParameterPtr coatNormalTexFile = coatNormalTex->addParameter("file", "filename");
        coatNormalTexFile->setInterfaceName("coat_normalmap");
        coatNormalTex->setParameterValue("default", mx::Vector3(0.5f, 0.5f, 1.0f));
        mx::NodePtr coatNormalMap = shaderGraph->addNode("normalmap", "normalmap2", "vector3");
        coatNormalMap->setConnectedNode("in", coatNormalTex);

        standardSurface->setConnectedNode("normal", specularNormalMap);
        standardSurface->setConnectedNode("coat_normal", coatNormalMap);

        mx::OutputPtr output = shaderGraph->addOutput("out", "surfaceshader");
        output->setConnectedNode(standardSurface);

        // Create a material with the above shader node as the shader ref
        mx::MaterialPtr material = doc->addMaterial("example4");
        mx::ShaderRefPtr shaderRef = material->addShaderRef("example4_surface", "testshader4");

        // Bind a couple of shader parameter values
        mx::BindInputPtr base_input = shaderRef->addBindInput("base", "float");
        base_input->setValue(0.8f);
        mx::BindInputPtr base_color_input = shaderRef->addBindInput("base_color", "color3");
        base_color_input->setValue(mx::Color3(1.0f, 1.0f, 1.0f));
        mx::BindInputPtr specular_input = shaderRef->addBindInput("specular", "float");
        specular_input->setValue(1.0f);
        mx::BindInputPtr specular_color_input = shaderRef->addBindInput("specular_color", "color3");
        specular_color_input->setValue(mx::Color3(1.0f, 1.0f, 1.0f));
        mx::BindInputPtr specular_IOR_input = shaderRef->addBindInput("specular_IOR", "float");
        specular_IOR_input->setValue(1.52f);
        mx::BindInputPtr specular_roughness_input = shaderRef->addBindInput("specular_roughness", "float");
        specular_roughness_input->setValue(0.1f);
        mx::BindInputPtr coat_IOR_input = shaderRef->addBindInput("coat_IOR", "float");
        coat_IOR_input->setValue(3.0);
    }

    // Example5: Create a material directly from 'standard_surface'
    {
        mx::MaterialPtr material = doc->addMaterial("example5");
        mx::ShaderRefPtr shaderRef = material->addShaderRef("example5_surface", "standard_surface");

        // Bind a couple of shader parameter values
        mx::BindInputPtr specular_roughness_input = shaderRef->addBindInput("specular_roughness", "float");
        specular_roughness_input->setValue(0.123f);
        mx::BindInputPtr specular_IOR_input = shaderRef->addBindInput("specular_IOR", "float");
        specular_IOR_input->setValue(2.0f);
    }
    
    // Get all materials created
    for (const mx::MaterialPtr& material : doc->getMaterials())
    {
        materials.push_back(material);
    }

    mx::writeToXmlFile(doc, "example_materials.mtlx");
}

float cosAngle(float degrees)
{
    static const float PI = 3.14159265f;
    return cos(degrees * PI / 180.0f);
}

// Light type id's for common light shaders
// Using id's matching the OgsFx light sources
// here which simplifies light binding for OGS.
// Note that another target systems could use other ids
// as required by that system.
enum LightType
{
    SPOT = 2,
    POINT = 3,
    DIRECTIONAL = 4,
};

void createLightRig(mx::DocumentPtr doc, mx::HwLightHandler& lightHandler, mx::HwShaderGenerator& shadergen)
{
    // Create a custom light shader by a graph compound
    createLightCompoundExample(doc);

    mx::NodeDefPtr dirLightNodeDef = doc->getNodeDef("ND_directionallight");
    mx::NodeDefPtr pointLightNodeDef = doc->getNodeDef("ND_pointlight");
    mx::NodeDefPtr spotLightNodeDef = doc->getNodeDef("ND_spotlight");
    mx::NodeDefPtr compoundLightNodeDef = doc->getNodeDef("ND_lightcompound");
    REQUIRE(dirLightNodeDef != nullptr);
    REQUIRE(pointLightNodeDef != nullptr);
    REQUIRE(spotLightNodeDef != nullptr);
    REQUIRE(compoundLightNodeDef != nullptr);

    // Add the common light shaders
    lightHandler.addLightShader(LightType::DIRECTIONAL, dirLightNodeDef);
    lightHandler.addLightShader(LightType::POINT, pointLightNodeDef);
    lightHandler.addLightShader(LightType::SPOT, spotLightNodeDef);

    // Add our custom coumpund light shader
    const size_t compoundLightId = 42;
    lightHandler.addLightShader(compoundLightId, compoundLightNodeDef);

    // Create a light rig with one light source for each light shader

    mx::LightSourcePtr dirLight = lightHandler.createLightSource(LightType::DIRECTIONAL);
    dirLight->setParameter("direction", mx::Vector3(0, 0, -1));
    dirLight->setParameter("color", mx::Color3(1, 1, 1));
    dirLight->setParameter("intensity", 0.2f);
    
    mx::LightSourcePtr pointLight = lightHandler.createLightSource(LightType::POINT);
    pointLight->setParameter("position", mx::Vector3(-2, -2, 2));
    pointLight->setParameter("color", mx::Color3(0, 0.0, 1));
    pointLight->setParameter("intensity", 10.0f);
    pointLight->setParameter("decayRate", 3.0f);

    mx::LightSourcePtr spotLight = lightHandler.createLightSource(LightType::SPOT);
    mx::Vector3 position(3, 3, 3);
    spotLight->setParameter("position", position);
    mx::Vector3 direction = position.getNormalized() * -1;;
    spotLight->setParameter("direction", direction);
    spotLight->setParameter("color", mx::Color3(1, 0, 0));
    spotLight->setParameter("intensity", 1.0f);
    spotLight->setParameter("decayRate", 0.0f);
    spotLight->setParameter("innerConeAngle", cosAngle(5.0f));
    spotLight->setParameter("outerConeAngle", cosAngle(10.0f));

    mx::LightSourcePtr compoundLight = lightHandler.createLightSource(compoundLightId);
    position = { -3, 3, 3 };
    direction = position.getNormalized() * -1;
    compoundLight->setParameter("position", position);
    compoundLight->setParameter("direction", direction);
    compoundLight->setParameter("color", mx::Color3(0, 1, 0));
    compoundLight->setParameter("intensity", 10.0f);

    // Let the shader generator know of these light shaders
    lightHandler.bindLightShaders(shadergen);
}

TEST_CASE("Syntax", "[shadergen]")
{
    {
        mx::SyntaxPtr syntax = mx::OslSyntax::create();

        REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "float");
        REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "color");
        REQUIRE(syntax->getTypeName(mx::Type::VECTOR3) == "vector");

        REQUIRE(syntax->getTypeName(mx::Type::BSDF) == "BSDF");
        REQUIRE(syntax->getOutputTypeName(mx::Type::BSDF) == "output BSDF");

        // Set fixed precision with one digit
        mx::Value::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

        std::string value;
        value = syntax->getDefaultValue(mx::Type::FLOAT);
        REQUIRE(value == "0.0");
        value = syntax->getDefaultValue(mx::Type::COLOR3);
        REQUIRE(value == "color(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR3, true);
        REQUIRE(value == "color(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR4);
        REQUIRE(value == "color4(color(0.0), 0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR4, true);
        REQUIRE(value == "{color(0.0), 0.0}");

        mx::ValuePtr floatValue = mx::Value::createValue<float>(42.0f);
        value = syntax->getValue(mx::Type::FLOAT, *floatValue);
        REQUIRE(value == "42.0");
        value = syntax->getValue(mx::Type::FLOAT, *floatValue, true);
        REQUIRE(value == "42.0");

        mx::ValuePtr color3Value = mx::Value::createValue<mx::Color3>(mx::Color3(1.0f, 2.0f, 3.0f));
        value = syntax->getValue(mx::Type::COLOR3, *color3Value);
        REQUIRE(value == "color(1.0, 2.0, 3.0)");
        value = syntax->getValue(mx::Type::COLOR3, *color3Value, true);
        REQUIRE(value == "color(1.0, 2.0, 3.0)");

        mx::ValuePtr color4Value = mx::Value::createValue<mx::Color4>(mx::Color4(1.0f, 2.0f, 3.0f, 4.0f));
        value = syntax->getValue(mx::Type::COLOR4, *color4Value);
        REQUIRE(value == "color4(color(1.0, 2.0, 3.0), 4.0)");
        value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
        REQUIRE(value == "{color(1.0, 2.0, 3.0), 4.0}");
    }

    {
        mx::SyntaxPtr syntax = mx::GlslSyntax::create();

        REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "float");
        REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "vec3");
        REQUIRE(syntax->getTypeName(mx::Type::VECTOR3) == "vec3");

        REQUIRE(syntax->getTypeName(mx::Type::BSDF) == "BSDF");
        REQUIRE(syntax->getOutputTypeName(mx::Type::BSDF) == "out BSDF");

        // Set fixed precision with one digit
        mx::Value::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

        std::string value;
        value = syntax->getDefaultValue(mx::Type::FLOAT);
        REQUIRE(value == "0.0");
        value = syntax->getDefaultValue(mx::Type::COLOR3);
        REQUIRE(value == "vec3(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR3, true);
        REQUIRE(value == "vec3(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR4);
        REQUIRE(value == "vec4(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR4, true);
        REQUIRE(value == "vec4(0.0)");

        mx::ValuePtr floatValue = mx::Value::createValue<float>(42.0f);
        value = syntax->getValue(mx::Type::FLOAT, *floatValue);
        REQUIRE(value == "42.0");
        value = syntax->getValue(mx::Type::FLOAT, *floatValue, true);
        REQUIRE(value == "42.0");

        mx::ValuePtr color3Value = mx::Value::createValue<mx::Color3>(mx::Color3(1.0f, 2.0f, 3.0f));
        value = syntax->getValue(mx::Type::COLOR3, *color3Value);
        REQUIRE(value == "vec3(1.0, 2.0, 3.0)");
        value = syntax->getValue(mx::Type::COLOR3, *color3Value, true);
        REQUIRE(value == "vec3(1.0, 2.0, 3.0)");

        mx::ValuePtr color4Value = mx::Value::createValue<mx::Color4>(mx::Color4(1.0f, 2.0f, 3.0f, 4.0f));
        value = syntax->getValue(mx::Type::COLOR4, *color4Value);
        REQUIRE(value == "vec4(1.0, 2.0, 3.0, 4.0)");
        value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
        REQUIRE(value == "vec4(1.0, 2.0, 3.0, 4.0)");
    }

    {
        mx::SyntaxPtr syntax = mx::OgsFxSyntax::create();

        REQUIRE(syntax->getTypeName(mx::Type::FLOAT) == "float");
        REQUIRE(syntax->getTypeName(mx::Type::COLOR3) == "vec3");
        REQUIRE(syntax->getTypeName(mx::Type::VECTOR3) == "vec3");

        REQUIRE(syntax->getTypeName(mx::Type::BSDF) == "BSDF");
        REQUIRE(syntax->getOutputTypeName(mx::Type::BSDF) == "out BSDF");

        // Set fixed precision with one digit
        mx::Value::ScopedFloatFormatting format(mx::Value::FloatFormatFixed, 1);

        std::string value;
        value = syntax->getDefaultValue(mx::Type::FLOAT);
        REQUIRE(value == "0.0");
        value = syntax->getDefaultValue(mx::Type::COLOR3);
        REQUIRE(value == "vec3(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR3, true);
        REQUIRE(value == "{0.0, 0.0, 0.0}");
        value = syntax->getDefaultValue(mx::Type::COLOR4);
        REQUIRE(value == "vec4(0.0)");
        value = syntax->getDefaultValue(mx::Type::COLOR4, true);
        REQUIRE(value == "{0.0, 0.0, 0.0, 0.0}");

        mx::ValuePtr floatValue = mx::Value::createValue<float>(42.0f);
        value = syntax->getValue(mx::Type::FLOAT, *floatValue);
        REQUIRE(value == "42.0");
        value = syntax->getValue(mx::Type::FLOAT, *floatValue, true);
        REQUIRE(value == "42.0");

        mx::ValuePtr color3Value = mx::Value::createValue<mx::Color3>(mx::Color3(1.0f, 2.0f, 3.0f));
        value = syntax->getValue(mx::Type::COLOR3, *color3Value);
        REQUIRE(value == "vec3(1.0, 2.0, 3.0)");
        value = syntax->getValue(mx::Type::COLOR3, *color3Value, true);
        REQUIRE(value == "{1.0, 2.0, 3.0}");

        mx::ValuePtr color4Value = mx::Value::createValue<mx::Color4>(mx::Color4(1.0f, 2.0f, 3.0f, 4.0f));
        value = syntax->getValue(mx::Type::COLOR4, *color4Value);
        REQUIRE(value == "vec4(1.0, 2.0, 3.0, 4.0)");
        value = syntax->getValue(mx::Type::COLOR4, *color4Value, true);
        REQUIRE(value == "{1.0, 2.0, 3.0, 4.0}");
    }
}

TEST_CASE("TypeDesc", "[shadergen]")
{
    // Make sure the standard types are registered
    const mx::TypeDesc* floatType = mx::TypeDesc::get("float");
    REQUIRE(floatType != nullptr);
    REQUIRE(floatType->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    const mx::TypeDesc* integerType = mx::TypeDesc::get("integer");
    REQUIRE(integerType != nullptr);
    REQUIRE(integerType->getBaseType() == mx::TypeDesc::BASETYPE_INTEGER);
    const mx::TypeDesc* booleanType = mx::TypeDesc::get("boolean");
    REQUIRE(booleanType != nullptr);
    REQUIRE(booleanType->getBaseType() == mx::TypeDesc::BASETYPE_BOOLEAN);
    const mx::TypeDesc* color2Type = mx::TypeDesc::get("color2");
    REQUIRE(color2Type != nullptr);
    REQUIRE(color2Type->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color2Type->getSemantic() == mx::TypeDesc::SEMATIC_COLOR);
    REQUIRE(color2Type->isFloat2());
    const mx::TypeDesc* color3Type = mx::TypeDesc::get("color3");
    REQUIRE(color3Type != nullptr);
    REQUIRE(color3Type->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color3Type->getSemantic() == mx::TypeDesc::SEMATIC_COLOR);
    REQUIRE(color3Type->isFloat3());
    const mx::TypeDesc* color4Type = mx::TypeDesc::get("color4");
    REQUIRE(color4Type != nullptr);
    REQUIRE(color4Type->getBaseType() == mx::TypeDesc::BASETYPE_FLOAT);
    REQUIRE(color4Type->getSemantic() == mx::TypeDesc::SEMATIC_COLOR);
    REQUIRE(color4Type->isFloat4());

    // Make sure we can register a new sutom type
    const mx::TypeDesc* fooType = mx::TypeDesc::registerType("foo", mx::TypeDesc::BASETYPE_FLOAT, mx::TypeDesc::SEMATIC_COLOR, 5);
    REQUIRE(fooType != nullptr);

    // Make sure we can't use a name already take
    REQUIRE_THROWS(mx::TypeDesc::registerType("color3", mx::TypeDesc::BASETYPE_FLOAT));

    // Make sure we can't request an unknown type
    REQUIRE_THROWS(mx::TypeDesc::get("bar"));
}

TEST_CASE("Reference Implementation Validity", "[shadergen]")
{   
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    // Set source code search path
    mx::FileSearchPath sourceCodeSearchPath;
    sourceCodeSearchPath.append(searchPath);

    std::filebuf implDumpBuffer;
    std::string fileName = "reference_implementation_check.txt";
    implDumpBuffer.open(fileName, std::ios::out);
    std::ostream implDumpStream(&implDumpBuffer);

    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    implDumpStream << "Scanning language: osl. Target: reference" << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;

    const std::string language("osl");
    const std::string target("");

    std::vector<mx::ImplementationPtr> impls = doc->getImplementations();
    implDumpStream << "Existing implementations: " << std::to_string(impls.size()) << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    for (auto impl : impls)
    {
        if (language == impl->getLanguage() && impl->getTarget().empty())
        {
            std::string msg("Impl: ");
            msg += impl->getName();

            mx::NodeDefPtr nodedef = impl->getNodeDef();
            if (!nodedef)
            {
                std::string nodedefName = impl->getNodeDefString();
                msg += ". Does NOT have a nodedef with name: " + nodedefName;
            }
            implDumpStream << msg << std::endl;
        }
    }

    std::string nodeDefNode;
    std::string nodeDefType;
    unsigned int count = 0;
    unsigned int missing = 0;
    std::string missing_str;
    std::string found_str;

    // Scan through every nodedef defined
    for (mx::NodeDefPtr nodeDef : doc->getNodeDefs())
    {
        count++;

        std::string nodeDefName = nodeDef->getName();
        std::string nodeName = nodeDef->getNodeString();
        if (!requiresImplementation(nodeDef))
        {
            found_str += "No implementation required for nodedef: " + nodeDefName + ", Node: " + nodeName + ".\n";
            continue;
        }

        mx::InterfaceElementPtr inter = nodeDef->getImplementation(target, language);
        if (!inter)
        {
            missing++;
            missing_str += "Missing nodeDef implemenation: " + nodeDefName + ", Node: " + nodeName + ".\n";
        }
        else
        {
            mx::ImplementationPtr impl = inter->asA<mx::Implementation>();
            if (impl)
            {
                // Scan for file and see if we can read in the contents
                std::string sourceContents;
                mx::FilePath sourcePath = impl->getFile();
                mx::FilePath resolvedPath = sourceCodeSearchPath.find(sourcePath);
                bool found = mx::readFile(resolvedPath.asString(), sourceContents);
                if (!found)
                {
                    missing++;
                    missing_str += "Missing source code: " + sourcePath.asString() + " for nodeDef: "
                        + nodeDefName + ". Impl: " + impl->getName() + ".\n";
                }
                else
                {
                    found_str += "Found impl and src for nodedef: " + nodeDefName + ", Node: "
                        + nodeName + ". Impl: " + impl->getName() + ".\n";
                }
            }
            else
            {
                mx::NodeGraphPtr graph = inter->asA<mx::NodeGraph>();
                found_str += "Found NodeGraph impl for nodedef: " + nodeDefName + ", Node: "
                    + nodeName + ". Impl: " + graph->getName() + ".\n";
            }
        }
    }

    implDumpStream << "-----------------------------------------------------------------------" << std::endl;
    implDumpStream << "Missing: " << missing << " implementations out of: " << count << " nodedefs\n";
    implDumpStream << missing_str << std::endl;
    implDumpStream << found_str << std::endl;
    implDumpStream << "-----------------------------------------------------------------------" << std::endl;

    implDumpBuffer.close();

    // To enable once this is true
    //REQUIRE(missing == 0);
}

TEST_CASE("ShaderX Implementation Validity", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    std::vector<mx::ShaderGeneratorPtr> shaderGenerators =
    {
        mx::ArnoldShaderGenerator::create(),
        mx::OgsFxShaderGenerator::create(),
        mx::GlslShaderGenerator::create()
    };

    std::filebuf implDumpBuffer;
    std::string fileName = "shadgen_implementation_check.txt";
    implDumpBuffer.open(fileName, std::ios::out);
    std::ostream implDumpStream(&implDumpBuffer);

    for (auto generator : shaderGenerators)
    {
        generator->registerSourceCodeSearchPath(searchPath);

        const std::string& language = generator->getLanguage();
        const std::string& target = generator->getTarget();

        // Node types to explicitly skip temporarily.
        std::set<std::string> skipNodeTypes = 
        {
            "ambientocclusion",
            "arrayappend",
            "blur",
            "curveadjust",
        };

        // Skip light types in OSL for now
        if (language == mx::OslShaderGenerator::LANGUAGE)
        {
            skipNodeTypes.insert("light");
            skipNodeTypes.insert("pointlight");
            skipNodeTypes.insert("directionallight");
            skipNodeTypes.insert("spotlight");
        }

        // Explicit set of node defs to skip temporarily
        std::set<std::string> skipNodeDefs =
        {
            "ND_add_displacementshader",
            "ND_add_volumeshader",
            "ND_multiply_displacementshaderF",
            "ND_multiply_displacementshaderV",
            "ND_multiply_volumeshaderF",
            "ND_multiply_volumeshaderC",
            "ND_mix_displacementshader",
            "ND_mix_volumeshader"
        };
        // Skip some shader math in GLSL for now
        if (language == mx::GlslShaderGenerator::LANGUAGE)
        {
            skipNodeDefs.insert("ND_add_surfaceshader");
            skipNodeDefs.insert("ND_multiply_surfaceshaderF");
            skipNodeDefs.insert("ND_multiply_surfaceshaderC");
            skipNodeDefs.insert("ND_mix_surfaceshader");
        }

        implDumpStream << "-----------------------------------------------------------------------" << std::endl;
        implDumpStream << "Scanning language: " << language << ". Target: " << target << std::endl;
        implDumpStream << "-----------------------------------------------------------------------" << std::endl;

        std::vector<mx::ImplementationPtr> impls = doc->getImplementations();
        implDumpStream << "Existing implementations: " << std::to_string(impls.size()) << std::endl;
        implDumpStream << "-----------------------------------------------------------------------" << std::endl;
        for (auto impl : impls)
        {
            if (language == impl->getLanguage())
            {
                std::string msg("Impl: ");
                msg += impl->getName();
                std::string targetName = impl->getTarget();
                if (targetName.size())
                {
                    msg += ", target: " + targetName;
                }
                mx::NodeDefPtr nodedef = impl->getNodeDef();
                if (!nodedef)
                {
                    std::string nodedefName = impl->getNodeDefString();
                    msg += ". Does NOT have a nodedef with name: " + nodedefName;
                }
                implDumpStream << msg << std::endl;
            }
        }

        std::string nodeDefNode;
        std::string nodeDefType;
        unsigned int count = 0;
        unsigned int missing = 0;
        unsigned int skipped = 0;
        std::string missing_str;
        std::string found_str;

        // Scan through every nodedef defined
        for (mx::NodeDefPtr nodeDef : doc->getNodeDefs())
        {
            count++;

            const std::string& nodeDefName = nodeDef->getName();
            const std::string& nodeName = nodeDef->getNodeString();

            if (skipNodeTypes.count(nodeName))
            {
                found_str += "Temporarily skipping implementation required for nodedef: " + nodeDefName + ", Node : " + nodeName + ".\n";
                skipped++;
                continue;
            }
            if (skipNodeDefs.count(nodeDefName))
            {
                found_str += "Temporarily skipping implementation required for nodedef: " + nodeDefName + ", Node : " + nodeName + ".\n";
                skipped++;
                continue;
            }

            if (!requiresImplementation(nodeDef))
            {
                found_str += "No implementation required for nodedef: " + nodeDefName + ", Node: " + nodeName + ".\n";
                continue;
            }

            mx::InterfaceElementPtr inter = nodeDef->getImplementation(target, language);
            if (!inter)
            {
                missing++;
                missing_str += "Missing nodeDef implementation: " + nodeDefName + ", Node: " + nodeName + ".\n";

                std::vector<mx::InterfaceElementPtr> inters = doc->getMatchingImplementations(nodeDefName);
                for (auto inter2 : inters)
                {
                    mx::ImplementationPtr impl = inter2->asA<mx::Implementation>();
                    if (impl)
                    {
                        std::string msg("\t Cached Impl: ");
                        msg += impl->getName();
                        msg += ", nodedef: " + impl->getNodeDefString();
                        msg += ", target: " + impl->getTarget();
                        msg += ", language: " + impl->getLanguage();
                        missing_str += msg + ".\n";
                    }
                }

                for (auto childImpl : impls)
                {
                    if (childImpl->getNodeDefString() == nodeDefName)
                    {
                        std::string msg("\t Doc Impl: ");
                        msg += childImpl->getName();
                        msg += ", nodedef: " + childImpl->getNodeDefString();
                        msg += ", target: " + childImpl->getTarget();
                        msg += ", language: " + childImpl->getLanguage();
                        missing_str += msg + ".\n";
                    }
                }

            }
            else
            {
                mx::ImplementationPtr impl = inter->asA<mx::Implementation>();
                if (impl)
                {
                    // Test if the generator has an interal implementation first
                    if (generator->implementationRegistered(impl->getName()))
                    {
                        found_str += "Found generator impl for nodedef: " + nodeDefName + ", Node: "
                            + nodeDefName + ". Impl: " + impl->getName() + ".\n";
                    }

                    // Check for an implementation explicitly stored
                    else
                    {
                        mx::FilePath sourcePath, resolvedPath;
                        std::string contents;
                        if (!getShaderSource(generator, impl, sourcePath, resolvedPath, contents))
                        {
                            missing++;
                            missing_str += "Missing source code: " + sourcePath.asString() + " for nodeDef: "
                                + nodeDefName + ". Impl: " + impl->getName() + ".\n";
                        }
                        else
                        {
                            found_str += "Found impl and src for nodedef: " + nodeDefName + ", Node: "
                                + nodeName + +". Impl: " + impl->getName() + ".\n";
                        }
                    }
                }
                else
                {
                    mx::NodeGraphPtr graph = inter->asA<mx::NodeGraph>();
                    found_str += "Found NodeGraph impl for nodedef: " + nodeDefName + ", Node: "
                        + nodeName + ". Impl: " + graph->getName() + ".\n";
                }
            }
        }

        implDumpStream << "-----------------------------------------------------------------------" << std::endl;
        implDumpStream << "Missing: " << missing << " implementations out of: " << count << " nodedefs. Skipped: " << skipped << std::endl;
        implDumpStream << missing_str << std::endl;
        implDumpStream << found_str << std::endl;
        implDumpStream << "-----------------------------------------------------------------------" << std::endl;

        // Should have 0 missing including skipped
        REQUIRE(missing == 0);
        REQUIRE(skipped == 45);
    }

    implDumpBuffer.close();
}

TEST_CASE("Swizzling", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({"stdlib"}, searchPath, doc);

    mx::SgOptions options;
    mx::SgNodeContext context(mx::ShaderGenerator::NODE_CONTEXT_DEFAULT);

    {
        mx::ArnoldShaderGenerator sg;
        sg.registerSourceCodeSearchPath(searchPath);
        const mx::Syntax* syntax = sg.getSyntax();

        // Test swizzle syntax
        std::string var1 = syntax->getSwizzledVariable("foo", mx::Type::COLOR3, "bgr", mx::Type::COLOR3);
        REQUIRE(var1 == "color(foo[2], foo[1], foo[0])");
        std::string var2 = syntax->getSwizzledVariable("foo", mx::Type::VECTOR2, "xy", mx::Type::COLOR2);
        REQUIRE(var2 == "color2(foo.x, foo.y)");
        std::string var3 = syntax->getSwizzledVariable("foo", mx::Type::FLOAT, "rr01", mx::Type::COLOR4);
        REQUIRE(var3 == "color4(color(foo, foo, 0), 1)");
        std::string var4 = syntax->getSwizzledVariable("foo", mx::Type::VECTOR3, "zyx", mx::Type::VECTOR3);
        REQUIRE(var4 == "vector(foo[2], foo[1], foo[0])");
        std::string var5 = syntax->getSwizzledVariable("foo", mx::Type::VECTOR4, "yy", mx::Type::VECTOR2);
        REQUIRE(var5 == "vector2(foo.y, foo.y)");
        std::string var6 = syntax->getSwizzledVariable("foo", mx::Type::COLOR2, "rrgg", mx::Type::VECTOR4);
        REQUIRE(var6 == "vector4(foo.r, foo.r, foo.a, foo.a)");

        // Create a simple test graph
        mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
        mx::NodePtr constant1 = nodeGraph->addNode("constant", "constant1", "color3");
        constant1->setParameterValue("value", mx::Color3(1, 2, 3));
        mx::NodePtr swizzle1 = nodeGraph->addNode("swizzle", "swizzle1", "color3");
        swizzle1->setConnectedNode("in", constant1);
        swizzle1->setParameterValue("channels", std::string("rrr"));
        mx::OutputPtr output1 = nodeGraph->addOutput();
        output1->setConnectedNode(swizzle1);

        // Test swizzle node implementation
        mx::Shader test1("test1");
        test1.initialize(output1, sg, options);
        mx::SgNode* sgNode = test1.getNodeGraph()->getNode("swizzle1");
        REQUIRE(sgNode);
        test1.addFunctionCall(sgNode, context, sg);
        const std::string test1Result = "color swizzle1_out = color(swizzle1_in[0], swizzle1_in[0], swizzle1_in[0]);\n";
        REQUIRE(test1.getSourceCode() == test1Result);

        // Change swizzle pattern and test again
        swizzle1->setParameterValue("channels", std::string("b0b"));
        mx::Shader test2("test2");
        test2.initialize(output1, sg, options);
        sgNode = test2.getNodeGraph()->getNode("swizzle1");
        REQUIRE(sgNode);
        test2.addFunctionCall(sgNode, context, sg);
        const std::string test2Result = "color swizzle1_out = color(swizzle1_in[2], 0, swizzle1_in[2]);\n";
        REQUIRE(test2.getSourceCode() == test2Result);
    }

    {
        mx::GlslShaderGenerator sg;
        sg.registerSourceCodeSearchPath(searchPath);
        const mx::Syntax* syntax = sg.getSyntax();

        // Test swizzle syntax
        std::string var1 = syntax->getSwizzledVariable("foo", mx::Type::COLOR3, "bgr", mx::Type::COLOR3);
        REQUIRE(var1 == "vec3(foo.z, foo.y, foo.x)");
        std::string var2 = syntax->getSwizzledVariable("foo", mx::Type::VECTOR2, "xy", mx::Type::COLOR2);
        REQUIRE(var2 == "vec2(foo.x, foo.y)");
        std::string var3 = syntax->getSwizzledVariable("foo", mx::Type::FLOAT, "rr01", mx::Type::COLOR4);
        REQUIRE(var3 == "vec4(foo, foo, 0, 1)");
        std::string var4 = syntax->getSwizzledVariable("foo", mx::Type::VECTOR3, "zyx", mx::Type::VECTOR3);
        REQUIRE(var4 == "vec3(foo.z, foo.y, foo.x)");
        std::string var5 = syntax->getSwizzledVariable("foo", mx::Type::VECTOR4, "yy", mx::Type::VECTOR2);
        REQUIRE(var5 == "vec2(foo.y, foo.y)");
        std::string var6 = syntax->getSwizzledVariable("foo", mx::Type::COLOR2, "rrgg", mx::Type::VECTOR4);
        REQUIRE(var6 == "vec4(foo.x, foo.x, foo.y, foo.y)");

        // Create a simple test graph
        mx::NodeGraphPtr nodeGraph = doc->addNodeGraph();
        mx::NodePtr constant1 = nodeGraph->addNode("constant", "constant1", "color3");
        constant1->setParameterValue("value", mx::Color3(1, 2, 3));
        mx::NodePtr swizzle1 = nodeGraph->addNode("swizzle", "swizzle1", "color3");
        swizzle1->setConnectedNode("in", constant1);
        swizzle1->setParameterValue("channels", std::string("rrr"));
        mx::OutputPtr output1 = nodeGraph->addOutput();
        output1->setConnectedNode(swizzle1);

        // Test swizzle node implementation
        mx::Shader test1("test1");
        test1.initialize(output1, sg, options);
        mx::SgNode* sgNode = test1.getNodeGraph()->getNode("swizzle1");
        REQUIRE(sgNode);
        test1.addFunctionCall(sgNode, context, sg);
        const std::string test1Result = "vec3 swizzle1_out = vec3(swizzle1_in.x, swizzle1_in.x, swizzle1_in.x);\n";
        REQUIRE(test1.getSourceCode() == test1Result);

        // Change swizzle pattern and test again
        swizzle1->setParameterValue("channels", std::string("b0b"));
        mx::Shader test2("test2");
        test2.initialize(output1, sg, options);
        sgNode = test2.getNodeGraph()->getNode("swizzle1");
        REQUIRE(sgNode);
        test2.addFunctionCall(sgNode, context, sg);
        const std::string test2Result = "vec3 swizzle1_out = vec3(swizzle1_in.z, 0, swizzle1_in.z);\n";
        REQUIRE(test2.getSourceCode() == test2Result);
    }
}

//
// Utility to call validate OSL. 
// For now only call into oslc to compile an OSL file and get the results.
//
static void validateOSL(const std::string oslFileName, std::string& errorResult)
{
    errorResult.clear();

    // Use the user specified build options for oslc exe, and include path
    const std::string oslcCommand(MATERIALX_OSLC_EXECUTABLE);
    const std::string oslIncludePath(MATERIALX_OSL_INCLUDE_PATH);
    if (oslcCommand.empty() || oslIncludePath.empty())
    {
        return;
    }

    // Use a known error file name to check
    std::string errorFile(oslFileName + "_errors.txt");
    const std::string redirectString(" 2>&1");

    // Run the command and get back the result. If non-empty string throw exception with error
    std::string command = oslcCommand + " -q -I\"" + oslIncludePath + "\" " + oslFileName + " > " +
        errorFile + redirectString;

    int returnValue = std::system(command.c_str());

    std::ifstream errorStream(errorFile);
    errorResult.assign(std::istreambuf_iterator<char>(errorStream),
        std::istreambuf_iterator<char>());

    if (errorResult.length())
    {
        errorResult = "Command return code: " + std::to_string(returnValue) + "\n" +
            errorResult;
        std::cout << "OSLC failed to compile: " << oslFileName << ":\n"
            << errorResult << std::endl;
    }
}

TEST_CASE("Hello World", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "hello_world";

    // Create a nodedef taking two color3 and producing another color3
    mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_" + exampleName, "color3", exampleName);
    mx::InputPtr inputA = nodeDef->addInput("a", "color3");
    mx::InputPtr inputB = nodeDef->addInput("b", "color3");
    inputA->setValue(mx::Color3(1.0f, 1.0f, 0.0f));
    inputB->setValue(mx::Color3(0.8f, 0.1f, 0.1f));

    // Create an implementation graph for the nodedef performing 
    // a multiplication of the two colors.
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    nodeGraph->setAttribute("nodedef", nodeDef->getName());
    mx::OutputPtr output1 = nodeGraph->addOutput("out", "color3");
    mx::NodePtr mult1 = nodeGraph->addNode("multiply", "mult1", "color3");
    mx::InputPtr in1 = mult1->addInput("in1", "color3");
    in1->setInterfaceName(inputA->getName());
    mx::InputPtr in2 = mult1->addInput("in2", "color3");
    in2->setInterfaceName(inputB->getName());
    output1->setConnectedNode(mult1);

    // Create a material with the above node as the shader
    mx::MaterialPtr mtrl = doc->addMaterial(exampleName + "_material");
    mx::ShaderRefPtr shaderRef = mtrl->addShaderRef(exampleName + "_shader", exampleName);

    mx::SgOptions options;

    // Arnold OSL
    {
        mx::ShaderGeneratorPtr shadergen = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shadergen->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shadergen->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        // Test shader generation from nodegraph
        mx::ShaderPtr shader = shadergen->generate(exampleName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        std::ofstream file;
        std::string fileName(shader->getName() + "_graph.osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXView library
        // For now only use externally specified oslc to check code.
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);

        // Test shader generation from shaderref
        shader = shadergen->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        fileName.assign(shader->getName() + "_shaderref.osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXView library
        // For now only use externally specified oslc to check code.
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }

    // OgsFx
    {
        mx::ShaderGeneratorPtr shadergen = mx::OgsFxShaderGenerator::create();
        shadergen->registerSourceCodeSearchPath(searchPath);

        // Test shader generation from nodegraph
        mx::ShaderPtr shader = shadergen->generate(exampleName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);
        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(shader->getName() + "_graph.ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
        file.close();

        // Test shader generation from shaderref
        shader = shadergen->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);
        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        file.open(shader->getName() + "_shaderref.ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
        file.close();
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shadergen = mx::GlslShaderGenerator::create();
        shadergen->registerSourceCodeSearchPath(searchPath);

        // Test shader generation from nodegraph
        mx::ShaderPtr shader = shadergen->generate(exampleName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::VERTEX_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::PIXEL_STAGE).length() > 0);
        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(shader->getName() + "_graph.vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(shader->getName() + "_graph.frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();

        // Test shader generation from shaderref
        shader = shadergen->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::VERTEX_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::PIXEL_STAGE).length() > 0);
        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        file.open(shader->getName() + "_shaderref.vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(shader->getName() + "_shaderref.frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
    }
}

TEST_CASE("Conditionals", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "conditionals";

    // Create a simple node graph
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph(exampleName + "_graph");

    mx::NodePtr constant1 = nodeGraph->addNode("constant", "constant1", "color3");
    constant1->setParameterValue("value", mx::Color3(0, 0, 0));
    mx::NodePtr constant2 = nodeGraph->addNode("constant", "constant2", "color3");
    constant2->setParameterValue("value", mx::Color3(1, 1, 1));
    mx::NodePtr constant3 = nodeGraph->addNode("constant", "constant3", "float");
    constant3->setParameterValue("value", 0.5f);

    mx::NodePtr compare1 = nodeGraph->addNode("compare", "compare1", "color3");
    compare1->setConnectedNode("in1", constant1);
    compare1->setConnectedNode("in2", constant2);
    compare1->setConnectedNode("intest", constant3);

    mx::NodePtr constant4 = nodeGraph->addNode("constant", "constant4", "color3");
    constant4->setParameterValue("value", mx::Color3(1, 0, 0));
    mx::NodePtr constant5 = nodeGraph->addNode("constant", "constant5", "color3");
    constant5->setParameterValue("value", mx::Color3(0, 1, 0));
    mx::NodePtr constant6 = nodeGraph->addNode("constant", "constant6", "color3");
    constant6->setParameterValue("value", mx::Color3(0, 0, 1));

    mx::NodePtr switch1 = nodeGraph->addNode("switch", "switch1", "color3");
    switch1->setConnectedNode("in1", constant4);
    switch1->setConnectedNode("in2", constant5);
    switch1->setConnectedNode("in3", constant6);
    switch1->setConnectedNode("in4", compare1);
    switch1->setParameterValue<int>("which", 3);

    // Connected to output.
    mx::OutputPtr output1 = nodeGraph->addOutput();
    output1->setConnectedNode(switch1);

    // Write out a .dot file for visualization
    std::ofstream file;
    std::string dot = nodeGraph->asStringDot();
    file.open(nodeGraph->getName() + ".dot");
    file << dot;
    file.close();

    mx::SgOptions options;

    // Arnold
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // All of the nodes should have been removed by optimization
        // leaving a graph with a single constant value
        REQUIRE(shader->getNodeGraph()->getNodes().empty());
        REQUIRE(shader->getNodeGraph()->getOutputSocket()->value != nullptr);
        REQUIRE(shader->getNodeGraph()->getOutputSocket()->value->getValueString() == constant2->getParameterValue("value")->getValueString());

        // Write out to file for inspection
        const std::string fileName(shader->getName() + ".osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXView library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }

    // OgsFx
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        file.open(shader->getName() + ".ogsfx");
        file << shader->getSourceCode();
        file.close();

        // All of the nodes should have been removed by optimization
        // leaving a graph with a single constant value
        REQUIRE(shader->getNodeGraph()->getNodes().empty());
        REQUIRE(shader->getNodeGraph()->getOutputSocket()->value != nullptr);
        REQUIRE(shader->getNodeGraph()->getOutputSocket()->value->getValueString() == constant2->getParameterValue("value")->getValueString());
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        file.open(shader->getName() + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(shader->getName() + ".frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();

        // All of the nodes should have been removed by optimization
        // leaving a graph with a single constant value
        REQUIRE(shader->getNodeGraph()->getNodes().empty());
        REQUIRE(shader->getNodeGraph()->getOutputSocket()->value != nullptr);
        REQUIRE(shader->getNodeGraph()->getOutputSocket()->value->getValueString() == constant2->getParameterValue("value")->getValueString());
    }
}

TEST_CASE("Geometric Nodes", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "geometric_nodes";

    // Create a nonsensical graph testing some geometric nodes
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);

    mx::NodePtr normal1 = nodeGraph->addNode("normal", "normal1", "vector3");
    normal1->setParameterValue("space", std::string("world"));

    mx::NodePtr position1 = nodeGraph->addNode("position", "position1", "vector3");
    position1->setParameterValue("space", std::string("world"));

    mx::NodePtr texcoord1 = nodeGraph->addNode("texcoord", "texcoord1", "vector2");
    texcoord1->setParameterValue("index", 0, "integer");

    mx::NodePtr geomcolor1 = nodeGraph->addNode("geomcolor", "geomcolor1", "color3");
    geomcolor1->setParameterValue("index", 0, "integer");

    mx::NodePtr geomattrvalue1 = nodeGraph->addNode("geomattrvalue", "geomattrvalue1", "float");
    geomattrvalue1->setParameterValue("attrname", std::string("temperature"));

    mx::NodePtr add1 = nodeGraph->addNode("add", "add1", "vector3");
    add1->setConnectedNode("in1", normal1);
    add1->setConnectedNode("in2", position1);

    mx::NodePtr multiply1 = nodeGraph->addNode("multiply", "multiply1", "color3");
    multiply1->setConnectedNode("in1", geomcolor1);
    multiply1->setConnectedNode("in2", geomattrvalue1);

    mx::NodePtr convert1 = nodeGraph->addNode("swizzle", "convert1", "color3");
    convert1->setConnectedNode("in", add1);
    convert1->setParameterValue("channels", std::string("xyz"));

    mx::NodePtr multiply2 = nodeGraph->addNode("multiply", "multiply2", "color3");
    multiply2->setConnectedNode("in1", convert1);
    multiply2->setConnectedNode("in2", multiply1);

    mx::NodePtr time1 = nodeGraph->addNode("time", "time1", "float");
    mx::NodePtr multiply3 = nodeGraph->addNode("multiply", "multiply3", "color3");
    multiply3->setConnectedNode("in1", multiply2);
    multiply3->setConnectedNode("in2", time1);

    mx::NodePtr frame1 = nodeGraph->addNode("frame", "frame1", "float");
    mx::NodePtr multiply4 = nodeGraph->addNode("multiply", "multiply4", "color3");
    multiply4->setConnectedNode("in1", multiply3);
    multiply4->setConnectedNode("in2", frame1);

    // Connected to output.
    mx::OutputPtr output1 = nodeGraph->addOutput(mx::EMPTY_STRING, "color3");
    output1->setConnectedNode(multiply4);

    // Create a nodedef and make its implementation be the graph above
    mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_" + exampleName, "color3", exampleName);
    nodeGraph->setAttribute("nodedef", nodeDef->getName());

    // Create a material with the above node as the shader
    mx::MaterialPtr mtrl = doc->addMaterial(exampleName + "_material");
    mx::ShaderRefPtr shaderRef = mtrl->addShaderRef(exampleName + "_shader", exampleName);

    mx::SgOptions options;

    // Arnold
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        std::ofstream file;
        const std::string fileName(shader->getName() + ".osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXView library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0); 
    }

    // OgsFx
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(shader->getName() + ".ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(shader->getName() + ".frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
        file.open(shader->getName() + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
    }
}

TEST_CASE("Noise", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "noise_test";

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    mx::OutputPtr output1 = nodeGraph->addOutput("out", "color3");

    std::vector<mx::NodePtr> noiseNodes;
    mx::NodePtr noise2d = nodeGraph->addNode("noise2d", "noise2d", "float");
    noiseNodes.push_back(noise2d);
    mx::NodePtr noise3d = nodeGraph->addNode("noise3d", "noise3d", "float");
    noiseNodes.push_back(noise3d);
    mx::NodePtr cellnoise2d = nodeGraph->addNode("cellnoise2d", "cellnoise2d", "float");
    noiseNodes.push_back(cellnoise2d);
    mx::NodePtr cellnoise3d = nodeGraph->addNode("cellnoise3d", "cellnoise3d", "float");
    noiseNodes.push_back(cellnoise3d);
    mx::NodePtr fractal3d = nodeGraph->addNode("fractal3d", "fractal3d", "float");
    noiseNodes.push_back(fractal3d);

    noise2d->setParameterValue("amplitude", 1.0);
    noise2d->setParameterValue("pivot", 0.0f);
    noise3d->setParameterValue("amplitude", 1.0);
    noise3d->setParameterValue("pivot", 0.0f);
    fractal3d->setParameterValue("amplitude", 1.0f);

    // Multiplier to scale noise input uv's
    mx::NodePtr uv1 = nodeGraph->addNode("texcoord", "uv1", "vector2");
    mx::NodePtr uvmult1 = nodeGraph->addNode("multiply", "uvmult1", "vector2");
    uvmult1->setConnectedNode("in1", uv1);
    uvmult1->setInputValue("in2", mx::Vector2(16, 16));

    // Multiplier to scale noise input position
    mx::NodePtr pos1 = nodeGraph->addNode("position", "pos1", "vector3");
    noise3d->setConnectedNode("position", pos1);
    mx::NodePtr posmult1 = nodeGraph->addNode("multiply", "posmult1", "vector3");
    posmult1->setConnectedNode("in1", pos1);
    posmult1->setInputValue("in2", mx::Vector3(16, 16, 16));

    noise2d->setConnectedNode("texcoord", uvmult1);
    noise3d->setConnectedNode("position", posmult1);
    cellnoise2d->setConnectedNode("texcoord", uvmult1);
    cellnoise3d->setConnectedNode("position", posmult1);
    fractal3d->setConnectedNode("position", posmult1);

    // Create a noise selector switch
    mx::NodePtr switch1 = nodeGraph->addNode("switch", "switch1", "float");
    switch1->setConnectedNode("in1", noise2d);
    switch1->setConnectedNode("in2", noise3d);
    switch1->setConnectedNode("in3", cellnoise2d);
    switch1->setConnectedNode("in4", cellnoise3d);
    switch1->setConnectedNode("in5", fractal3d);

    // Remap the noise to [0,1]
    mx::NodePtr add1 = nodeGraph->addNode("add", "add1", "float");
    mx::NodePtr multiply1 = nodeGraph->addNode("multiply", "multiply1", "float");
    add1->setConnectedNode("in1", switch1);
    add1->setInputValue("in2", 1.0f);
    multiply1->setConnectedNode("in1", add1);
    multiply1->setInputValue("in2", 0.5f);

    // Blend some colors using the noise
    mx::NodePtr mixer = nodeGraph->addNode("mix", "mixer", "color3");
    mixer->setInputValue("fg", mx::Color3(1, 0, 0));
    mixer->setInputValue("bg", mx::Color3(1, 1, 0));
    mixer->setConnectedNode("mix", multiply1);

    output1->setConnectedNode(mixer);

    mx::SgOptions options;

    const size_t numNoiseType = noiseNodes.size();
    for (size_t noiseType = 0; noiseType < numNoiseType; ++noiseType)
    {
        const std::string shaderName = "test_" + noiseNodes[noiseType]->getName();
        
        // Select the noise type
        switch1->setParameterValue("which", float(noiseType));

        // Arnold OSL
        {
            mx::ShaderGeneratorPtr shadergen = mx::ArnoldShaderGenerator::create();
            // Add path to find all source code snippets
            shadergen->registerSourceCodeSearchPath(searchPath);
            // Add path to find OSL include files
            shadergen->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

            // Test shader generation from nodegraph
            mx::ShaderPtr shader = shadergen->generate(shaderName, output1, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode().length() > 0);

            // Write out to file for inspection
            std::ofstream file;
            const std::string fileName(shader->getName() + ".osl");
            file.open(fileName);
            file << shader->getSourceCode();
            file.close();

            // TODO: Use validation in MaterialXView library
            std::string errorResult;
            validateOSL(fileName, errorResult);
            REQUIRE(errorResult.size() == 0); 
        }

        // OgsFx
        {
            mx::ShaderGeneratorPtr shadergen = mx::OgsFxShaderGenerator::create();
            shadergen->registerSourceCodeSearchPath(searchPath);

            // Test shader generation from nodegraph
            mx::ShaderPtr shader = shadergen->generate(shaderName, output1, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);
            // Write out to file for inspection
            // TODO: Use validation in MaterialXView library
            std::ofstream file;
            file.open(shader->getName() + ".ogsfx");
            file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
            file.close();
        }

        // Glsl
        {
            mx::ShaderGeneratorPtr shadergen = mx::GlslShaderGenerator::create();
            shadergen->registerSourceCodeSearchPath(searchPath);

            // Test shader generation from nodegraph
            mx::ShaderPtr shader = shadergen->generate(shaderName, output1, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode(mx::OgsFxShader::VERTEX_STAGE).length() > 0);
            REQUIRE(shader->getSourceCode(mx::OgsFxShader::PIXEL_STAGE).length() > 0);
            // Write out to file for inspection
            // TODO: Use validation in MaterialXView library
            std::ofstream file;
            file.open(shader->getName() + ".vert");
            file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
            file.close();
            file.open(shader->getName() + ".frag");
            file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
            file.close();
        }
    }
}


TEST_CASE("Unique Names", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "unique_names";

    // Generate a shade with an internal node having the same name as the shader,
    // which will result in a name conflict between the shader output and the
    // internal node output
    const std::string shaderName = "unique_names";
    const std::string nodeName = shaderName;

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    mx::OutputPtr output1 = nodeGraph->addOutput("out", "color3");
    mx::NodePtr node1 = nodeGraph->addNode("noise2d", nodeName, "color3");

    output1->setConnectedNode(node1);

    mx::SgOptions options;

    // Arnold
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        // Set the output to a restricted name for OSL
        output1->setName("output");

        mx::ShaderPtr shader = shaderGenerator->generate(shaderName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Make sure the output and internal node output has been renamed
        mx::SgOutputSocket* sgOutputSocket = shader->getNodeGraph()->getOutputSocket();
        REQUIRE(sgOutputSocket->name != "output");
        mx::SgNode* sgNode1 = shader->getNodeGraph()->getNode(node1->getName());
        REQUIRE(sgNode1->getOutput()->name == "unique_names_out");

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        const std::string fileName(exampleName + ".osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXView library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }

    // OgsFx
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Set the output to a restricted name for OgsFx
        output1->setName("out");

        mx::ShaderPtr shader = shaderGenerator->generate(shaderName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

        // Make sure the output and internal node output has been renamed
        mx::SgOutputSocket* sgOutputSocket = shader->getNodeGraph()->getOutputSocket();
        REQUIRE(sgOutputSocket->name != "out");
        mx::SgNode* sgNode1 = shader->getNodeGraph()->getNode(node1->getName());
        REQUIRE(sgNode1->getOutput()->name == "unique_names_out");

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(exampleName + ".ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Set the output to a restricted name for GLSL
        output1->setName("vec3");

        mx::ShaderPtr shader = shaderGenerator->generate(shaderName, output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        // Make sure the output and internal node output has been renamed
        mx::SgOutputSocket* sgOutputSocket = shader->getNodeGraph()->getOutputSocket();
        REQUIRE(sgOutputSocket->name != "vec3");
        mx::SgNode* sgNode1 = shader->getNodeGraph()->getNode(node1->getName());
        REQUIRE(sgNode1->getOutput()->name == "unique_names_out");

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(exampleName + ".frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
        file.open(exampleName + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
    }
}

TEST_CASE("Subgraphs", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    mx::FilePath examplesSearchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Examples");
    loadExamples({ "SubGraphs.mtlx"}, examplesSearchPath, searchPath,  doc);

    std::vector<std::string> exampleGraphNames = { "subgraph_ex1" , "subgraph_ex2" };

    mx::SgOptions options;

    // Arnold
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        for (const std::string& graphName : exampleGraphNames)
        {
            mx::NodeGraphPtr nodeGraph = doc->getNodeGraph(graphName);
            REQUIRE(nodeGraph != nullptr);

            mx::OutputPtr output = nodeGraph->getOutput("out");
            REQUIRE(output != nullptr);

            mx::ShaderPtr shader = shaderGenerator->generate(graphName, output, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode().length() > 0);

            // Write out to file for inspection
            // TODO: Use validation in MaterialXView library
            std::ofstream file;
            const std::string fileName(shader->getName() + ".osl");
            file.open(fileName);
            file << shader->getSourceCode();
            file.close();

            // TODO: Use validation in MaterialXView library
            std::string errorResult;
            validateOSL(fileName, errorResult);
            REQUIRE(errorResult.size() == 0); 
        }
    }

    // OgsFx
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        for (const std::string& graphName : exampleGraphNames)
        {
            mx::NodeGraphPtr nodeGraph = doc->getNodeGraph(graphName);
            REQUIRE(nodeGraph != nullptr);

            mx::OutputPtr output = nodeGraph->getOutput("out");
            REQUIRE(output != nullptr);

            mx::ShaderPtr shader = shaderGenerator->generate(graphName, output, options);
            REQUIRE(shader != nullptr);
            REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

            // Write out to file for inspection
            // TODO: Use validation in MaterialXView library
            std::ofstream file;
            file.open(shader->getName() + ".ogsfx");
            file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
        }
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        for (const std::string& graphName : exampleGraphNames)
        {
            mx::NodeGraphPtr nodeGraph = doc->getNodeGraph(graphName);
            REQUIRE(nodeGraph != nullptr);

            mx::OutputPtr output = nodeGraph->getOutput("out");
            REQUIRE(output != nullptr);

            mx::ShaderPtr shader = shaderGenerator->generate(graphName, output, options);
            REQUIRE(shader != nullptr);

            REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
            REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

            // Write out to file for inspection
            // TODO: Use validation in MaterialXView library
            std::ofstream file;
            file.open(shader->getName() + ".frag");
            file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
            file.close();
            file.open(shader->getName() + ".vert");
            file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        }
    }
}

TEST_CASE("Materials", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    std::vector<mx::MaterialPtr> materials;
    createExampleMaterials(doc, materials);

    mx::SgOptions options;

    // Arnold
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        for (const mx::MaterialPtr& material : materials)
        {
            for (mx::ShaderRefPtr shaderRef : material->getShaderRefs())
            {
                const std::string name = material->getName() + "_" + shaderRef->getName();
                mx::ShaderPtr shader = shaderGenerator->generate(name, shaderRef, options);
                REQUIRE(shader != nullptr);
                REQUIRE(shader->getSourceCode().length() > 0);

                // Write out to file for inspection
                std::ofstream file;
                const std::string fileName(shader->getName() + ".osl");
                file.open(fileName);
                file << shader->getSourceCode();
                file.close();

                // TODO: Use validation in MaterialXView library
                std::string errorResult;
                validateOSL(fileName, errorResult);
                REQUIRE(errorResult.size() == 0); 
            }
        }
    }

    // OgsFx
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        for (const mx::MaterialPtr& material : materials)
        {
            for (mx::ShaderRefPtr shaderRef : material->getShaderRefs())
            {
                const std::string name = material->getName() + "_" + shaderRef->getName();
                mx::ShaderPtr shader = shaderGenerator->generate(name, shaderRef, options);
                REQUIRE(shader != nullptr);
                REQUIRE(shader->getSourceCode().length() > 0);

                // Write out to file for inspection
                // TODO: Use validation in MaterialXView library
                std::ofstream file;
                file.open(shader->getName() + ".ogsfx");
                file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
            }
        }
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        for (const mx::MaterialPtr& material : materials)
        {
            for (mx::ShaderRefPtr shaderRef : material->getShaderRefs())
            {
                const std::string name = material->getName() + "_" + shaderRef->getName();
                mx::ShaderPtr shader = shaderGenerator->generate(name, shaderRef, options);
                REQUIRE(shader != nullptr);
                REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
                REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

                // Write out to file for inspection
                // TODO: Use validation in MaterialXView library
                std::ofstream file;
                file.open(shader->getName() + ".frag");
                file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
                file.close();
                file.open(shader->getName() + ".vert");
                file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
            }
        }
    }
}

TEST_CASE("Color Spaces", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::StringVec libraryNames;
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    mx::MaterialPtr material = doc->addMaterial("color_spaces");
    mx::ShaderRefPtr shaderRef = material->addShaderRef("color_spaces_surface", "standard_surface");

    // Bind an image texture to the base_color input, with sRGB color space
    mx::NodePtr baseColorTex = doc->addNode("image", "base_color_tex", "color3");
    mx::ParameterPtr baseColorTexFileParam = baseColorTex->setParameterValue("file", std::string("image1.png"), "filename");
    baseColorTexFileParam->setAttribute("colorspace", "sRGB");
    mx::OutputPtr baseColorOutput = doc->addOutput("baseColorOutput", "color3");
    baseColorOutput->setConnectedNode(baseColorTex);
    mx::BindInputPtr baseColorBind = shaderRef->addBindInput("base_color", "color3");
    baseColorBind->setConnectedOutput(baseColorOutput);

    // Bind an image texture to the specular_roughness input, with sRGB color space
    // This color spaces transform should be ignored since it's a float data type
    mx::NodePtr rougnessTex = doc->addNode("image", "specular_roughness_tex", "float");
    mx::ParameterPtr rougnessTexFileParam = rougnessTex->setParameterValue("file", std::string("image2.png"), "filename");
    rougnessTexFileParam->setAttribute("colorspace", "sRGB");
    mx::OutputPtr roughnessOutput = doc->addOutput("roughnessOutput", "float");
    roughnessOutput->setConnectedNode(rougnessTex);
    mx::BindInputPtr rougnessBind = shaderRef->addBindInput("specular_roughness", "float");
    rougnessBind->setConnectedOutput(roughnessOutput);

    mx::SgOptions options;

    // Arnold
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        mx::ShaderPtr shader = shaderGenerator->generate(material->getName(), shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        std::ofstream file;
        const std::string fileName(shader->getName() + ".osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXView library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }

    // OgsFx
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(material->getName(), shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(shader->getName() + ".ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        mx::ShaderPtr shader = shaderGenerator->generate(material->getName(), shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(shader->getName() + ".frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
        file.open(shader->getName() + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
    }
}


TEST_CASE("BSDF Layering", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    const std::string exampleName = "layered_bsdf";

    // Create a nodedef interface for the surface shader
    mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_" + exampleName, "surfaceshader", exampleName);
    nodeDef->addInput("diffuse_color", "color3");
    nodeDef->addInput("sss_color", "color3");
    nodeDef->addInput("sss_weight", "float");
    nodeDef->addInput("coating_color", "color3");
    nodeDef->addInput("coating_roughness", "float");
    nodeDef->addInput("coating_ior", "float");

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    nodeGraph->setAttribute("nodedef", nodeDef->getName());

    // Diffuse component
    mx::NodePtr diffuse = nodeGraph->addNode("diffusebsdf", "diffuse", "BSDF");
    mx::InputPtr diffuse_color = diffuse->addInput("reflectance", "color3");
    diffuse_color->setInterfaceName("diffuse_color");

    // Translucent (thin walled SSS) component
    mx::NodePtr sss = nodeGraph->addNode("translucentbsdf", "sss", "BSDF");
    mx::InputPtr sss_color = sss->addInput("transmittance", "color3");
    sss_color->setInterfaceName("sss_color");

    // Mix diffuse over sss
    mx::NodePtr substrate = nodeGraph->addNode("mixbsdf", "substrate", "BSDF");
    mx::NodePtr substrate_weight_inv = nodeGraph->addNode("invert", "substrate_weight_inv", "float");
    substrate->setConnectedNode("in1", diffuse);
    substrate->setConnectedNode("in2", sss);
    substrate->setConnectedNode("weight", substrate_weight_inv);
    mx::InputPtr sss_weight = substrate_weight_inv->addInput("in", "float");
    sss_weight->setInterfaceName("sss_weight");

    // Add a coating specular component on top
    mx::NodePtr coating = nodeGraph->addNode("coatingbsdf", "coating", "BSDF");
    coating->setConnectedNode("base", substrate);
    mx::InputPtr coating_color = coating->addInput("reflectance", "color3");
    coating_color->setInterfaceName("coating_color");

    mx::InputPtr coating_roughness = coating->addInput("roughness", "float");
    coating_roughness->setInterfaceName("coating_roughness");
    mx::InputPtr coating_ior = coating->addInput("ior", "float");
    coating_ior->setInterfaceName("coating_ior");

    // Create a surface shader
    mx::NodePtr surface = nodeGraph->addNode("surface", "surface1", "surfaceshader");
    surface->setConnectedNode("bsdf", coating);

    // Connect to graph output
    mx::OutputPtr output = nodeGraph->addOutput("out", "surfaceshader");
    output->setConnectedNode(surface);

    // Create a material with the above node as the shader
    mx::MaterialPtr mtrl = doc->addMaterial(exampleName + "_material");
    mx::ShaderRefPtr shaderRef = mtrl->addShaderRef(exampleName + "_shader", exampleName);

    // Bind shader parameter values
    mx::BindInputPtr diffuse_color_input = shaderRef->addBindInput("diffuse_color", "color3");
    diffuse_color_input->setValue(mx::Color3(0.8f, 0.2f, 0.8f));
    mx::BindInputPtr sss_color_input = shaderRef->addBindInput("sss_color", "color3");
    sss_color_input->setValue(mx::Color3(1.0f, 0.0f, 0.0f));
    mx::BindInputPtr sss_weight_input = shaderRef->addBindInput("sss_weight", "float");
    sss_weight_input->setValue(0.45f);
    mx::BindInputPtr coating_color_input = shaderRef->addBindInput("coating_color", "color3");
    coating_color_input->setValue(mx::Color3(1.0f, 1.0f, 1.0f));
    mx::BindInputPtr coating_roughness_input = shaderRef->addBindInput("coating_roughness", "float");
    coating_roughness_input->setValue(0.2f);
    mx::BindInputPtr coating_ior_input = shaderRef->addBindInput("coating_ior", "float");
    coating_ior_input->setValue(1.52f);

    mx::SgOptions options;

    // Arnold
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        const std::string fileName(shader->getName() + ".osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXView library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }

    // OgsFx
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(shader->getName() + ".ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(shader->getName() + ".frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
        file.open(shader->getName() + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
    }
}

TEST_CASE("Transparency", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    const std::string exampleName = "transparent_surface";

    // Create a nodedef interface for the surface shader
    mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_" + exampleName, "surfaceshader", exampleName);
    nodeDef->addInput("transmittance", "color3");
    nodeDef->addInput("coating_color", "color3");
    nodeDef->addInput("roughness", "float");
    nodeDef->addInput("ior", "float");
    nodeDef->addInput("opacity", "float");

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    nodeGraph->setAttribute("nodedef", nodeDef->getName());

    mx::NodePtr refraction = nodeGraph->addNode("refractionbsdf", "refraction", "BSDF");
    mx::InputPtr transmittance = refraction->addInput("transmittance", "color3");
    transmittance->setInterfaceName("transmittance");

    mx::NodePtr coating = nodeGraph->addNode("coatingbsdf", "coating", "BSDF");
    coating->setConnectedNode("base", refraction);
    mx::InputPtr coating_color = coating->addInput("reflectance", "color3");
    coating_color->setInterfaceName("coating_color");

    mx::NodePtr ior_common = nodeGraph->addNode("constant", "ior_common", "float");
    mx::ParameterPtr ior = ior_common->addParameter("value", "float");
    ior->setInterfaceName("ior");
    coating->setConnectedNode("ior", ior_common);
    refraction->setConnectedNode("ior", ior_common);

    mx::NodePtr roughness_common = nodeGraph->addNode("constant", "roughness_common", "float");
    mx::ParameterPtr roughness = roughness_common->addParameter("value", "float");
    roughness->setInterfaceName("roughness");
    coating->setConnectedNode("roughness", roughness_common);
    refraction->setConnectedNode("roughness", roughness_common);

    mx::NodePtr surface = nodeGraph->addNode("surface", "surface1", "surfaceshader");
    surface->setConnectedNode("bsdf", coating);
    mx::InputPtr opacity = surface->addInput("opacity", "float");
    opacity->setInterfaceName("opacity");

    mx::OutputPtr output = nodeGraph->addOutput("out", "surfaceshader");
    output->setConnectedNode(surface);

    // Create a material with the above node as the shader
    mx::MaterialPtr mtrl = doc->addMaterial(exampleName + "_material");
    mx::ShaderRefPtr shaderRef = mtrl->addShaderRef(exampleName + "_shader", exampleName);

    // Bind shader parameter values
    mx::BindInputPtr transmittance_input = shaderRef->addBindInput("transmittance", "color3");
    transmittance_input->setValue(mx::Color3(0.0f, 0.0f, 0.0f));
    mx::BindInputPtr coating_color_input = shaderRef->addBindInput("coating_color", "color3");
    coating_color_input->setValue(mx::Color3(1.0f, 1.0f, 1.0f));
    mx::BindInputPtr roughness_input = shaderRef->addBindInput("roughness", "float");
    roughness_input->setValue(0.2f);
    mx::BindInputPtr ior_input = shaderRef->addBindInput("ior", "float");
    ior_input->setValue(1.52f);
    mx::BindInputPtr opacity_input = shaderRef->addBindInput("opacity", "float");
    opacity_input->setValue(1.0f);

    mx::SgOptions options;

    // Arnold
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shaderGenerator->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        const std::string fileName(shader->getName() + ".osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXView library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0);
    }

    // OgsFx
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(shader->getName() + ".ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(shader->getName() + ".frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
        file.open(shader->getName() + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
    }
}

TEST_CASE("Surface Layering", "[shadergen]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    const std::string exampleName = "layered_surface";

    // Create a nodedef interface for the surface shader
    mx::NodeDefPtr nodeDef = doc->addNodeDef("ND_" + exampleName, "surfaceshader", exampleName);
    nodeDef->addInput("layer1_diffuse", "color3");
    nodeDef->addInput("layer1_specular", "color3");
    nodeDef->addInput("layer2_diffuse", "color3");
    nodeDef->addInput("layer2_specular", "color3");
    nodeDef->addInput("mix_weight", "float");

    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("IMP_" + exampleName);
    nodeGraph->setAttribute("nodedef", nodeDef->getName());

    // Create first surface layer from a surface with two BSDF's
    mx::NodePtr layer1_diffuse = nodeGraph->addNode("diffusebsdf", "layer1_diffuse", "BSDF");
    mx::InputPtr layer1_diffuse_color = layer1_diffuse->addInput("reflectance", "color3");
    layer1_diffuse_color->setInterfaceName("layer1_diffuse");
    mx::NodePtr layer1_specular = nodeGraph->addNode("coatingbsdf", "layer1_specular", "BSDF");
    layer1_specular->setConnectedNode("base", layer1_diffuse);
    mx::InputPtr layer1_specular_color = layer1_specular->addInput("reflectance", "color3");
    layer1_specular_color->setInterfaceName("layer1_specular");
    mx::NodePtr layer1 = nodeGraph->addNode("surface", "layer1", "surfaceshader");
    layer1->setConnectedNode("bsdf", layer1_specular);

    // Create second surface layer from a standard uber shader
    mx::NodePtr layer2 = nodeGraph->addNode("standard_surface", "layer2", "surfaceshader");
    mx::InputPtr layer2_diffuse_color = layer2->addInput("base_color", "color3");
    layer2_diffuse_color->setInterfaceName("layer2_diffuse");
    mx::InputPtr layer2_specular_color = layer2->addInput("specular_color", "color3");
    layer2_specular_color->setInterfaceName("layer2_specular");

    // Create layer mixer
    mx::NodePtr mixer = nodeGraph->addNode("layeredsurface", "mixer", "surfaceshader");
    mixer->setConnectedNode("top", layer2);
    mixer->setConnectedNode("base", layer1);
    mx::InputPtr mix_weight = mixer->addInput("weight", "float");
    mix_weight->setInterfaceName("mix_weight");

    // Connect to graph output
    mx::OutputPtr output = nodeGraph->addOutput("out", "surfaceshader");
    output->setConnectedNode(mixer);

    // Create a material with the above node as the shader
    mx::MaterialPtr mtrl = doc->addMaterial(exampleName + "_material");
    mx::ShaderRefPtr shaderRef = mtrl->addShaderRef(exampleName + "_shader", exampleName);

    // Bind shader parameter values
    mx::BindInputPtr layer1_diffuse_input = shaderRef->addBindInput("layer1_diffuse", "color3");
    layer1_diffuse_input->setValue(mx::Color3(0.2f, 0.8f, 0.2f));
    mx::BindInputPtr layer1_specular_input = shaderRef->addBindInput("layer1_specular", "color3");
    layer1_specular_input->setValue(mx::Color3(1.0f, 1.0f, 1.0f));
    mx::BindInputPtr layer2_diffuse_input = shaderRef->addBindInput("layer2_diffuse", "color3");
    layer2_diffuse_input->setValue(mx::Color3(0.8f, 0.2f, 0.8f));
    mx::BindInputPtr layer2_specular_input = shaderRef->addBindInput("layer2_specular", "color3");
    layer2_specular_input->setValue(mx::Color3(1.0f, 0.0f, 0.0f));
    mx::BindInputPtr mix_weight_input = shaderRef->addBindInput("mix_weight", "float");
    mix_weight_input->setValue(0.5f);

    mx::SgOptions options;

    // OgsFx
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::OgsFxShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(shader->getName() + ".ogsfx");
        file << shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
    }

    // Glsl
    {
        mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
        shaderGenerator->registerSourceCodeSearchPath(searchPath);

        // Setup lighting
        mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
        createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

        mx::ShaderPtr shader = shaderGenerator->generate(exampleName, shaderRef, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        // Write out to file for inspection
        // TODO: Use validation in MaterialXView library
        std::ofstream file;
        file.open(shader->getName() + ".frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();
        file.open(shader->getName() + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
    }
}

TEST_CASE("Osl Output Types", "[shadergen]")
{
    // OSL doesn't support having color2/color4 as shader output types.
    // The color2/color4 types are custom struct types added by MaterialX.
    // It's actually crashing the OSL compiler right now.
    // TODO: Report this problem to the OSL team.
    //
    // This test makes sure that color2/color4/vector2/vector4 gets converted
    // to color/vector when used as shader outputs.

    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib" }, searchPath, doc);

    const std::string exampleName = "osl_output";

    mx::NodeGraphPtr nodeGraph1 = doc->addNodeGraph();
    mx::OutputPtr output1 = nodeGraph1->addOutput(mx::EMPTY_STRING, "color2");
    mx::NodePtr node1 = nodeGraph1->addNode("remap", mx::EMPTY_STRING, "color2");
    output1->setConnectedNode(node1);
    mx::NodeDefPtr nodeDef1 = doc->addNodeDef(mx::EMPTY_STRING, "color2", exampleName + "_color2");
    nodeGraph1->setAttribute("nodedef", nodeDef1->getName());

    mx::NodeGraphPtr nodeGraph2 = doc->addNodeGraph();
    mx::OutputPtr output2 = nodeGraph2->addOutput(mx::EMPTY_STRING, "color4");
    mx::NodePtr node2 = nodeGraph2->addNode("remap", mx::EMPTY_STRING, "color4");
    output2->setConnectedNode(node2);
    mx::NodeDefPtr nodeDef2 = doc->addNodeDef(mx::EMPTY_STRING, "color4", exampleName + "_color4");
    nodeGraph2->setAttribute("nodedef", nodeDef2->getName());

    mx::SgOptions options;

    {
        mx::ShaderGeneratorPtr shadergen = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shadergen->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shadergen->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        // Test shader generation from color2 type graph
        mx::ShaderPtr shader = shadergen->generate(exampleName + "_color2", output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        std::ofstream file;
        std::string fileName(exampleName + "_color2.osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXView library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0); 

        // Test shader generation from color4 type graph
        shader = shadergen->generate(exampleName + "_color4", output2, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        fileName.assign(exampleName + "_color4.osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXView library
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0); 
    }

    // Change to vector2/vector4 types
    output1->setType("vector2");
    node1->setType("vector2");
    nodeDef1->setType("vector2");
    output2->setType("vector4");
    node2->setType("vector4");
    nodeDef2->setType("vector4");

    // Add swizzling to make sure type remapping works with swizzling
    //output1->setChannels("yx");
    //output2->setChannels("wzyx");

    {
        mx::ShaderGeneratorPtr shadergen = mx::ArnoldShaderGenerator::create();
        // Add path to find all source code snippets
        shadergen->registerSourceCodeSearchPath(searchPath);
        // Add path to find OSL include files
        shadergen->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        // Test shader generation from color2 type graph
        mx::ShaderPtr shader = shadergen->generate(exampleName + "_vector2", output1, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        std::ofstream file;
        std::string fileName(exampleName + "_vector2.osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXView library
        std::string errorResult;
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0); 

        // Test shader generation from color4 type graph
        shader = shadergen->generate(exampleName + "_vector4", output2, options);
        REQUIRE(shader != nullptr);
        REQUIRE(shader->getSourceCode().length() > 0);
        // Write out to file for inspection
        fileName.assign(exampleName + "_vector4.osl");
        file.open(fileName);
        file << shader->getSourceCode();
        file.close();

        // TODO: Use validation in MaterialXView library
        validateOSL(fileName, errorResult);
        REQUIRE(errorResult.size() == 0); 
    }
}

