//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#if defined(MATERIALX_BUILD_RENDER) 

// Run only on supported platforms
#include <MaterialXRender/HardwarePlatform.h>
#if defined(OSWin_) || defined(OSLinux_) || defined(OSMac_)

#include <MaterialXTest/Catch/catch.hpp>
#include <MaterialXTest/GenShaderUtil.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/Util.h>

#ifdef MATERIALX_BUILD_RENDERGLSL
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXRenderGlsl/GlslValidator.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>
#endif

#ifdef MATERIALX_BUILD_RENDEROSL
#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXRenderOsl/OslValidator.h>
#endif

#ifdef MATERIALX_BUILD_OIIO
#include <MaterialXRender/OiioImageLoader.h>
#endif
#include <MaterialXRender/StbImageLoader.h>

#include <MaterialXRender/GeometryHandler.h>
#include <MaterialXRender/TinyObjLoader.h>

#include <fstream>
#include <iostream>
#include <unordered_set>
#include <chrono>
#include <ctime>

namespace mx = MaterialX;

#define LOG_TO_FILE

void createLightRig(mx::DocumentPtr doc, mx::LightHandler& lightHandler, mx::GenContext& context,
                    const mx::FilePath& envIrradiancePath, const mx::FilePath& envRadiancePath)
{
    // Scan for lights
    std::vector<mx::NodePtr> lights;
    lightHandler.findLights(doc, lights);
    lightHandler.registerLights(doc, lights, context);

    // Set the list of lights on the with the generator
    lightHandler.setLightSources(lights);
    // Set up IBL inputs
    lightHandler.setLightEnvIrradiancePath(envIrradiancePath);
    lightHandler.setLightEnvRadiancePath(envRadiancePath);
}


#ifdef MATERIALX_BUILD_RENDERGLSL
//
// Create a validator with an image and geometry handler
// If a filename is supplied then a stock geometry of that name will be used if it can be loaded.
// By default if the file can be loaded it is assumed that rendering is done using a perspective
// view vs an orthographic view. This flag argument is updated and returned.
//
static mx::GlslValidatorPtr createGLSLValidator(const std::string& fileName, std::ostream& log)
{
    bool initialized = false;
    mx::GlslValidatorPtr validator = mx::GlslValidator::create();
    mx::StbImageLoaderPtr stbLoader = mx::StbImageLoader::create();
    mx::GLTextureHandlerPtr imageHandler = mx::GLTextureHandler::create(stbLoader);
    try
    {
        validator->initialize();
        validator->setImageHandler(imageHandler);
        validator->setLightHandler(nullptr);
        mx::GeometryHandlerPtr geometryHandler = validator->getGeometryHandler();
        std::string geometryFile;
        if (fileName.length())
        {
            geometryFile = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry/") / mx::FilePath(fileName);
            if (!geometryHandler->hasGeometry(geometryFile))
            {
                geometryHandler->clearGeometry();
                geometryHandler->loadGeometry(geometryFile);
            }
        }
        initialized = true;
    }
    catch (mx::ExceptionShaderValidationError& e)
    {
        for (auto error : e.errorLog())
        {
            log << e.what() << " " << error << std::endl;
        }
    }
    catch (mx::Exception& e)
    {
        log << e.what() << std::endl;
    }
    REQUIRE(initialized);

    return validator;
}
#endif

#ifdef MATERIALX_BUILD_RENDEROSL
static mx::OslValidatorPtr createOSLValidator(std::ostream& log)
{
    bool initialized = false;

    mx::OslValidatorPtr validator = mx::OslValidator::create();
    const std::string oslcExecutable(MATERIALX_OSLC_EXECUTABLE);
    validator->setOslCompilerExecutable(oslcExecutable);
    const std::string testRenderExecutable(MATERIALX_TESTRENDER_EXECUTABLE);
    validator->setOslTestRenderExecutable(testRenderExecutable);
    validator->setOslIncludePath(mx::FilePath(MATERIALX_OSL_INCLUDE_PATH));
    try
    {
        validator->initialize();
        validator->setImageHandler(nullptr);
        validator->setLightHandler(nullptr);
        initialized = true;

        // Pre-compile some required shaders for testrender
        if (!oslcExecutable.empty() && !testRenderExecutable.empty())
        {
            mx::FilePath shaderPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/Utilities/");
            validator->setOslOutputFilePath(shaderPath);

            const std::string OSL_EXTENSION("osl");
            mx::FilePathVec files = shaderPath.getFilesInDirectory(OSL_EXTENSION);
            for (auto file : files)
            {
                mx::FilePath filePath = shaderPath / file;
                validator->compileOSL(filePath.asString());
            }

            // Set the search path for these compiled shaders.
            validator->setOslUtilityOSOPath(shaderPath);
        }
    }
    catch (mx::ExceptionShaderValidationError& e)
    {
        for (auto error : e.errorLog())
        {
            log << e.what() << " " << error << std::endl;
        }
    }
    REQUIRE(initialized);

    return validator;
}
#endif

//
// Shader validation options structure
//
class ShaderValidTestOptions
{
  public:
    void print(std::ostream& output) const
    {
        output << "Shader Validation Test Options:" << std::endl;
        output << "\tOverride Files: ";
        for (auto overrideFile : overrideFiles)
        {
            output << overrideFile << " ";
        }
        output << std::endl;
        output << "\tLight Setup Files: ";
        for (auto lightFile : lightFiles)
        {
            output << lightFile << " ";
        }
        output << std::endl;
        output << "\tRun GLSL Tests: " << runGLSLTests << std::endl;
        output << "\tRun OSL Tests: " << runOSLTests << std::endl;
        output << "\tCheck Implementation Usage Count: " << checkImplCount << std::endl;
        output << "\tDump Generated Code: " << dumpGeneratedCode << std::endl;
        output << "\tShader Interfaces: " << shaderInterfaces << std::endl;
        output << "\tValidate Element To Render: " << validateElementToRender << std::endl;
        output << "\tCompile code: " << compileCode << std::endl;
        output << "\tRender Images: " << renderImages << std::endl;
        output << "\tSave Images: " << saveImages << std::endl;
        output << "\tDump GLSL Uniforms and Attributes  " << dumpGlslUniformsAndAttributes << std::endl;
        output << "\tGLSL Non-Shader Geometry: " << glslNonShaderGeometry.asString() << std::endl;
        output << "\tGLSL Shader Geometry: " << glslShaderGeometry.asString() << std::endl;
        output << "\tRadiance IBL File Path " << radianceIBLPath.asString() << std::endl;
        output << "\tIrradiance IBL File Path: " << irradianceIBLPath.asString() << std::endl;
    }

    // Filter list of files to only run validation on.
    MaterialX::StringVec overrideFiles;

    // Comma separated list of light setup files
    mx::StringVec lightFiles;

    // Set to true to always dump generated code to disk
    bool dumpGeneratedCode = false;

    // Execute GLSL tests
    bool runGLSLTests = true;

    // Execute OSL tests
    bool runOSLTests = true;

    // Check the count of number of implementations used
    bool checkImplCount = true;

    // Run using a set of interfaces:
    // - 3 = run complete + reduced.
    // - 2 = run complete only (default)
    // - 1 = run reduced only.
    int shaderInterfaces = 2;

    // Validate element before attempting to generate code. Default is false.
    bool validateElementToRender = false;

    // Perform source code compilation validation test
    bool compileCode = true;

    // Perform rendering validation test
    bool renderImages = true;

    // Perform saving of image. Can only be disabled for GLSL tests.
    bool saveImages = true;

    // Set this to be true if it is desired to dump out GLSL uniform and attribut information to the logging file.
    bool dumpGlslUniformsAndAttributes = true;

    // Non-shader GLSL geometry file
    MaterialX::FilePath glslNonShaderGeometry;

    // Shader GLSL geometry file
    MaterialX::FilePath glslShaderGeometry;

    // Radiance IBL file
    MaterialX::FilePath radianceIBLPath;

    // IradianceIBL file
    MaterialX::FilePath irradianceIBLPath;
};

// Per language profile times
class LanguageProfileTimes
{
public:
    void print(const std::string& label, std::ostream& output) const
    {
        output << label << std::endl;
        output << "\tTotal: " << totalTime << " seconds" << std::endl;;
        output << "\tSetup: " << setupTime << " seconds" << std::endl;;
        output << "\tTransparency: " << transparencyTime << " seconds" << std::endl;;
        output << "\tGeneration: " << generationTime << " seconds" << std::endl;;
        output << "\tCompile: " << compileTime << " seconds" << std::endl;
        output << "\tRender: " << renderTime << " seconds" << std::endl;
        output << "\tI/O: " << ioTime << " seconds" << std::endl;
        output << "\tImage save: " << imageSaveTime << " seconds" << std::endl;
    }
    double totalTime = 0.0;
    double setupTime = 0.0;
    double transparencyTime = 0.0;
    double generationTime = 0.0;
    double compileTime = 0.0;
    double renderTime = 0.0;
    double ioTime = 0.0;
    double imageSaveTime = 0.0;
};

//
// Shader validation profiling structure
//
class ShaderValidProfileTimes
{
public:
    void print(std::ostream& output) const
    {
        output << "Overall time: " << totalTime << " seconds" << std::endl;
        output << "\tOverhead time: " << (totalTime - glslTimes.totalTime - oslTimes.totalTime) << " seconds" << std::endl;
        output << "\tI/O time: " << ioTime << " seconds" << std::endl;
        output << "\tValidation time: " << validateTime << " seconds" << std::endl;
        output << "\tRenderable search time: " << renderableSearchTime << " seconds" << std::endl;

        glslTimes.print("GLSL Profile Times:", output);
        oslTimes.print("OSL Profile Times:", output);

        output << "Elements tested: " << elementsTested << std::endl;
    }

    LanguageProfileTimes glslTimes;
    LanguageProfileTimes oslTimes;
    double totalTime = 0;
    double ioTime = 0.0;
    double validateTime = 0.0;
    double renderableSearchTime = 0.0;
    unsigned int elementsTested = 0;
};

// Scoped timer which adds a duration to a given externally reference timing duration
class AdditiveScopedTimer
{
public:
    AdditiveScopedTimer(double& durationRefence, const std::string& label)
        : _duration(durationRefence)
        , _label(label)
    {
        startTimer();
    }

    ~AdditiveScopedTimer()
    {
        endTimer();
    }

    void startTimer()
    {
        _startTime = std::chrono::system_clock::now();

        if (_debugUpdate)
        {
            std::cout << "Start time for timer (" << _label << ") is: " << _duration << std::endl;
        }
    }

    void endTimer()
    {
        std::chrono::time_point<std::chrono::system_clock> endTime = std::chrono::system_clock::now();
        std::chrono::duration<double> timeDuration = endTime - _startTime;
        double currentDuration = timeDuration.count();
        _duration += currentDuration;
        _startTime = endTime;

        if (_debugUpdate)
        {
            std::cout << "Current duration for timer (" << _label << ") is: " << currentDuration << ". Total duration: " << _duration << std::endl;
        }
    }

protected:
    double& _duration;
    bool _debugUpdate = false;
    std::string _label;
    std::chrono::time_point<std::chrono::system_clock> _startTime;
};

// Create a list of generation options based on unit test options
// These options will override the original generation context options.
void getGenerationOptions(const ShaderValidTestOptions& testOptions, 
                          const mx::GenOptions& originalOptions, 
                          std::vector<mx::GenOptions>& optionsList)
{
    optionsList.clear();
    if (testOptions.shaderInterfaces & 1)
    {
        mx::GenOptions reducedOption = originalOptions;
        reducedOption.shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;
        optionsList.push_back(reducedOption);
    }
    // Alway fallback to complete if no options specified.
    if ((testOptions.shaderInterfaces & 2) || optionsList.empty())
    {
        mx::GenOptions completeOption = originalOptions;
        completeOption.shaderInterfaceType = mx::SHADER_INTERFACE_COMPLETE;
        optionsList.push_back(completeOption);
    }
}

#ifdef MATERIALX_BUILD_RENDERGLSL
// Test by connecting it to a supplied element
// 1. Create the shader and checks for source generation
// 2. Writes doc to disk if valid
// 3. Writes vertex and pixel shaders to disk
// 4. Validates creation / compilation of shader program
// 5. Validates that inputs were created properly
// 6. Validates rendering
// 7. Saves rendered image to disk
//
// Outputs error log if validation fails
//
static void runGLSLValidation(const std::string& shaderName, mx::TypedElementPtr element, mx::GlslValidator& validator,
                              mx::GenContext& context, const mx::LightHandlerPtr lightHandler,
                              mx::DocumentPtr doc, std::ostream& log, const ShaderValidTestOptions& testOptions, ShaderValidProfileTimes& profileTimes,
                              const mx::FileSearchPath& imageSearchPath, const std::string& outputPath=".")
{
    AdditiveScopedTimer totalGLSLTime(profileTimes.glslTimes.totalTime, "GLSL total time");

    const mx::ShaderGenerator& shadergen = context.getShaderGenerator();

    // Perform validation if requested
    if (testOptions.validateElementToRender)
    {
        std::string message;
        if (!element->validate(&message))
        {
            log << "Element is invalid: " << message << std::endl;
            return;
        }
    }

    std::vector<mx::GenOptions> optionsList;
    getGenerationOptions(testOptions, context.getOptions(), optionsList);

    if (element && doc)
    {
        log << "------------ Run GLSL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (auto options : optionsList)
        {
            profileTimes.elementsTested++;

            mx::FilePath outputFilePath = outputPath;
            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
            }

            // Note: mkdir will fail if the directory already exists which is ok.
            {
                AdditiveScopedTimer ioDir(profileTimes.glslTimes.ioTime, "GLSL dir time");
                outputFilePath.createDirectory();
            }

            std::string shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);
            mx::ShaderPtr shader;
            try
            {
                AdditiveScopedTimer transpTimer(profileTimes.glslTimes.transparencyTime, "GLSL transparency time");
                options.hwTransparency = mx::isTransparentSurface(element, shadergen);
                transpTimer.endTimer();

                AdditiveScopedTimer generationTimer(profileTimes.glslTimes.generationTime, "GLSL generation time");
                mx::GenOptions& contextOptions = context.getOptions();
                contextOptions = options;
                contextOptions.fileTextureVerticalFlip = true;
                shader = shadergen.generate(shaderName, element, context);
                generationTimer.endTimer();
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
                return;
            }
            const std::string& vertexSourceCode = shader->getSourceCode(mx::Stage::VERTEX);
            const std::string& pixelSourceCode = shader->getSourceCode(mx::Stage::PIXEL);
            CHECK(vertexSourceCode.length() > 0);
            CHECK(pixelSourceCode.length() > 0);

            if (testOptions.dumpGeneratedCode)
            {
                AdditiveScopedTimer dumpTimer(profileTimes.glslTimes.ioTime, "GLSL io time");
                std::ofstream file;
                file.open(shaderPath + "_vs.glsl");
                file << vertexSourceCode;
                file.close();
                file.open(shaderPath + "_ps.glsl");
                file << pixelSourceCode;
                file.close();
            }

            if (!testOptions.compileCode)
            {
                return;
            }

            // Validate
            MaterialX::GlslProgramPtr program = validator.program();
            bool validated = false;
            try
            {
                mx::GeometryHandlerPtr geomHandler = validator.getGeometryHandler();

                bool isShader = mx::elementRequiresShading(element);
                if (isShader)
                {
                    mx::FilePath geomPath;
                    if (!testOptions.glslShaderGeometry.isEmpty())
                    {
                        if (!testOptions.glslShaderGeometry.isAbsolute())
                        {
                            geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry") / testOptions.glslShaderGeometry;
                        }
                        else
                        {
                            geomPath = testOptions.glslShaderGeometry;
                        }
                    }
                    else
                    {
                        geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry/shaderball.obj");
                    }
                    if (!geomHandler->hasGeometry(geomPath))
                    {
                        geomHandler->clearGeometry();
                        geomHandler->loadGeometry(geomPath);
                    }
                    validator.setLightHandler(lightHandler);
                }
                else
                {
                    mx::FilePath geomPath;
                    if (!testOptions.glslNonShaderGeometry.isEmpty())
                    {
                        if (!testOptions.glslNonShaderGeometry.isAbsolute())
                        {
                            geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry") / testOptions.glslNonShaderGeometry;
                        }
                        else
                        {
                            geomPath = testOptions.glslNonShaderGeometry;
                        }
                    }
                    else
                    {
                        geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry/sphere.obj");
                    }
                    if (!geomHandler->hasGeometry(geomPath))
                    {
                        geomHandler->clearGeometry();
                        geomHandler->loadGeometry(geomPath);
                    }
                    validator.setLightHandler(nullptr);
                }

                {
                    AdditiveScopedTimer compileTimer(profileTimes.glslTimes.compileTime, "GLSL compile time");
                    validator.validateCreation(shader);
                    validator.validateInputs();
                }

                if (testOptions.dumpGlslUniformsAndAttributes)
                {
                    AdditiveScopedTimer printTimer(profileTimes.glslTimes.ioTime, "GLSL io time");
                    log << "* Uniform:" << std::endl;
                    program->printUniforms(log);
                    log << "* Attributes:" << std::endl;
                    program->printAttributes(log);

                    log << "* Uniform UI Properties:" << std::endl;
                    const std::string& target = shadergen.getTarget();
                    const MaterialX::GlslProgram::InputMap& uniforms = program->getUniformsList();
                    for (auto uniform : uniforms)
                    {
                        const std::string& path = uniform.second->path;
                        if (path.empty())
                        {
                            continue;
                        }

                        mx::UIProperties uiProperties;
                        if (getUIProperties(path, doc, target, uiProperties) > 0)
                        {
                            log << "Program Uniform: " << uniform.first << ". Path: " << path;
                            if (!uiProperties.uiName.empty())
                                log << ". UI Name: \"" << uiProperties.uiName << "\"";
                            if (!uiProperties.uiFolder.empty())
                                log << ". UI Folder: \"" << uiProperties.uiFolder << "\"";
                            if (!uiProperties.enumeration.empty())
                            {
                                log << ". Enumeration: {";
                                for (size_t i = 0; i<uiProperties.enumeration.size(); i++)
                                    log << uiProperties.enumeration[i] << " ";
                                log << "}";
                            }
                            if (!uiProperties.enumerationValues.empty())
                            {
                                log << ". Enum Values: {";
                                for (size_t i = 0; i < uiProperties.enumerationValues.size(); i++)
                                    log << uiProperties.enumerationValues[i]->getValueString() << "; ";
                                log << "}";
                            }
                            if (uiProperties.uiMin)
                                log << ". UI Min: " << uiProperties.uiMin->getValueString();
                            if (uiProperties.uiMax)
                                log << ". UI Max: " << uiProperties.uiMax->getValueString();
                            log << std::endl;
                        }
                    }
                }

                if (testOptions.renderImages)
                {
                    {
                        AdditiveScopedTimer renderTimer(profileTimes.glslTimes.renderTime, "GLSL render time");
                        validator.getImageHandler()->setSearchPath(imageSearchPath);
                        validator.validateRender(!isShader);
                    }

                    if (testOptions.saveImages)
                    {
                        AdditiveScopedTimer ioTimer(profileTimes.glslTimes.imageSaveTime, "GLSL image save time");
                        std::string fileName = shaderPath + "_glsl.png";
                        validator.save(fileName, false);
                    }
                }

                validated = true;
            }
            catch (mx::ExceptionShaderValidationError& e)
            {
                // Always dump shader stages on error
                std::ofstream file;
                file.open(shaderPath + "_vs.glsl");
                file << shader->getSourceCode(mx::Stage::VERTEX);
                file.close();
                file.open(shaderPath + "_ps.glsl");
                file << shader->getSourceCode(mx::Stage::PIXEL);
                file.close();

                for (auto error : e.errorLog())
                {
                    log << e.what() << " " << error << std::endl;
                }
                log << ">> Refer to shader code in dump files: " << shaderPath << "_ps.glsl and _vs.glsl files" << std::endl;
            }
            catch (mx::Exception& e)
            {
                log << e.what() << std::endl;
            }
            CHECK(validated);
        }
    }
}
#endif

#ifdef MATERIALX_BUILD_RENDEROSL
static void runOSLValidation(const std::string& shaderName, mx::TypedElementPtr element,
                             mx::OslValidator& validator, mx::GenContext& context, mx::DocumentPtr doc,
                             std::ostream& log, const ShaderValidTestOptions& testOptions, ShaderValidProfileTimes& profileTimes,
                             const mx::FileSearchPath& imageSearchPath, const std::string& outputPath=".")
{
    AdditiveScopedTimer totalOSLTime(profileTimes.oslTimes.totalTime, "OSL total time");

    const mx::ShaderGenerator& shadergen = context.getShaderGenerator();

    // Perform validation if requested
    if (testOptions.validateElementToRender)
    {
        std::string message;
        if (!element->validate(&message))
        {
            log << "Element is invalid: " << message << std::endl;
            return;
        }
    }

    std::vector<mx::GenOptions> optionsList;
    getGenerationOptions(testOptions, context.getOptions(), optionsList);

    if (element && doc)
    {
        log << "------------ Run OSL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (auto options : optionsList)
        {
            profileTimes.elementsTested++;

            mx::ShaderPtr shader;
            try
            {
                AdditiveScopedTimer genTimer(profileTimes.oslTimes.generationTime, "OSL generation time");
                mx::GenOptions& contextOptions = context.getOptions();
                contextOptions = options;
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
                return;
            }
            CHECK(shader->getSourceCode().length() > 0);

            std::string shaderPath;
            mx::FilePath outputFilePath = outputPath;
            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
            }

            // Note: mkdir will fail if the directory already exists which is ok.
            {
                AdditiveScopedTimer ioDir(profileTimes.oslTimes.ioTime, "OSL dir time");
                outputFilePath.createDirectory();
            }

            shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);

            // Write out osl file
            if (testOptions.dumpGeneratedCode)
            {
                AdditiveScopedTimer ioTimer(profileTimes.oslTimes.ioTime, "OSL io time");
                std::ofstream file;
                file.open(shaderPath + ".osl");
                file << shader->getSourceCode();
                file.close();
            }

            if (!testOptions.compileCode)
            {
                return;
            }

            // Validate
            bool validated = false;
            try
            {
                // Set output path and shader name
                validator.setOslOutputFilePath(outputFilePath);
                validator.setOslShaderName(shaderName);

                // Validate compilation
                {
                    AdditiveScopedTimer compileTimer(profileTimes.oslTimes.compileTime, "OSL compile time");
                    validator.validateCreation(shader);
                }

                if (testOptions.renderImages)
                {
                    const mx::ShaderStage& stage = shader->getStage(mx::Stage::PIXEL);

                    // Look for textures and build parameter override string for each image
                    // files if a relative path maps to an absolute path
                    const mx::VariableBlock& uniforms = stage.getUniformBlock(mx::OSL::UNIFORMS);

                    mx::StringVec overrides;
                    mx::StringMap separatorMapper;
                    separatorMapper["\\\\"] = "/";
                    separatorMapper["\\"] = "/";
                    for (size_t i = 0; i<uniforms.size(); ++i)
                    {
                        const mx::ShaderPort* uniform = uniforms[i];
                        if (uniform->getType() != MaterialX::Type::FILENAME)
                        {
                            continue;
                        }
                        if (uniform->getValue())
                        {
                            const std::string& uniformName = uniform->getName();
                            mx::FilePath filename;
                            mx::FilePath origFilename(uniform->getValue()->getValueString());
                            if (!origFilename.isAbsolute())
                            {
                                filename = imageSearchPath.find(origFilename);
                                if (filename != origFilename)
                                {
                                    std::string overrideString("string " + uniformName + " \"" + filename.asString() + "\";\n");
                                    overrideString = mx::replaceSubstrings(overrideString, separatorMapper);
                                    overrides.push_back(overrideString);
                                }
                            }
                        }
                    }
                    validator.setShaderParameterOverrides(overrides);

                    const mx::VariableBlock& outputs = stage.getOutputBlock(mx::OSL::OUTPUTS);
                    if (outputs.size() > 0)
                    {
                        const mx::ShaderPort* output = outputs[0];
                        const mx::TypeSyntax& typeSyntax = shadergen.getSyntax().getTypeSyntax(output->getType());

                        const std::string& outputName = output->getName();
                        const std::string& outputType = typeSyntax.getTypeAlias().empty() ? typeSyntax.getName() : typeSyntax.getTypeAlias();

                        static const std::string SHADING_SCENE_FILE = "closure_color_scene.xml";
                        static const std::string NON_SHADING_SCENE_FILE = "constant_color_scene.xml";
                        const std::string& sceneTemplateFile = mx::elementRequiresShading(element) ? SHADING_SCENE_FILE : NON_SHADING_SCENE_FILE;

                        // Set shader output name and type to use
                        validator.setOslShaderOutput(outputName, outputType);

                        // Set scene template file. For now we only have the constant color scene file
                        mx::FilePath sceneTemplatePath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/Utilities/");
                        sceneTemplatePath = sceneTemplatePath / sceneTemplateFile;
                        validator.setOslTestRenderSceneTemplateFile(sceneTemplatePath.asString());

                        // Validate rendering
                        {
                            AdditiveScopedTimer renderTimer(profileTimes.oslTimes.renderTime, "OSL render time");
                            validator.validateRender();
                        }
                    }
                    else
                    {
                        CHECK(false);
                        log << ">> Shader has no output to render from\n";
                    }
                }

                validated = true;
            }
            catch (mx::ExceptionShaderValidationError& e)
            {
                // Always dump shader on error
                std::ofstream file;
                file.open(shaderPath + ".osl");
                file << shader->getSourceCode();
                file.close();

                for (auto error : e.errorLog())
                {
                    log << e.what() << " " << error << std::endl;
                }
                log << ">> Refer to shader code in dump file: " << shaderPath << ".osl file" << std::endl;
            }
            catch (mx::Exception& e)
            {
                std::cout << e.what();
            }
            CHECK(validated);
        }
    }
}
#endif


bool getTestOptions(const std::string& optionFile, ShaderValidTestOptions& options)
{
    const std::string SHADER_VALID_TEST_OPTIONS_STRING("ShaderValidTestOptions");
    const std::string OVERRIDE_FILES_STRING("overrideFiles");
    const std::string LIGHT_FILES_STRING("lightFiles");
    const std::string SHADER_INTERFACES_STRING("shaderInterfaces");
    const std::string VALIDATE_ELEMENT_TO_RENDER_STRING("validateElementToRender");
    const std::string COMPILE_CODE_STRING("compileCode");
    const std::string RENDER_IMAGES_STRING("renderImages");
    const std::string SAVE_IMAGES_STRING("saveImages");
    const std::string DUMP_GLSL_UNIFORMS_AND_ATTRIBUTES_STRING("dumpGlslUniformsAndAttributes");
    const std::string RUN_OSL_TESTS_STRING("runOSLTests");
    const std::string RUN_GLSL_TESTS_STRING("runGLSLTests");
    const std::string CHECK_IMPL_COUNT_STRING("checkImplCount");
    const std::string DUMP_GENERATED_CODE_STRING("dumpGeneratedCode");
    const std::string GLSL_NONSHADER_GEOMETRY_STRING("glslNonShaderGeometry");
    const std::string GLSL_SHADER_GEOMETRY_STRING("glslShaderGeometry");
    const std::string RADIANCE_IBL_PATH_STRING("radianceIBLPath");
    const std::string IRRADIANCE_IBL_PATH_STRING("irradianceIBLPath");
    const std::string SPHERE_OBJ("sphere.obj");
    const std::string SHADERBALL_OBJ("shaderball.obj");

    options.overrideFiles.clear();
    options.dumpGeneratedCode = false;
    options.glslNonShaderGeometry = SPHERE_OBJ;
    options.glslShaderGeometry = SHADERBALL_OBJ;

    MaterialX::DocumentPtr doc = MaterialX::createDocument();
    try {
        MaterialX::readFromXmlFile(doc, optionFile);

        MaterialX::NodeDefPtr optionDefs = doc->getNodeDef(SHADER_VALID_TEST_OPTIONS_STRING);
        if (optionDefs)
        {
            for (MaterialX::ParameterPtr p : optionDefs->getParameters())
            {
                const std::string& name = p->getName();
                MaterialX::ValuePtr val = p->getValue();
                if (val)
                {
                    if (name == OVERRIDE_FILES_STRING)
                    {
                        options.overrideFiles = MaterialX::splitString(p->getValueString(), ",");
                    }
                    if (name == LIGHT_FILES_STRING)
                    {
                        options.lightFiles = MaterialX::splitString(p->getValueString(), ",");
                    }
                    else if (name == SHADER_INTERFACES_STRING)
                    {
                        options.shaderInterfaces = val->asA<int>();
                    }
                    else if (name == VALIDATE_ELEMENT_TO_RENDER_STRING)
                    {
                        options.validateElementToRender = val->asA<bool>();
                    }
                    else if (name == COMPILE_CODE_STRING)
                    {
                        options.compileCode = val->asA<bool>();
                    }
                    else if (name == RENDER_IMAGES_STRING)
                    {
                        options.renderImages = val->asA<bool>();
                    }
                    else if (name == SAVE_IMAGES_STRING)
                    {
                        options.saveImages = val->asA<bool>();
                    }
                    else if (name == DUMP_GLSL_UNIFORMS_AND_ATTRIBUTES_STRING)
                    {
                        options.dumpGlslUniformsAndAttributes = val->asA<bool>();
                    }
                    else if (name == RUN_OSL_TESTS_STRING)
                    {
                        options.runOSLTests = val->asA<bool>();
                    }
                    else if (name == RUN_GLSL_TESTS_STRING)
                    {
                        options.runGLSLTests = val->asA<bool>();
                    }
                    else if (name == CHECK_IMPL_COUNT_STRING)
                    {
                        options.checkImplCount = val->asA<bool>();
                    }
                    else if (name == DUMP_GENERATED_CODE_STRING)
                    {
                        options.dumpGeneratedCode = val->asA<bool>();
                    }
                    else if (name == GLSL_NONSHADER_GEOMETRY_STRING)
                    {
                        options.glslNonShaderGeometry = p->getValueString();
                    }
                    else if (name == GLSL_SHADER_GEOMETRY_STRING)
                    {
                        options.glslShaderGeometry = p->getValueString();
                    }
                    else if (name == RADIANCE_IBL_PATH_STRING)
                    {
                        options.radianceIBLPath = p->getValueString();
                    }
                    else if (name == IRRADIANCE_IBL_PATH_STRING)
                    {
                        options.irradianceIBLPath = p->getValueString();
                    }
                }
            }
        }

        // Disable render and save of images if not compiled code will be generated
        if (!options.compileCode)
        {
            options.renderImages = false;
            options.saveImages = false;
        }
        // Disable saving images, if no images are to be produced
        if (!options.renderImages)
        {
            options.saveImages = false;
        }

        // If there is a filter on the files to run turn off profile checking
        if (!options.overrideFiles.empty())
        {
            options.checkImplCount = false;
        }

        // If implementation count check is required, then OSL and GLSL
        // code generation must be executed to be able to check implementation usage.
        if (options.checkImplCount)
        {
            options.runGLSLTests = true;
            options.runOSLTests = true;
        }
        return true;
    }
    catch (mx::Exception& e)
    {
        std::cout << e.what();
    }
    return false;
}

void printRunLog(const ShaderValidProfileTimes &profileTimes, const ShaderValidTestOptions& options,
                mx::StringSet& usedImpls, std::ostream& profilingLog, mx::DocumentPtr dependLib
#ifdef MATERIALX_BUILD_RENDEROSL
    , mx::GenContext& oslContext
#endif
#ifdef MATERIALX_BUILD_RENDERGLSL
    , mx::GenContext& glslContext
#endif
)
{
    profileTimes.print(profilingLog);

    profilingLog << "---------------------------------------" << std::endl;
    options.print(profilingLog);

    if (options.checkImplCount)
    {
        profilingLog << "---------------------------------------" << std::endl;

        // Get implementation count from libraries. 
        std::set<mx::ImplementationPtr> libraryImpls;
        const std::vector<mx::ElementPtr>& children = dependLib->getChildren();
        for (auto child : children)
        {
            mx::ImplementationPtr impl = child->asA<mx::Implementation>();
            if (!impl)
            {
                continue;
            }

            // Only check implementations for languages we're interested in and
            // are testing.
            // 
            if ((options.runGLSLTests && impl->getLanguage() == mx::GlslShaderGenerator::LANGUAGE) ||
                (options.runOSLTests && impl->getLanguage() == mx::OslShaderGenerator::LANGUAGE))
            {
                libraryImpls.insert(impl);
            }
        }

        size_t skipCount = 0;
        profilingLog << "-- Possibly missed implementations ----" << std::endl;
        mx::StringVec whiteList =
        {
            "ambientocclusion", "arrayappend", "backfacing", "screen", "curveadjust", "displacementshader",
            "volumeshader", "IM_constant_", "IM_dot_", "IM_geomattrvalue"
        };
        unsigned int implementationUseCount = 0;
        for (auto libraryImpl : libraryImpls)
        {
            const std::string& implName = libraryImpl->getName();

            // Skip white-list items
            bool inWhiteList = false;
            for (auto w : whiteList)
            {
                if (implName.find(w) != std::string::npos)
                {
                    skipCount++;
                    inWhiteList = true;
                    break;
                }
            }
            if (inWhiteList)
            {
                implementationUseCount++;
                continue;
            }

            if (usedImpls.count(implName))
            {
                implementationUseCount++;
                continue;
            }

#ifdef MATERIALX_BUILD_RENDEROSL
            if (oslContext.findNodeImplementation(implName))
            {
                implementationUseCount++;
                continue;
            }
#endif
#ifdef MATERIALX_BUILD_RENDERGLSL
            if (glslContext.findNodeImplementation(implName))
            {
                implementationUseCount++;
                continue;
            }
#endif
            profilingLog << "\t" << implName << std::endl;
        }
        size_t libraryCount = libraryImpls.size();
        profilingLog << "Tested: " << implementationUseCount << " out of: " << libraryCount << " library implementations." << std::endl;
        // Enable when implementations and testing are complete
        // CHECK(implementationUseCount == libraryCount);
    }
}

struct GeomHandlerTestOptions
{
    mx::GeometryHandlerPtr geomHandler;
    std::ofstream* logFile;

    mx::StringSet testExtensions;
    mx::StringVec skipExtensions;
};

void testGeomHandler(GeomHandlerTestOptions& options)
{
    mx::FilePath imagePath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Geometry/");
    mx::FilePathVec files;

    unsigned int loadFailed = 0;
    for (const std::string& extension : options.testExtensions)
    {
        if (options.skipExtensions.end() != std::find(options.skipExtensions.begin(), options.skipExtensions.end(), extension))
        {
            continue;
        }
        files = imagePath.getFilesInDirectory(extension);
        for (const mx::FilePath& file : files)
        {
            const mx::FilePath filePath = imagePath / file;
            mx::ImageDesc desc;
            bool loaded = options.geomHandler->loadGeometry(filePath);
            if (options.logFile)
            {
                *(options.logFile) << "Loaded image: " << filePath.asString() << ". Loaded: " << loaded << std::endl;
            }
            if (!loaded)
            {
                loadFailed++;
            }
        }
    }
    CHECK(loadFailed == 0);
}

TEST_CASE("Render: Geometry Handler Load", "[rendercore]")
{
    std::ofstream geomHandlerLog;
    geomHandlerLog.open("render_geom_handler_test.txt");
    bool geomLoaded = false;
    try
    {
        geomHandlerLog << "** Test TinyOBJ geom loader **" << std::endl;
        mx::TinyObjLoaderPtr loader = mx::TinyObjLoader::create();
        mx::GeometryHandlerPtr handler = mx::GeometryHandler::create();
        handler->addLoader(loader);

        GeomHandlerTestOptions options;
        options.logFile = &geomHandlerLog;
        options.geomHandler = handler;
        handler->supportedExtensions(options.testExtensions);
        testGeomHandler(options);

        geomLoaded = true;
    }
    catch (mx::ExceptionShaderValidationError& e)
    {
        for (auto error : e.errorLog())
        {
            geomHandlerLog << e.what() << " " << error << std::endl;
        }
    }
    catch (mx::Exception& e)
    {
        std::cout << e.what();
    }
    CHECK(geomLoaded);
    geomHandlerLog.close();
}


struct ImageHandlerTestOptions
{
    mx::ImageHandlerPtr imageHandler;
    std::ofstream* logFile;

    mx::StringSet testExtensions;
    mx::StringVec skipExtensions;
};

void testImageHandler(ImageHandlerTestOptions& options)
{
    mx::FilePath imagePath = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Images/");
    mx::FilePathVec files;

    unsigned int loadFailed = 0;
    for (const std::string& extension : options.testExtensions)
    {
        if (options.skipExtensions.end() != std::find(options.skipExtensions.begin(), options.skipExtensions.end(), extension))
        {
            continue;
        }
        files = imagePath.getFilesInDirectory(extension);
        for (const mx::FilePath& file : files)
        {
            const mx::FilePath filePath = imagePath / file;
            mx::ImageDesc desc;
            bool loaded = options.imageHandler->acquireImage(filePath, desc, false, nullptr);
            desc.freeResourceBuffer();
            CHECK(!desc.resourceBuffer);
            if (options.logFile)
            {
                *(options.logFile) << "Loaded image: " << filePath.asString() << ". Loaded: " << loaded << std::endl;
            }
            if (!loaded)
            {
                loadFailed++;
            }
        }
    }
    CHECK(loadFailed == 0);
}

TEST_CASE("Render: Image Handler Load", "[rendercore]")
{
    std::ofstream imageHandlerLog;
    imageHandlerLog.open("render_image_handler_test.txt");
    bool imagesLoaded = false;
    try
    {
        // Create a stock color image
        mx::ImageHandlerPtr imageHandler = mx::ImageHandler::create(nullptr);
        mx::Color4 color(1.0f, 0.0f, 0.0f, 1.0f);
        mx::ImageDesc desc;
        bool createdColorImage = imageHandler->createColorImage(color, desc);
        CHECK(!createdColorImage);
        desc.width = 1;
        desc.height = 1;
        desc.channelCount = 3;
        createdColorImage = imageHandler->createColorImage(color, desc);
        CHECK(createdColorImage);
        desc.freeResourceBuffer();
        CHECK(!desc.resourceBuffer);

        ImageHandlerTestOptions options;
        options.logFile = &imageHandlerLog;

        imageHandlerLog << "** Test STB image loader **" << std::endl;
        mx::StbImageLoaderPtr stbLoader = mx::StbImageLoader::create();
        imageHandler->addLoader(stbLoader);
        options.testExtensions = stbLoader->supportedExtensions();
        options.imageHandler = imageHandler;
        testImageHandler(options);

#if defined(MATERIALX_BUILD_OIIO) && defined(OPENIMAGEIO_ROOT_DIR)
        imageHandlerLog << "** Test OpenImageIO image loader **" << std::endl;
        mx::OiioImageLoaderPtr oiioLoader = mx::OiioImageLoader::create();
        mx::ImageHandlerPtr imageHandler3 = mx::ImageHandler::create(nullptr);
        imageHandler3->addLoader(oiioLoader);
        options.testExtensions = oiioLoader->supportedExtensions();
        options.imageHandler = imageHandler3;
        // Getting libpng warning: iCCP: known incorrect sRGB profile for some reason. TBD.
        options.skipExtensions.push_back("gif");
        testImageHandler(options);
#endif
        imagesLoaded = true;
    }
    catch (mx::ExceptionShaderValidationError& e)
    {
        for (auto error : e.errorLog())
        {
            imageHandlerLog << e.what() << " " << error << std::endl;
        }
    }
    catch (mx::Exception& e)
    {
        std::cout << e.what();
    }
    CHECK(imagesLoaded);
    imageHandlerLog.close();
}

TEST_CASE("Render: TestSuite", "[render]")
{
    bool skipTest = false;
#if !defined(MATERIALX_TEST_RENDER) 
    skipTest = true;
#endif
#if !defined(MATERIALX_BUILD_RENDERGLSL) && !defined(MATERIALX_BUILD_RENDEROSL)
    skipTest = true;
#endif

    if (skipTest)
    {
        return;
    }

    // Profiling times
    ShaderValidProfileTimes profileTimes;
    // Global setup timer
    AdditiveScopedTimer totalTime(profileTimes.totalTime, "Global total time");

#ifdef LOG_TO_FILE
#ifdef MATERIALX_BUILD_RENDERGLSL
    std::ofstream glslLogfile("genglsl_render_test.txt");
    std::ostream& glslLog(glslLogfile);
#endif
#ifdef MATERIALX_BUILD_RENDEROSL
    std::ofstream oslLogfile("genosl_vanilla_render_test.txt");
    std::ostream& oslLog(oslLogfile);
#endif
    std::string docValidLogFilename = "render_validate_doc_log.txt";
    std::ofstream docValidLogFile(docValidLogFilename);
    std::ostream& docValidLog(docValidLogFile);
    std::ofstream profilingLogfile("render_profiling_log.txt");
    std::ostream& profilingLog(profilingLogfile);
#else
#ifdef MATERIALX_BUILD_RENDERGLSL
    std::ostream& glslLog(std::cout);
#endif
#ifdef MATERIALX_BUILD_RENDEROSL
    std::ostream& oslLog(std::cout);
#endif
    std::string docValidLogFilename = "std::out";
    std::ostream& docValidLog(std::cout);
    std::ostream& profilingLog(std::cout);
#endif

    // For debugging, add files to this set to override
    // which files in the test suite are being tested.
    // Add only the test suite filename not the full path.
    mx::StringSet testfileOverride;

    AdditiveScopedTimer ioTimer(profileTimes.ioTime, "Global I/O time");
    mx::FilePath path = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite");
    mx::FilePathVec dirs;
    mx::FilePath baseDirectory = path;
    dirs = baseDirectory.getSubDirectories();

    // Check for an option file
    ShaderValidTestOptions options;
    const mx::FilePath optionsPath = path / mx::FilePath("_options.mtlx");
    // Append to file filter list
    if (getTestOptions(optionsPath, options))
    {
        for (auto filterFile : options.overrideFiles)
        {
            testfileOverride.insert(filterFile);
        }
    }
    ioTimer.endTimer();

    // All tests have been turned off so just stop the test.
    if (!options.runGLSLTests && !options.runOSLTests)
    {
        return;
    }

    // Library search path
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("libraries");

    // Load in the library dependencies once
    // This will be imported in each test document below
    ioTimer.startTimer();
    mx::DocumentPtr dependLib = mx::createDocument();
    mx::StringSet excludeFiles;
    if (!options.runGLSLTests)
    {
        excludeFiles.insert("stdlib_" + mx::GlslShaderGenerator::LANGUAGE + "_impl.mtlx");
    }
    if (!options.runOSLTests)
    {
        excludeFiles.insert("stdlib_osl_impl.mtlx");
        excludeFiles.insert("stdlib_" + mx::OslShaderGenerator::LANGUAGE + "_impl.mtlx");
    }

    const mx::StringVec libraries = { "stdlib", "pbrlib" };
    GenShaderUtil::loadLibraries(libraries, searchPath, dependLib, &excludeFiles);
    GenShaderUtil::loadLibrary(mx::FilePath::getCurrentPath() / mx::FilePath("libraries/bxdf/standard_surface.mtlx"), dependLib);

    mx::FilePath lightDir = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/Utilities/Lights");
    if (options.lightFiles.size() == 0)
    {
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath("lightcompoundtest.mtlx"), dependLib);
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath("lightcompoundtest_ng.mtlx"), dependLib);
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath("light_rig.mtlx"), dependLib);
    }
    else
    {
        for (auto lightFile : options.lightFiles)
        {
            GenShaderUtil::loadLibrary(lightDir / mx::FilePath(lightFile), dependLib);
        }
    }
    ioTimer.endTimer();

    // Create validators and generators
#if defined(MATERIALX_BUILD_RENDERGLSL)
    AdditiveScopedTimer glslSetupTime(profileTimes.glslTimes.setupTime, "GLSL setup time");

    mx::GlslValidatorPtr glslValidator = createGLSLValidator("sphere.obj", glslLog);
    mx::ShaderGeneratorPtr glslShaderGenerator = mx::GlslShaderGenerator::create();

    mx::ColorManagementSystemPtr glslColorManagementSystem = mx::DefaultColorManagementSystem::create(glslShaderGenerator->getLanguage());
    glslColorManagementSystem->loadLibrary(dependLib);
    glslShaderGenerator->setColorManagementSystem(glslColorManagementSystem);

    mx::GenContext glslContext(glslShaderGenerator);
    glslContext.registerSourceCodeSearchPath(searchPath);

    glslSetupTime.endTimer();
#endif // defined(MATERIALX_BUILD_RENDERGLSL)

#ifdef MATERIALX_BUILD_RENDEROSL
    AdditiveScopedTimer oslSetupTime(profileTimes.oslTimes.setupTime, "OSL setup time");

    mx::OslValidatorPtr oslValidator = createOSLValidator(oslLog);
    mx::ShaderGeneratorPtr oslShaderGenerator = mx::OslShaderGenerator::create();

    mx::GenContext oslContext(oslShaderGenerator);
    oslContext.registerSourceCodeSearchPath(searchPath);
    oslContext.registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));

    mx::ColorManagementSystemPtr oslColorManagementSystem = mx::DefaultColorManagementSystem::create(oslShaderGenerator->getLanguage());
    oslColorManagementSystem->loadLibrary(dependLib);
    oslShaderGenerator->setColorManagementSystem(oslColorManagementSystem);

    oslSetupTime.endTimer();
#endif

    mx::CopyOptions importOptions;
    importOptions.skipDuplicateElements = true;

#if defined(MATERIALX_BUILD_RENDERGLSL)
    mx::LightHandlerPtr glslLightHandler = nullptr;
    if (options.runGLSLTests)
    {
        AdditiveScopedTimer glslSetupLightingTimer(profileTimes.glslTimes.setupTime, "GLSL setup lighting time");

        if (glslShaderGenerator)
        {
            // Add lights as a dependency
            glslLightHandler = mx::LightHandler::create();
            createLightRig(dependLib, *glslLightHandler, glslContext,
                           options.radianceIBLPath, options.irradianceIBLPath);
        }
    }
#endif

    // Map to replace "/" in Element path names with "_".
    mx::StringMap pathMap;
    pathMap["/"] = "_";

    AdditiveScopedTimer validateTimer(profileTimes.validateTime, "Global validation time");
    AdditiveScopedTimer renderableSearchTimer(profileTimes.renderableSearchTime, "Global renderable search time");

    mx::StringSet usedImpls;

    const std::string MTLX_EXTENSION("mtlx");
    const std::string OPTIONS_FILENAME("_options.mtlx");
    for (auto dir : dirs)
    {
        ioTimer.startTimer();
        mx::FilePathVec files;
        files = dir.getFilesInDirectory(MTLX_EXTENSION);
        ioTimer.endTimer();

        for (const std::string& file : files)
        {

            if (file == OPTIONS_FILENAME)
            {
                continue;
            }

            ioTimer.startTimer();
            // Check if a file override set is used and ignore all files
            // not part of the override set
            if (testfileOverride.size() && testfileOverride.count(file) == 0)
            {
                ioTimer.endTimer();
                continue;
            }

            const mx::FilePath filePath = mx::FilePath(dir) / mx::FilePath(file);
            const std::string filename = filePath;

            mx::DocumentPtr doc = mx::createDocument();
            mx::readFromXmlFile(doc, filename, dir);

            doc->importLibrary(dependLib, &importOptions);
            ioTimer.endTimer();

            validateTimer.startTimer();
            std::cout << "Validating MTLX file: " << filename << std::endl;
#ifdef MATERIALX_BUILD_RENDERGLSL
            if (options.runGLSLTests)
                glslLog << "MTLX Filename: " << filename << std::endl;
#endif
#ifdef MATERIALX_BUILD_RENDEROSL
            if (options.runOSLTests)
                oslLog << "MTLX Filename: " << filename << std::endl;
#endif
            // Validate the test document
            std::string validationErrors;
            bool validDoc = doc->validate(&validationErrors);
            if (!validDoc)
            {
                docValidLog << filename << std::endl;
                docValidLog << validationErrors << std::endl;
            }
            validateTimer.endTimer();
            CHECK(validDoc);

            renderableSearchTimer.startTimer();
            std::vector<mx::TypedElementPtr> elements;
            try
            {
                mx::findRenderableElements(doc, elements);
            }
            catch (mx::Exception& e)
            {
                docValidLog << e.what() << std::endl;
                WARN("Find renderable elements failed, see: " + docValidLogFilename + " for details.");
            }
            renderableSearchTimer.endTimer();

            std::string outputPath = mx::FilePath(dir) / mx::FilePath(mx::removeExtension(file));
            mx::FileSearchPath imageSearchPath(dir);
            for (auto element : elements)
            {
                mx::OutputPtr output = element->asA<mx::Output>();
                mx::ShaderRefPtr shaderRef = element->asA<mx::ShaderRef>();
                mx::NodeDefPtr nodeDef = nullptr;
                if (output)
                {
                    nodeDef = output->getConnectedNode()->getNodeDef();
                }
                else if (shaderRef)
                {
                    nodeDef = shaderRef->getNodeDef();
                }
                if (nodeDef)
                {
                    mx::string elementName = mx::replaceSubstrings(element->getNamePath(), pathMap);
                    elementName = mx::createValidName(elementName);
#ifdef MATERIALX_BUILD_RENDERGLSL
                    if (options.runGLSLTests)
                    {
                        renderableSearchTimer.startTimer();
                        mx::InterfaceElementPtr impl = nodeDef->getImplementation(glslShaderGenerator->getTarget(), glslShaderGenerator->getLanguage());
                        renderableSearchTimer.endTimer();
                        if (impl)
                        {
                            if (options.checkImplCount)
                            {
                                mx::NodeGraphPtr nodeGraph = impl->asA<mx::NodeGraph>();
                                mx::InterfaceElementPtr nodeGraphImpl = nodeGraph ? nodeGraph->getImplementation() : nullptr;
                                usedImpls.insert(nodeGraphImpl ? nodeGraphImpl->getName() : impl->getName());
                            }
                            runGLSLValidation(elementName, element, *glslValidator, glslContext, glslLightHandler, doc, glslLog, options, profileTimes, imageSearchPath, outputPath);
                        }
                    }
#endif
#ifdef MATERIALX_BUILD_RENDEROSL
                    if (options.runOSLTests)
                    {
                        renderableSearchTimer.startTimer();
                        mx::InterfaceElementPtr impl2 = nodeDef->getImplementation(oslShaderGenerator->getTarget(), oslShaderGenerator->getLanguage());
                        renderableSearchTimer.endTimer();
                        if (impl2)
                        {
                            if (options.checkImplCount)
                            {
                                mx::NodeGraphPtr nodeGraph = impl2->asA<mx::NodeGraph>();
                                mx::InterfaceElementPtr nodeGraphImpl = nodeGraph ? nodeGraph->getImplementation() : nullptr;
                                usedImpls.insert(nodeGraphImpl ? nodeGraphImpl->getName() : impl2->getName());
                            }
                            runOSLValidation(elementName, element, *oslValidator, oslContext, doc, oslLog, options, profileTimes, imageSearchPath, outputPath);
                        }
                    }
#endif
                }
            }
        }
    }

    // Dump out profiling information
    totalTime.endTimer();
    printRunLog(profileTimes, options, usedImpls, profilingLog, dependLib
#ifdef MATERIALX_BUILD_RENDEROSL
        , oslContext
#endif
#ifdef MATERIALX_BUILD_RENDERGLSL
        , glslContext
#endif
    );
}

#endif
#endif
