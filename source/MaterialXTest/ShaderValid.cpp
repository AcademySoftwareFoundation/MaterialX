// Compile if module flags were set
#if defined(MATERIALX_TEST_RENDER) && defined(MATERIALX_BUILD_RENDER) && defined(MATERIALX_BUILD_GEN_GLSL)

// Run only on supported platforms
#include <MaterialXRender/Window/HardwarePlatform.h>
#if defined(OSWin_) || defined(OSLinux_) || defined(OSMac_)

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXRender/Handlers/HwLightHandler.h>

#ifdef MATERIALX_BUILD_GEN_GLSL
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXRender/ShaderValidators/Glsl/GlslValidator.h>
#include <MaterialXRender/OpenGL/GLTextureHandler.h>
#endif

#ifdef MATERIALX_BUILD_GEN_OGSFX
#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>
#endif

#ifdef MATERIALX_BUILD_GEN_OSL
#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXRender/ShaderValidators/Osl/OslValidator.h>
#endif

#ifdef MATERIALX_BUILD_CONTRIB
#include <MaterialXContrib/Handlers/TinyEXRImageLoader.h>
#endif
#include <MaterialXRender/Handlers/stbImageLoader.h>

#include <fstream>
#include <iostream>
#include <unordered_set>
#include <chrono>
#include <ctime>

namespace mx = MaterialX;

#define LOG_TO_FILE

extern void loadLibrary(const mx::FilePath& file, mx::DocumentPtr doc);
extern void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc,
                          const std::set<std::string>* excludeFiles = nullptr);

void createLightRig(mx::DocumentPtr doc, mx::HwLightHandler& lightHandler, mx::HwShaderGenerator& shadergen, const mx::GenOptions& options,
                    const mx::FilePath&  envIrradiancePath, const mx::FilePath& envRadiancePath)
{
    // Scan for lights
    const std::string LIGHT_SHADER_TYPE("lightshader");
    std::vector<mx::NodePtr> lights;
    for (mx::NodePtr node : doc->getNodes())
    {
        if (node->getType() == LIGHT_SHADER_TYPE)
        {
            lights.push_back(node);
        }
    }
    if (!lights.empty()) 
    {
        // Set the list of lights on the with the generator
        lightHandler.setLightSources(lights);

        // Find light types (node definitions) and generate ids.
        // Register types and ids with the generator
        std::unordered_map<std::string, unsigned int> identifiers;
        mx::mapNodeDefToIdentiers(lights, identifiers);
        for (auto id : identifiers)
        {
            mx::NodeDefPtr nodeDef = doc->getNodeDef(id.first);
            if (nodeDef)
            {
                shadergen.bindLightShader(*nodeDef, id.second, options);
            }
        }
    }

    // Clamp the number of light sources to the number found
    unsigned int lightSourceCount = static_cast<unsigned int>(lightHandler.getLightSources().size());
    shadergen.setMaxActiveLightSources(lightSourceCount);

    // Set up IBL inputs
    lightHandler.setLightEnvIrradiancePath(envIrradiancePath);
    lightHandler.setLightEnvRadiancePath(envRadiancePath);
}


#ifdef MATERIALX_BUILD_GEN_GLSL
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
    mx::stbImageLoaderPtr stbLoader = mx::stbImageLoader::create();
    mx::GLTextureHandlerPtr imageHandler = mx::GLTextureHandler::create(stbLoader);
#ifdef MATERIALX_BUILD_CONTRIB
    mx::TinyEXRImageLoaderPtr exrLoader = mx::TinyEXRImageLoader::create();
    imageHandler->addLoader(exrLoader);
#endif
    try
    {
        validator->initialize();
        validator->setImageHandler(imageHandler);
        validator->setLightHandler(nullptr);
        mx::GeometryHandler& geometryHandler = validator->getGeometryHandler();
        std::string geometryFile;
        if (fileName.length())
        {
            geometryFile =  mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite/Geometry/") / mx::FilePath(fileName);
            if (!geometryHandler.hasGeometry(geometryFile))
            {
                geometryHandler.clearGeometry();
                geometryHandler.loadGeometry(geometryFile);
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
    REQUIRE(initialized);

    return validator;
}
#endif

#ifdef MATERIALX_BUILD_GEN_OSL
static mx::OslValidatorPtr createOSLValidator(std::ostream& log)
{
    bool initialized = false;

    mx::OslValidatorPtr validator = mx::OslValidator::create();
    const std::string oslcExecutable(MATERIALX_OSLC_EXECUTABLE);
    validator->setOslCompilerExecutable(oslcExecutable);
    validator->setOslTestShadeExecutable(MATERIALX_TESTSHADE_EXECUTABLE);
    const std::string testRenderExecutable(MATERIALX_TESTRENDER_EXECUTABLE);
    validator->setOslTestRenderExecutable(testRenderExecutable);
    validator->setOslIncludePath(MATERIALX_OSL_INCLUDE_PATH);
    try
    {
        validator->initialize();
        validator->setImageHandler(nullptr);
        validator->setLightHandler(nullptr);
        initialized = true;

        // Pre-compile some required shaders for testrender
        if (!oslcExecutable.empty() && !testRenderExecutable.empty())
        {
            mx::FilePath shaderPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite/Utilities/");
            validator->setOslOutputFilePath(shaderPath);

            mx::StringVec files;
            const std::string OSL_EXTENSION("osl");
            mx::getFilesInDirectory(shaderPath.asString(), files, OSL_EXTENSION);
            for (std::string file : files)
            {
                mx::FilePath filePath = shaderPath / file;
                validator->compileOSL(filePath.asString());
            }

            // Set the search path for these compiled shaders.
            validator->setOslUtilityOSOPath(shaderPath);
        }
    }
    catch(mx::ExceptionShaderValidationError& e)
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
        output << "\tColor Management Files: ";
        output << std::endl;
        for (auto cmsFile : cmsFiles)
        {
            output << cmsFile << " ";
        }
        output << std::endl;
        output << "\tRun GLSL Tests: " << runGLSLTests << std::endl;
        output << "\tRun OGSFX Tests: " << runOGSFXTests << std::endl;
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
    std::vector<std::string> lightFiles;

    // List of comma separated file names which require color management.
    std::set<std::string> cmsFiles;

    // Set to true to always dump generated code to disk
    bool dumpGeneratedCode = false;

    // Execute GLSL tests
    bool runGLSLTests = true;

    // Execute OGSFX tests
    bool runOGSFXTests = false;

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
    MaterialX::FilePath glslNonShaderGeometry = "sphere.obj";

    // Shader GLSL geometry file
    MaterialX::FilePath glslShaderGeometry = "shaderball.obj";

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
        output << "\tOverhead time: " << (totalTime - glslTimes.totalTime - ogsfxTimes.totalTime- oslTimes.totalTime) << " seconds" << std::endl;
        output << "\tI/O time: " << ioTime << " seconds" << std::endl;
        output << "\tValidation time: " << validateTime << " seconds" << std::endl;
        output << "\tRenderable search time: " << renderableSearchTime << " seconds" << std::endl;

        glslTimes.print("GLSL Profile Times:", output);
        oslTimes.print("OSL Profile Times:", output);
        ogsfxTimes.print("OGSFX Profile Times:", output);

        output << "Elements tested: " << elementsTested << std::endl;
    }

    LanguageProfileTimes glslTimes;
    LanguageProfileTimes ogsfxTimes;
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
void getGenerationOptions(const ShaderValidTestOptions& testOptions, std::vector<mx::GenOptions>& optionsList)
{
    optionsList.clear();
    if (testOptions.shaderInterfaces & 1)
    {
        mx::GenOptions reducedOption;
        reducedOption.shaderInterfaceType = mx::SHADER_INTERFACE_REDUCED;
        optionsList.push_back(reducedOption);
    }
    // Alway fallback to complete if no options specified.
    if ((testOptions.shaderInterfaces & 2) || optionsList.empty())
    {
        mx::GenOptions completeOption;
        completeOption.shaderInterfaceType = mx::SHADER_INTERFACE_COMPLETE;
        optionsList.push_back(completeOption);
    }
}

#ifdef MATERIALX_BUILD_GEN_OGSFX
static void runOGSFXValidation(const std::string& shaderName, mx::TypedElementPtr element,
    mx::OgsFxShaderGenerator& shaderGenerator, const mx::HwLightHandlerPtr lightHandler, mx::DocumentPtr doc,
    std::ostream& log, const ShaderValidTestOptions& testOptions, ShaderValidProfileTimes& profileTimes, const std::string& outputPath = ".")
{
    AdditiveScopedTimer totalOgsFXTime(profileTimes.ogsfxTimes.totalTime, "OGSFX total time");

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
    getGenerationOptions(testOptions, optionsList);

    if (element && doc)
    {
        log << "------------ Run OGSFX validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (auto options : optionsList)
        {
            profileTimes.elementsTested++;

            std::string shaderPath;
            mx::FilePath outputFilePath = outputPath;
            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
            }

            // Note: mkdir will fail if the directory already exists which is ok.
            {
                AdditiveScopedTimer ioDir(profileTimes.ogsfxTimes.ioTime, "OGSFX dir time");
                mx::makeDirectory(outputFilePath);
            }

            shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);
            mx::ShaderPtr shader;
            try
            {
                AdditiveScopedTimer transpTimer(profileTimes.ogsfxTimes.transparencyTime, "OGSFX transparency time");
                options.hwTransparency = mx::isTransparentSurface(element, shaderGenerator);
                transpTimer.endTimer();
                AdditiveScopedTimer generationTimer(profileTimes.ogsfxTimes.generationTime, "OGSFX generation time");
                shader = shaderGenerator.generate(shaderName, element, options);
                generationTimer.endTimer();

                if (shader && testOptions.dumpGeneratedCode)
                {
                    AdditiveScopedTimer dumpTimer(profileTimes.ogsfxTimes.ioTime, "OGSFX io time");
                    std::ofstream file;
                    file.open(shaderPath + ".ogsfx");
                    file << shader->getSourceCode(MaterialX::OgsFxShader::FINAL_FX_STAGE);
                    file.close();
                }
            }
            catch (mx::ExceptionShaderGenError& e)
            {
                log << ">> " << e.what() << "\n";

                // Always dump shader on error
                if (shader)
                {
                    std::ofstream file;
                    file.open(shaderPath + ".ogsfx");
                    file << shader->getSourceCode(MaterialX::OgsFxShader::FINAL_FX_STAGE);
                    file.close();
                }

                shader = nullptr;
            }

            CHECK(shader != nullptr);
            if (shader == nullptr)
            {
                log << ">> Failed to generate shader\n";
                return;
            }

            const std::string& sourceCode = shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
            CHECK(sourceCode.length() > 0);
        }
    }
}
#endif

#ifdef MATERIALX_BUILD_GEN_GLSL
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
                              mx::GlslShaderGenerator& shaderGenerator, const mx::HwLightHandlerPtr lightHandler, mx::DocumentPtr doc,
                              std::ostream& log, const ShaderValidTestOptions& testOptions, ShaderValidProfileTimes& profileTimes, const std::string& outputPath=".")
{
    AdditiveScopedTimer totalGLSLTime(profileTimes.glslTimes.totalTime, "GLSL total time");

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
    getGenerationOptions(testOptions, optionsList);

    if(element && doc)
    {
        log << "------------ Run GLSL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (auto options : optionsList)
        {
            profileTimes.elementsTested++;

            std::string shaderPath;
            mx::FilePath outputFilePath = outputPath;
            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
            }

            // Note: mkdir will fail if the directory already exists which is ok.
            {
                AdditiveScopedTimer ioDir(profileTimes.glslTimes.ioTime, "GLSL dir time");
                mx::makeDirectory(outputFilePath);
            }

            shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);
            mx::ShaderPtr shader;
            try
            {
                AdditiveScopedTimer transpTimer(profileTimes.glslTimes.transparencyTime, "GLSL transparency time");
                options.hwTransparency = mx::isTransparentSurface(element, shaderGenerator);
                transpTimer.endTimer();
                AdditiveScopedTimer generationTimer(profileTimes.glslTimes.generationTime, "GLSL generation time");
                shader = shaderGenerator.generate(shaderName, element, options);
                generationTimer.endTimer();
            }
            catch (mx::ExceptionShaderGenError& e)
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
            const std::string& vertexSourceCode = shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
            const std::string& pixelSourceCode = shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
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
                mx::GeometryHandler& geomHandler = validator.getGeometryHandler();

                bool isShader = mx::elementRequiresShading(element);
                if (isShader)
                {
                    mx::FilePath geomPath;
                    if (!testOptions.glslShaderGeometry.isEmpty())
                    {
                        if (!testOptions.glslShaderGeometry.isAbsolute())
                        {
                            geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite/Geometry") / testOptions.glslShaderGeometry;
                        }
                        else
                        {
                            geomPath = testOptions.glslShaderGeometry;
                        }
                    }
                    else
                    {
                        geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite/Geometry/shaderball.obj");
                    }
                    if (!geomHandler.hasGeometry(geomPath))
                    {
                        geomHandler.clearGeometry();
                        geomHandler.loadGeometry(geomPath);
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
                            geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite/Geometry") / testOptions.glslNonShaderGeometry;
                        }
                        else
                        {
                            geomPath = testOptions.glslNonShaderGeometry;
                        }
                    }
                    else
                    {
                        geomPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite/Geometry/sphere.obj");
                    }
                    if (!geomHandler.hasGeometry(geomPath))
                    {
                        geomHandler.clearGeometry();
                        geomHandler.loadGeometry(geomPath);
                    }
                    validator.setLightHandler(nullptr);
                }

                {
                    AdditiveScopedTimer compileTimer(profileTimes.glslTimes.compileTime, "GLSL compile time");
                    validator.validateCreation(shader);
                    validator.validateInputs();
                }

                if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
                {
                    log << "-- SHADER_INTERFACE_REDUCED output:" << std::endl;
                }
                else
                {
                    log << "-- SHADER_INTERFACE_COMPLETE output:" << std::endl;
                }

                if (testOptions.dumpGlslUniformsAndAttributes)
                {
                    AdditiveScopedTimer printTimer(profileTimes.glslTimes.ioTime, "GLSL io time");
                    log << "* Uniform:" << std::endl;
                    program->printUniforms(log);
                    log << "* Attributes:" << std::endl;
                    program->printAttributes(log);

                    log << "* Uniform UI Properties:" << std::endl;
                    const std::string& target = shaderGenerator.getTarget();
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
                                for (size_t i=0; i<uiProperties.enumeration.size(); i++)
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
                file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
                file.close();
                file.open(shaderPath + "_ps.glsl");
                file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
                file.close();

                for (auto error : e.errorLog())
                {
                    log << e.what() << " " << error << std::endl;
                }
                log << ">> Refer to shader code in dump files: " << shaderPath << "_ps.glsl and _vs.glsl files" << std::endl;
            }
            CHECK(validated);
        }
    }
}
#endif

#ifdef MATERIALX_BUILD_GEN_OSL
static void runOSLValidation(const std::string& shaderName, mx::TypedElementPtr element, mx::OslValidator& validator,
                             mx::OslShaderGenerator& shaderGenerator, mx::DocumentPtr doc, std::ostream& log,
                             const ShaderValidTestOptions& testOptions, ShaderValidProfileTimes& profileTimes, const std::string& outputPath=".")
{
    AdditiveScopedTimer totalOSLTime(profileTimes.oslTimes.totalTime, "OSL total time");

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
    getGenerationOptions(testOptions, optionsList);

    if(element && doc)
    {
        log << "------------ Run OSL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (auto options : optionsList)
        {
            profileTimes.elementsTested++;

            mx::ShaderPtr shader;
            try
            {
                AdditiveScopedTimer genTimer(profileTimes.oslTimes.generationTime, "OSL generation time");
                shader = shaderGenerator.generate(shaderName, element, options);
            }
            catch (mx::ExceptionShaderGenError& e)
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
                mx::makeDirectory(outputFilePath);
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
                    if (shader->getOutputBlock().size() > 0)
                    {
                        const mx::Shader::Variable* output = shader->getOutputBlock()[0];
                        const mx::TypeSyntax& syntax = shaderGenerator.getSyntax()->getTypeSyntax(output->type);

                        const std::string& outputName = output->name;
                        const std::string& outputType = syntax.getTypeAlias().empty() ? syntax.getName() : syntax.getTypeAlias();

                        static const std::string SHADING_SCENE_FILE = "closure_color_scene.xml";
                        static const std::string NON_SHADING_SCENE_FILE = "constant_color_scene.xml";
                        const std::string& sceneTemplateFile = mx::elementRequiresShading(element) ? SHADING_SCENE_FILE : NON_SHADING_SCENE_FILE;

                        // Set shader output name and type to use
                        validator.setOslShaderOutput(outputName, outputType);

                        // Set scene template file. For now we only have the constant color scene file
                        mx::FilePath sceneTemplatePath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite/Utilities/");
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
    const std::string CMS_FILES_STRING("cmsFiles");
    const std::string SHADER_INTERFACES_STRING("shaderInterfaces");
    const std::string VALIDATE_ELEMENT_TO_RENDER_STRING("validateElementToRender");
    const std::string COMPILE_CODE_STRING("compileCode");
    const std::string RENDER_IMAGES_STRING("renderImages");
    const std::string SAVE_IMAGES_STRING("saveImages");
    const std::string DUMP_GLSL_UNIFORMS_AND_ATTRIBUTES_STRING("dumpGlslUniformsAndAttributes");
    const std::string RUN_OSL_TESTS_STRING("runOSLTests");
    const std::string RUN_GLSL_TESTS_STRING("runGLSLTests");
    const std::string RUN_OGSFX_TESTS_STRING("runOGSFXTests");
    const std::string CHECK_IMPL_COUNT_STRING("checkImplCount");
    const std::string DUMP_GENERATED_CODE_STRING("dumpGeneratedCode");
    const std::string GLSL_NONSHADER_GEOMETRY_STRING("glslNonShaderGeometry");
    const std::string GLSL_SHADER_GEOMETRY_STRING("glslShaderGeometry");
    const std::string RADIANCE_IBL_PATH_STRING("radianceIBLPath");
    const std::string IRRADIANCE_IBL_PATH_STRING("irradianceIBLPath");

    options.overrideFiles.clear();
    options.dumpGeneratedCode = false;
    options.cmsFiles.clear();

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
                        options.lightFiles  = MaterialX::splitString(p->getValueString(), ",");
                    }
                    if (name == CMS_FILES_STRING)
                    {
                        MaterialX::StringVec cmsStrings = MaterialX::splitString(p->getValueString(), ",");
                        for (auto cmsString : cmsStrings)
                        {
                            options.cmsFiles.insert(cmsString);
                        }
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
                    else if (name == RUN_OGSFX_TESTS_STRING)
                    {
                        options.runOGSFXTests = val->asA<bool>();
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
        // Disable saveing imsages, if no images are to be produced
        if (!options.renderImages)
        {
            options.saveImages = false;
        }

        // If implementation count check is required, then OSL and GLSL/OGSFX
        // code generation must be executed to be able to check implementation usage.
        if (options.checkImplCount)
        {
            options.runGLSLTests = true;
            options.runOSLTests = true;
            options.runOGSFXTests = true;
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
    std::set<std::string>& usedImpls, std::ostream& profilingLog, mx::DocumentPtr dependLib,
    mx::OslShaderGeneratorPtr oslShaderGenerator, mx::GlslShaderGeneratorPtr glslShaderGenerator,
    mx::OgsFxShaderGeneratorPtr ogsfxShaderGenerator)
{
    profileTimes.print(profilingLog);

    profilingLog << "---------------------------------------" << std::endl;
    options.print(profilingLog);

    if (options.checkImplCount)
    {
        profilingLog << "---------------------------------------" << std::endl;

        // Get implementation count from libraries
        std::set<mx::ImplementationPtr> libraryImpls;
        const std::vector<mx::ElementPtr>& children = dependLib->getChildren();
        for (auto child : children)
        {
            mx::ImplementationPtr impl = child->asA<mx::Implementation>();
            if (!impl)
            {
                continue;
            }
            libraryImpls.insert(impl);
        }

        mx::ColorManagementSystemPtr oslCms = nullptr;
#ifdef MATERIALX_BUILD_GEN_GLSL
        if (oslShaderGenerator)
            oslCms = oslShaderGenerator->getColorManagementSystem();
#endif
        mx::ColorManagementSystemPtr glslCms = nullptr;
#ifdef MATERIALX_BUILD_GEN_OSL
        if (glslShaderGenerator)
            glslCms = glslShaderGenerator->getColorManagementSystem();
#endif
        mx::ColorManagementSystemPtr ogsfxCms = nullptr;
#ifdef MATERIALX_BUILD_GEN_OGSFX
        if (ogsfxShaderGenerator)
            ogsfxCms = ogsfxShaderGenerator->getColorManagementSystem();
#endif
        size_t skipCount = 0;
        profilingLog << "-- Possibly missed implementations ----" << std::endl;
        std::vector<std::string> whiteList =
        {
            "arrayappend", "backfacing", "screen", "curveadjust", "dot_surfaceshader", "mix_surfaceshader"
            "displacementShader", "displacementshader", "volumeshader", "IM_dot_filename", "ambientocclusion", "dot_lightshader",
            "geomattrvalue_integer", "geomattrvalue_boolean", "geomattrvalue_string", "constant_matrix33", "add_matrix33FA",
            "add_matrix33", "subtract_matrix33FA", "subtract_matrix33", "multiply_matrix33", "divide_matrix33", "invert_matrix33",
            "transpose_matrix33", "transformvector_vector3M", "transformnormal_vector3M", "transformpoint_vector3M",
            "determinant_matrix33", "IM_dot_", "IM_constant_string_", "IM_constant_filename_"
        };
        const std::string OSL_STRING("osl");
        const std::string GEN_OSL_STRING(mx::OslShaderGenerator::LANGUAGE);
        unsigned int implementationUseCount = 0;
        for (auto libraryImpl : libraryImpls)
        {
            const std::string& implName = libraryImpl->getName();

            // Skip white-list items
            bool whileListFound = false;
            for (auto w : whiteList)
            {
                if (implName.find(w) != std::string::npos)
                {
                    skipCount++;
                    whileListFound = true;
                    break;
                }
            }
            if (whileListFound)
            {
                implementationUseCount++;
                continue;
            }

            if (usedImpls.count(implName))
            {
                implementationUseCount++;
                continue;
            }

            if (oslShaderGenerator && oslShaderGenerator->getCachedImplementation(implName))
            {
                implementationUseCount++;
                continue;
            }
            if (glslShaderGenerator && glslShaderGenerator->getCachedImplementation(implName))
            {
                implementationUseCount++;
                continue;
            }
            if (oslCms && oslCms->getCachedImplementation(implName))
            {
                implementationUseCount++;
                continue;
            }
            if (glslCms && glslCms->getCachedImplementation(implName) ||
                ogsfxCms && ogsfxCms->getCachedImplementation(implName))
            {
                implementationUseCount++;
                continue;
            }

            // See if we have a genosl implementation used
            // instead of the reference one
            if (libraryImpl->getLanguage() == OSL_STRING)
            {
                size_t endSize = implName.size() - 3;
                if (endSize > 0)
                {
                    std::string ending = implName.substr(endSize);
                    if (ending == OSL_STRING)
                    {
                        std::string sxImplName = implName.substr(0, endSize) + GEN_OSL_STRING;
                        if (oslShaderGenerator->getCachedImplementation(sxImplName))
                        {
                            implementationUseCount++;
                            continue;
                        }
                    }
                }
            }

            profilingLog << "\t" << implName << std::endl;
        }
        size_t libraryCount = libraryImpls.size();
        profilingLog << "Tested: " << implementationUseCount << " out of: " << libraryCount << " library implementations." << std::endl;
        CHECK(implementationUseCount == libraryCount);
    }
}

TEST_CASE("MaterialX documents", "[shadervalid]")
{
#if !defined(MATERIALX_BUILD_GEN_GLSL) && !defined(MATERIALX_BUILD_GEN_OSL) && defined(MATERIALX_BUILD_GEN_OGSFX)
    return;
#endif

    // Profiling times
    ShaderValidProfileTimes profileTimes;
    // Global setup timer
    AdditiveScopedTimer totalTime(profileTimes.totalTime, "Global total time");

#ifdef LOG_TO_FILE
#ifdef MATERIALX_BUILD_GEN_GLSL
    std::ofstream glslLogfile("shadervalid_GLSL_log.txt");
    std::ostream& glslLog(glslLogfile);
#endif
#ifdef MATERIALX_BUILD_GEN_OGSFX
    std::ofstream ogsfxLogfile("shadervalid_OGSFX_log.txt");
    std::ostream& ogsfxLog(ogsfxLogfile);
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    std::ofstream oslLogfile("shadervalid_OSL_log.txt");
    std::ostream& oslLog(oslLogfile);
#endif
    std::string docValidLogFilename = "shadervalid_validate_doc_log.txt";
    std::ofstream docValidLogFile(docValidLogFilename);
    std::ostream& docValidLog(docValidLogFile);
    std::ofstream profilingLogfile("shadervalid_profiling_log.txt");
    std::ostream& profilingLog(profilingLogfile);
#else
#ifdef MATERIALX_BUILD_GEN_GLSL
    std::ostream& glslLog(std::cout);
#endif
#ifdef MATERIALX_BUILD_GEN_OGSFX
    std::ostream& ogsfxLog(std::cout);
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    std::ostream& oslLog(std::cout);
#endif
    std::ostream& docValidLog(std::cout);
    std::ostream& profilingLog(std::cout);
#endif

    // For debugging, add files to this set to override
    // which files in the test suite are being tested.
    // Add only the test suite filename not the full path.
    std::set<std::string> testfileOverride;

    AdditiveScopedTimer ioTimer(profileTimes.ioTime, "Global I/O time");
    mx::FilePath path = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite");
    mx::StringVec dirs;
    std::string baseDirectory = path;
    mx::getSubDirectories(baseDirectory, dirs);

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
    if (!options.runGLSLTests && !options.runOSLTests && !options.runOGSFXTests)
    {
        return;
    }

    // Library search path
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");

    // Create validators and generators
#if defined(MATERIALX_BUILD_GEN_GLSL) || defined(MATERIALX_BUILD_GEN_OGSFX)
    mx::DefaultColorManagementSystemPtr glslColorManagementSystem = nullptr;
    mx::GlslValidatorPtr glslValidator = nullptr;
    mx::GlslShaderGeneratorPtr glslShaderGenerator = nullptr;
    mx::OgsFxShaderGeneratorPtr ogsfxShaderGenerator = nullptr;
#ifdef MATERIALX_BUILD_GEN_GLSL
    if (options.runGLSLTests)
    {
        AdditiveScopedTimer glslSetupTime(profileTimes.glslTimes.setupTime, "GLSL setup time");
        glslValidator = createGLSLValidator("sphere.obj", glslLog);
        glslShaderGenerator = std::static_pointer_cast<mx::GlslShaderGenerator>(mx::GlslShaderGenerator::create());
        glslShaderGenerator->registerSourceCodeSearchPath(searchPath);
        glslSetupTime.endTimer();
    }
#endif
#ifdef MATERIALX_BUILD_GEN_OGSFX
    if (options.runOGSFXTests)
    {
        AdditiveScopedTimer ogsfxSetupTime(profileTimes.ogsfxTimes.setupTime, "OGSFX setup time");
        ogsfxShaderGenerator = std::static_pointer_cast<mx::OgsFxShaderGenerator>(mx::OgsFxShaderGenerator::create());
        ogsfxShaderGenerator->registerSourceCodeSearchPath(searchPath);
        ogsfxSetupTime.endTimer();
    }
#endif
#endif // defined(MATERIALX_BUILD_GEN_GLSL) || defined(MATERIALX_BUILD_GEN_OGSFX)

#ifdef MATERIALX_BUILD_GEN_OSL
    mx::OslValidatorPtr oslValidator = nullptr;
    mx::OslShaderGeneratorPtr oslShaderGenerator = nullptr;
    mx::DefaultColorManagementSystemPtr oslColorManagementSystem = nullptr;
    if (options.runOSLTests)
    {
        AdditiveScopedTimer oslSetupTime(profileTimes.oslTimes.setupTime, "OSL setup time");
        oslValidator = createOSLValidator(oslLog);
        oslShaderGenerator = std::static_pointer_cast<mx::OslShaderGenerator>(mx::OslShaderGenerator::create());
        oslShaderGenerator->registerSourceCodeSearchPath(searchPath);
        oslShaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));
        oslSetupTime.endTimer();
    }
#endif

    // Load in the library dependencies once
    // This will be imported in each test document below
    ioTimer.startTimer();
    mx::DocumentPtr dependLib = mx::createDocument();
    std::set<std::string> excludeFiles;
    if (!options.runGLSLTests && !options.runOGSFXTests)
    {
        excludeFiles.insert("stdlib_" + mx::GlslShaderGenerator::LANGUAGE + "_impl.mtlx");
        excludeFiles.insert("stdlib_" + mx::GlslShaderGenerator::LANGUAGE + "_ogsfx_impl.mtlx");
    }
    if (!options.runOSLTests)
    {
        excludeFiles.insert("stdlib_osl_impl.mtlx");
        excludeFiles.insert("stdlib_" + mx::OslShaderGenerator::LANGUAGE + "_impl.mtlx");
    }
    if (options.cmsFiles.size() == 0)
    {
        excludeFiles.insert("cm_impl.mtlx");
    }

    const mx::StringVec libraries = { "stdlib", "pbrlib" };
    loadLibraries(libraries, searchPath, dependLib, &excludeFiles);
    mx::FilePath lightDir = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite/Utilities/Lights");
    if (options.lightFiles.size() == 0)
    {
        loadLibrary(lightDir / mx::FilePath("lightcompoundtest.mtlx"), dependLib);
        loadLibrary(lightDir / mx::FilePath("lightcompoundtest_ng.mtlx"), dependLib);
        loadLibrary(lightDir / mx::FilePath("light_rig.mtlx"), dependLib);
    }
    else
    {
        for (auto lightFile : options.lightFiles)
        {
            loadLibrary(lightDir / mx::FilePath(lightFile), dependLib);
        }
    }
    ioTimer.endTimer();

    mx::CopyOptions importOptions;
    importOptions.skipDuplicateElements = true;

#if defined(MATERIALX_BUILD_GEN_GLSL)
    mx::HwLightHandlerPtr glslLightHandler = nullptr;
    if (options.runGLSLTests)
    {
        AdditiveScopedTimer glslSetupLightingTimer(profileTimes.glslTimes.setupTime, "GLSL setup lighting time");

        if (glslShaderGenerator)
        {
            // Add lights as a dependency
            mx::GenOptions genOptions;
            glslLightHandler = mx::HwLightHandler::create();
            createLightRig(dependLib, *glslLightHandler, *glslShaderGenerator, genOptions, 
                           options.radianceIBLPath, options.irradianceIBLPath);
        }
    }
#endif

#if defined(MATERIALX_BUILD_GEN_OGSFX)
    mx::HwLightHandlerPtr ogsfxLightHandler = nullptr;
    if (options.runOGSFXTests)
    {
        AdditiveScopedTimer glslSetupLightingTimer(profileTimes.glslTimes.setupTime, "OGSFX setup lighting time");

        if (ogsfxShaderGenerator)
        {
            // Add lights as a dependency
            mx::GenOptions genOptions;
            ogsfxLightHandler = mx::HwLightHandler::create();
            createLightRig(dependLib, *ogsfxLightHandler, *ogsfxShaderGenerator, genOptions,
                           options.radianceIBLPath, options.irradianceIBLPath);
        }
    }
#endif

    // Map to replace "/" in Element path names with "_".
    mx::StringMap pathMap;
    pathMap["/"] = "_";

    AdditiveScopedTimer validateTimer(profileTimes.validateTime, "Global validation time");
    AdditiveScopedTimer renderableSearchTimer(profileTimes.renderableSearchTime, "Global renderable search time");

    std::set<std::string> usedImpls;

    const std::string MTLX_EXTENSION("mtlx");
    const std::string OPTIONS_FILENAME("_options.mtlx");
    for (auto dir : dirs)
    {
        ioTimer.startTimer();
        mx::StringVec files;
        mx::getFilesInDirectory(dir, files, MTLX_EXTENSION);
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
            readFromXmlFile(doc, filename);

            if (options.cmsFiles.size() && options.cmsFiles.count(file))
            {
#if defined(MATERIALX_BUILD_GEN_GLSL) || defined(MATERIALX_BUILD_GEN_OGSFX)
                // Load CMS system on demand if there is a file requiring color transforms
                if ((options.runGLSLTests || options.runOGSFXTests) && !glslColorManagementSystem)
                {
                    glslColorManagementSystem = mx::DefaultColorManagementSystem::create(glslShaderGenerator->getLanguage());
                    if (glslShaderGenerator)
                        glslShaderGenerator->setColorManagementSystem(glslColorManagementSystem);
                    if (ogsfxShaderGenerator)
                        ogsfxShaderGenerator->setColorManagementSystem(glslColorManagementSystem);
                    glslColorManagementSystem->loadLibrary(dependLib);
                }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
                if ((options.runOSLTests) && !oslColorManagementSystem)
                {
                    oslColorManagementSystem = mx::DefaultColorManagementSystem::create(oslShaderGenerator->getLanguage());
                    if (oslShaderGenerator)
                        oslShaderGenerator->setColorManagementSystem(oslColorManagementSystem);
                    oslColorManagementSystem->loadLibrary(dependLib);
                }
#endif
            }
            doc->importLibrary(dependLib, &importOptions);
            ioTimer.endTimer();

            validateTimer.startTimer();
            std::cout << "Validating MTLX file: " << filename << std::endl;
#ifdef MATERIALX_BUILD_GEN_GLSL
            if (options.runGLSLTests)
                glslLog << "MTLX Filename: " << filename << std::endl;
#endif
#ifdef MATERIALX_BUILD_GEN_OGSFX
            if (options.runOGSFXTests)
                ogsfxLog << "MTLX Filename: " << filename << std::endl;
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
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
            catch (mx::ExceptionShaderGenError& e)
            {
                docValidLog << e.what() << std::endl;
                WARN("Find renderable elements failed, see: " + docValidLogFilename + " for details.");
            }
            renderableSearchTimer.endTimer();

            std::string outputPath = mx::FilePath(dir) / mx::FilePath(mx::removeExtension(file));
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
#ifdef MATERIALX_BUILD_GEN_GLSL
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
                            runGLSLValidation(elementName, element, *glslValidator, *glslShaderGenerator, glslLightHandler, doc, glslLog, options, profileTimes, outputPath);
                        }
                    }
#endif
#ifdef MATERIALX_BUILD_GEN_OGSFX
                    if (options.runOGSFXTests)
                    {
                        renderableSearchTimer.startTimer();
                        mx::InterfaceElementPtr impl = nodeDef->getImplementation(ogsfxShaderGenerator->getTarget(), ogsfxShaderGenerator->getLanguage());
                        renderableSearchTimer.endTimer();
                        if (impl)
                        {
                            if (options.checkImplCount)
                            {
                                mx::NodeGraphPtr nodeGraph = impl->asA<mx::NodeGraph>();
                                mx::InterfaceElementPtr nodeGraphImpl = nodeGraph ? nodeGraph->getImplementation() : nullptr;
                                usedImpls.insert(nodeGraphImpl ? nodeGraphImpl->getName() : impl->getName());
                            }
                            runOGSFXValidation(elementName, element, *ogsfxShaderGenerator, ogsfxLightHandler, doc, ogsfxLog, options, profileTimes, outputPath);
                        }
                    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
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
                            runOSLValidation(elementName, element, *oslValidator, *oslShaderGenerator, doc, oslLog, options, profileTimes, outputPath);
                        }
                    }
#endif
                }
            }
        }
    }

    // Dump out profiling information
    totalTime.endTimer();
    printRunLog(profileTimes, options, usedImpls, profilingLog, dependLib, oslShaderGenerator, glslShaderGenerator, ogsfxShaderGenerator);
}



#endif
#endif
