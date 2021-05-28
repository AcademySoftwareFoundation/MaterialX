//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>

#include <MaterialXFormat/Util.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXFormat/Environ.h>

#include <MaterialXGenArnold/ArnoldShaderGenerator.h>

#include <MaterialXRender/StbImageLoader.h>
#if defined(MATERIALX_BUILD_OIIO)
#include <MaterialXRender/OiioImageLoader.h>
#endif

namespace mx = MaterialX;

class ArnoldShaderRenderTester : public RenderUtil::ShaderRenderTester
{
  public:
    ArnoldShaderRenderTester() :
        RenderUtil::ShaderRenderTester(mx::ArnoldShaderGenerator::create())
    {
    }

  protected:
    void registerSourceCodeSearchPaths(mx::GenContext& context) override
    {
        // Include extra OSL implementation files
        mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");
        context.registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

        // Include current path to find resources.
        context.registerSourceCodeSearchPath(mx::FilePath::getCurrentPath());
    }

    void createRenderer(std::ostream& log) override;

    bool runRenderer(const std::string& shaderName,
                      mx::TypedElementPtr element,
                      mx::GenContext& context,
                      mx::DocumentPtr doc,
                      std::ostream& log,
                      const GenShaderUtil::TestSuiteOptions& testOptions,
                      RenderUtil::RenderProfileTimes& profileTimes,
                      const mx::FileSearchPath& imageSearchPath,
                      const std::string& outputPath = ".",
                      mx::ImageVec* imageVec = nullptr) override;

    mx::ImageHandlerPtr _imageHandler;
};

void ArnoldShaderRenderTester::createRenderer(std::ostream& /*log*/)
{
    mx::StbImageLoaderPtr stbLoader = mx::StbImageLoader::create();
    _imageHandler = mx::ImageHandler::create(stbLoader);
#if defined(MATERIALX_BUILD_OIIO)
    mx::OiioImageLoaderPtr oiioLoader = mx::OiioImageLoader::create();
    _imageHandler->addLoader(oiioLoader);
#endif

}

bool ArnoldShaderRenderTester::runRenderer(const std::string& shaderName,
                                            mx::TypedElementPtr element,
                                            mx::GenContext& context,
                                            mx::DocumentPtr doc,
                                            std::ostream& log,
                                            const GenShaderUtil::TestSuiteOptions& testOptions,
                                            RenderUtil::RenderProfileTimes& profileTimes,
                                            const mx::FileSearchPath& imageSearchPath,
                                            const std::string& outputPath,
                                            mx::ImageVec* /*returnImage*/)
{
    RenderUtil::AdditiveScopedTimer totalArnoldTime(profileTimes.languageTimes.totalTime, "Arnold total time");

    const mx::ShaderGenerator& shadergen = context.getShaderGenerator();

    // Perform validation if requested
    if (testOptions.validateElementToRender)
    {
        std::string message;
        if (!element->validate(&message))
        {
            log << "Element is invalid: " << message << std::endl;
            return false;
        }
    }

    // Make a working copy of the document
    mx::DocumentPtr arnoldDoc = doc->copy();
    doc = arnoldDoc;
    element = (doc->getDescendant(element->getNamePath()))->asA<mx::TypedElement>();

    // Prepocess 1: Create resolver to replace all Windows separators with POSIX ones
    // and flatten all image filenames
    mx::StringResolverPtr separatorReplacer = mx::StringResolver::create();
    separatorReplacer->setFilenameSubstitution("\\\\", "/");
    separatorReplacer->setFilenameSubstitution("\\", "/");
    mx::FileSearchPath flattenSearchPath(mx::FilePath::getCurrentPath());
    flattenSearchPath.append(imageSearchPath);
    mx::flattenFilenames(doc, flattenSearchPath, separatorReplacer);

    // Prepocess to convert units to the final values.
    // TODO: Make this a utility.
    mx::UnitSystemPtr unitSystem = shadergen.getUnitSystem();
    if (unitSystem)
    {
        const std::string DISTANCE_TYPE_STRING("distance");
        const std::string& targetDistanceUnit = context.getOptions().targetDistanceUnit;
        mx::UnitConverterRegistryPtr unitRegistry = unitSystem->getUnitConverterRegistry();
        mx::UnitTypeDefPtr distanceTypeDef = doc->getUnitTypeDef(DISTANCE_TYPE_STRING);
        mx::UnitConverterPtr distanceConverter = unitRegistry->getUnitConverter(distanceTypeDef);

        bool convertedUnits = false;
        for (mx::ElementPtr elem : doc->traverseTree())
        {
            mx::NodePtr pNode = elem->asA<mx::Node>();
            if (pNode)
            {
                if (pNode->getInputCount()) 
                {
                    for (mx::InputPtr input : pNode->getInputs()) 
                    {
                        const std::string type = input->getType();
                        const mx::ValuePtr value = input->getValue();
                        if (value && input->hasUnit() && (input->getUnitType() == DISTANCE_TYPE_STRING) && value) 
                        {
                            if (type == "float")
                            {
                                float originalval = value->asA<float>();
                                float convertedValue = distanceConverter->convert(originalval, input->getUnit(), targetDistanceUnit);
                                input->setValue<float>(convertedValue);
                                input->removeAttribute(mx::ValueElement::UNIT_ATTRIBUTE);
                                input->removeAttribute(mx::ValueElement::UNITTYPE_ATTRIBUTE);
                                convertedUnits = true;
                            }
                            else if (type == "vector2")
                            {
                                mx::Vector2 originalval = value->asA<mx::Vector2>();
                                mx::Vector2 convertedValue = distanceConverter->convert(originalval, input->getUnit(), targetDistanceUnit);
                                input->setValue<mx::Vector2>(convertedValue);
                                input->removeAttribute(mx::ValueElement::UNIT_ATTRIBUTE);
                                input->removeAttribute(mx::ValueElement::UNITTYPE_ATTRIBUTE);
                                convertedUnits = true;
                            }
                            else if (type == "vector3")
                            {
                                mx::Vector3 originalval = value->asA<mx::Vector3>();
                                mx::Vector3 convertedValue = distanceConverter->convert(originalval, input->getUnit(), targetDistanceUnit);
                                input->setValue<mx::Vector3>(convertedValue);
                                input->removeAttribute(mx::ValueElement::UNIT_ATTRIBUTE);
                                input->removeAttribute(mx::ValueElement::UNITTYPE_ATTRIBUTE);
                                convertedUnits = true;
                            }
                            else if (type == "vector4")
                            {
                                mx::Vector4 originalval = value->asA<mx::Vector4>();
                                mx::Vector4 convertedValue = distanceConverter->convert(originalval, input->getUnit(), targetDistanceUnit);
                                input->setValue<mx::Vector4>(convertedValue);
                                input->removeAttribute(mx::ValueElement::UNIT_ATTRIBUTE);
                                input->removeAttribute(mx::ValueElement::UNITTYPE_ATTRIBUTE);
                                convertedUnits = true;
                            }
                        }
                    }
                }
            }
        }

        if (convertedUnits)
        {
            log << "- Performed inlined unit converstion" << std::endl;
        }
    }

    // Prepocess 2: Handle configurations that Arnold does not understand.
    // For now Arnold only handles rendering materials as roots. For now this means <surfacematerial>
    // materials nodes only.
    mx::NodePtr renderMaterial = nullptr;
    if (element->getType() == mx::SURFACE_SHADER_TYPE_STRING || 
        element->getType() == mx::DISPLACEMENT_SHADER_TYPE_STRING)
    {
        mx::NodePtr shaderNode = element->asA<mx::Node>();
        for (const mx::NodePtr& material : doc->getMaterialNodes())
        {
            for (auto shader : mx::getShaderNodes(material))
            {
                if (shader->getNamePath() == element->getNamePath())
                {
                    renderMaterial = material;
                    break;
                }
            }
        }

        if (!renderMaterial)
        {
            renderMaterial = doc->addMaterialNode(doc->createValidChildName(shaderName + "_material"), shaderNode);
        }
    }

    // Create an unlit shader node and corresponding material
    else 
    {
        mx::OutputPtr outputPtr = element->asA<mx::Output>();
        if (outputPtr)
        {
            mx::NodePtr renderShader = doc->addNode("unlit_surface", doc->createValidChildName(shaderName + "shader"), mx::SURFACE_SHADER_TYPE_STRING);
            std::cout << "--- Create dummy unlit shader: " << renderShader->getName() << std::endl;
            log << "--- Create dummy unlit shader: " << renderShader->getName() << std::endl;
            renderShader->setVersionString("1.0");
            mx::InputPtr input = renderShader->addInput("color", outputPtr->getType());
            mx::ElementPtr outputParent = outputPtr->getParent();                       
            if (outputParent->isA<mx::NodeGraph>())
            {
                mx::NodeGraphPtr outputGraph = outputParent->asA<mx::NodeGraph>();
                input->setNodeGraphString(outputGraph->getName());
                input->setOutputString(outputPtr->getName());
            }
            else
            {
                // This does not work for adsk_procedurals.mtlx
                // output is a top level output connected to a surface shader.
                // TODO: Fix this
                input->setOutputString(outputPtr->getName());
            }
            renderMaterial = doc->addMaterialNode(doc->createValidChildName(shaderName + "_material"), renderShader);
            mx::InputPtr displShaderInput = renderMaterial->addInput("displacementshader");
            displShaderInput->setType("displacementshader");
            displShaderInput->setValue(mx::EMPTY_STRING);
        }
    }

    // Early exit if can't set up with a <surfacematerial> configuration.
    // That is the any other element type will not work with the Arnold shader operator
    // so early exit.
    static mx::ImagePtr errorImage;
    if (!renderMaterial)
    {
        if (!errorImage)
        {
            mx::Color4 color(1.0f, 0.0f, 0.0f, 1.0f);
            errorImage = createUniformImage(1, 1, 4, mx::Image::BaseType::UINT8, color);
        }
        const std::string& arnoldShaderName = mx::FilePath(shaderName + "_arnold.png");
        mx::FilePath shaderPath = mx::FilePath(outputPath) / arnoldShaderName;
        _imageHandler->saveImage(shaderPath, errorImage);
        log << "- ERROR: Skip rendering of unsupported element: " + element->getNamePath() 
            << ". Wrote dummy image to: "  << shaderPath.asString() << std::endl;
        return true;
    }

    std::string validationString;
    if (!doc->validate(&validationString))
    {
        log << "Document is invalid: " << validationString << std::endl;
        std::cout << "Document is invalid: " << validationString << std::endl;
    }

    // Write modified for to a temp file and user that for rendering.
    mx::FilePath documentPath(doc->getSourceUri());
    documentPath.removeExtension();
    documentPath.addExtension(shaderName + "_arnold.xml");
    mx::writeToXmlFile(doc, documentPath);
    std::cout << "--- Wrote temp arnold mtlx file to: " << documentPath.asString() << std::endl;
    log << "--- Wrote temp arnold mtlx file to: " << documentPath.asString() << std::endl;

    std::vector<mx::GenOptions> optionsList;
    getGenerationOptions(testOptions, context.getOptions(), optionsList);

    if (element && doc)
    {
        const std::string ARNOLD_CRASH_STRING("CRASH");

        // Set up appropriate parameter inputs for kick
        log << "------------ Run Arnold validation with element: " << element->getNamePath() << "-------------------" << std::endl;
        mx::FilePath currentPath = mx::FilePath::getCurrentPath();
        mx::FilePath templateFile = currentPath / mx::FilePath("resources/Materials/TestSuite/Utilities/arnold_mtlxTemplate.ass");
        mx::FilePath geometryFile = currentPath / mx::FilePath("resources/Materials/TestSuite/Utilities/sphere.ass");
        mx::FilePath envMapFile = mx::FilePath::getCurrentPath() / testOptions.radianceIBLPath;
        std::string materialXOperator = "assign_material";
        std::string materialXOperatorAssignType = materialXOperator + ".assign_type";
        std::string materialXOperatorLook = materialXOperator + ".look";
        std::string cameraName = "unit_camera";
        const std::string resolutionString = " -r " + std::to_string(static_cast<int>(testOptions.renderSize[0])) + " " + std::to_string(static_cast<int>(testOptions.renderSize[1]));
        const std::string IMAGE_CODEC("png");

        for (const auto& options : optionsList)
        {
            profileTimes.elementsTested++;

            mx::ShaderPtr shader;
            try
            {
                RenderUtil::AdditiveScopedTimer genTimer(profileTimes.languageTimes.generationTime, "Arnold generation time");
                mx::GenOptions& contextOptions = context.getOptions();
                contextOptions = options;
                contextOptions.targetColorSpaceOverride = "lin_rec709";
                shader = shadergen.generate(shaderName, element, context);
            }
            catch (mx::Exception& e)
            {
                log << ">> " << e.what() << "\n";
                shader = nullptr;
            }
            CHECK(shader != nullptr);
            if (shader == nullptr)
            {
                log << ">> Failed to generate shader\n";
                return false;
            }
            CHECK(shader->getSourceCode().length() > 0);

            mx::FilePath shaderPath;
            mx::FilePath outputFilePath = outputPath;
            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
            }

            // Note: mkdir will fail if the directory already exists which is ok.
            {
                RenderUtil::AdditiveScopedTimer ioDir(profileTimes.languageTimes.ioTime, "Arnold dir time");
                outputFilePath.createDirectory();
            }

            const std::string& arnoldShaderName = mx::FilePath(shaderName + "_arnold");
            shaderPath = mx::FilePath(outputFilePath) / arnoldShaderName;

            // Write out osl file
            if (testOptions.dumpGeneratedCode || testOptions.renderImages)
            {
                RenderUtil::AdditiveScopedTimer ioTimer(profileTimes.languageTimes.ioTime, "Arnold I/O time");
                std::ofstream file;
                file.open(shaderPath.asString() + ".osl");
                file << shader->getSourceCode();
                file.close();
            }

            if (testOptions.renderImages)
            {
                // Run kick to test with template file:
                std::string testRenderer = mx::getEnviron("MATERIALX_ARNOLD_EXECUTABLE");
                if (testRenderer.empty())
                {
                    testRenderer = std::string(MATERIALX_ARNOLD_EXECUTABLE);
                }

                if (!testRenderer.empty())
                {
                    const std::string renderOSL = shaderPath.asString() + "." + IMAGE_CODEC;
                    const std::string inputArgs = " -ib -as 1 -i " + templateFile.asString();
                    const std::string outputArgs = resolutionString + " -of " + IMAGE_CODEC + " -dw -o " + renderOSL;
                    std::string setParameters;
                    setParameters += " -set " + materialXOperatorAssignType + " \"" + "material" + "\"";
                    setParameters += " -set " + materialXOperatorLook + " \"" + renderMaterial->getName() + "\"";
                    setParameters += " -set environment_map_name.filename \"" + envMapFile.asString() + "\"";
                    setParameters += " -set options.operator \"" + materialXOperator + "\"";
                    setParameters += " -set " + materialXOperator + ".filename \"" + documentPath.asString() + "\"";
                    setParameters += " -set options.texture_searchpath \"" + flattenSearchPath.asString() + "\"";
                    setParameters += " -set geometry_standin.filename \"" + geometryFile.asString() + "\"";
                    setParameters += " -set options.camera \"" + cameraName + "\"";                    

                    std::string errorFile(shaderPath.asString() + "arnold_errors.txt");
                    const std::string redirectString(" 2>&1");

                    std::string command =  testRenderer + inputArgs + setParameters + outputArgs
                        + " > " + errorFile + redirectString;
                    log << command << std::endl;
                    std::cout << command << std::endl;

                    // Set environment variable for definition search: ARNOLD_MATERIALX_NODE_DEFINITIONS
                    const std::string ARNOLD_MATERIALX_NODE_DEFINITIONS_ENV_VAR("ARNOLD_MATERIALX_NODE_DEFINITIONS");
                    const std::string& prevPath = mx::getEnviron(ARNOLD_MATERIALX_NODE_DEFINITIONS_ENV_VAR);
                    mx::FileSearchPath definitionSearchPath;
                    definitionSearchPath.append(mx::FilePath::getCurrentPath() / mx::FilePath("libraries"));

                    // Switch to path where shader is before running and then switch back.
                    int returnValue = -1;
                    std::string result;
                    if (outputFilePath.setCurrentPath() && 
                        mx::setEnviron(ARNOLD_MATERIALX_NODE_DEFINITIONS_ENV_VAR, definitionSearchPath.asString()))
                    {
                        returnValue = std::system(command.c_str());
                        currentPath.setCurrentPath();
                        mx::setEnviron(ARNOLD_MATERIALX_NODE_DEFINITIONS_ENV_VAR, prevPath);

                        std::ifstream errorStream(errorFile);
                        result.assign(std::istreambuf_iterator<char>(errorStream),
                            std::istreambuf_iterator<char>());
                    }
                    else
                    {
                        result = "Failed to set test path: " + outputFilePath.asString();
                    }

                    if (!result.empty())
                    {
                        const std::string errorType("Arnold log.");
                        mx::StringVec errors;
                        errors.push_back("Command string: " + command);
                        errors.push_back("Command return code: " + std::to_string(returnValue));
                        errors.push_back(result);

                        // On failure, write out a dummy image.
                        if (returnValue != 0 ||
                            result.find(ARNOLD_CRASH_STRING) != std::string::npos)
                        {
                            if (!errorImage)
                            {
                                mx::Color4 color(0.0f, 0.0f, 1.0f, 1.0f);
                                errorImage = createUniformImage(1, 1, 4, mx::Image::BaseType::UINT8, color);
                            }
                            _imageHandler->saveImage(renderOSL, errorImage);
                            log << "ERROR: Failed rendering of element: " + element->getNamePath() 
                                << ". Wrote dummy image to: " << renderOSL << std::endl;
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

TEST_CASE("Render: Arnold TestSuite", "[renderarnold]")
{
    ArnoldShaderRenderTester renderTester;

    const mx::FilePath currentPath = mx::FilePath::getCurrentPath();
    const mx::FilePath testRootPath = currentPath / mx::FilePath("resources/Materials/TestSuite");
    const mx::FilePath testRootPath2 = currentPath / mx::FilePath("resources/Materials/Examples/StandardSurface");
    const mx::FilePath testRootPath3 = currentPath / mx::FilePath("resources/Materials/Examples/UsdPreviewSurface");
    mx::FilePathVec testRootPaths;
    testRootPaths.push_back(testRootPath);
    testRootPaths.push_back(testRootPath2);
    testRootPaths.push_back(testRootPath3);

    mx::FilePath optionsFilePath = testRootPath / mx::FilePath("_options.mtlx");

    renderTester.validate(testRootPaths, optionsFilePath);
}
