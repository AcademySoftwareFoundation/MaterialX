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
#include <MaterialXGenShader/HwLightHandler.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>

#ifdef MATERIALX_BUILD_GEN_GLSL
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXRender/ShaderValidators/Glsl/GlslValidator.h>
#include <MaterialXRender/OpenGL/GLTextureHandler.h>
#endif

#ifdef MATERIALX_BUILD_GEN_OGSFX
#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>
#endif

#ifdef MATERIALX_BUILD_GEN_OSL
#include <MaterialXGenOsl/ArnoldShaderGenerator.h>
#include <MaterialXRender/ShaderValidators/Osl/OslValidator.h>
#endif

#include <MaterialXRender/Handlers/TinyEXRImageLoader.h>
#include <MaterialXRender/Handlers/stbImageLoader.h>

#include <fstream>
#include <iostream>
#include <unordered_set>
#include <chrono>
#include <ctime>

namespace mx = MaterialX;

#define LOG_TO_FILE

extern void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc,
                          const std::set<std::string>* excludeFiles = nullptr);
extern void createLightRig(mx::DocumentPtr doc, mx::HwLightHandler& lightHandler, mx::HwShaderGenerator& shadergen, const mx::GenOptions& options);

#ifdef MATERIALX_BUILD_GEN_GLSL
//
// Create a validator with an image and geometry handler
// If a filename is supplied then a stock geometry of that name will be used if it can be loaded.
// By default if the file can be loaded it is assumed that rendering is done using a perspective
// view vs an orthographic view. This flag argument is updated and returned.
//
static mx::GlslValidatorPtr createGLSLValidator(bool& orthographicView, const std::string& fileName,
                                                std::ostream& log)
{
    bool initialized = false;
    orthographicView = true;
    mx::GlslValidatorPtr validator = mx::GlslValidator::create();
    mx::TinyEXRImageLoaderPtr imageLoader = mx::TinyEXRImageLoader::create();
    mx::GLTextureHandlerPtr imageHandler = mx::GLTextureHandler::create(imageLoader);
    mx::stbImageLoaderPtr stbLoader = mx::stbImageLoader::create();
    imageHandler->addLoader(stbLoader);
    try
    {
        validator->initialize();
        validator->setImageHandler(imageHandler);
        validator->setLightHandler(nullptr);
        mx::GeometryHandlerPtr geometryHandler = validator->getGeometryHandler();
        std::string geometryFile;
        if (fileName.length())
        {
            geometryFile =  mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite/Geometry/") / mx::FilePath(fileName);
            geometryHandler->setIdentifier(geometryFile);
        }
        if (geometryHandler->getIdentifier() == geometryFile)
        {
            orthographicView = false;
        }
        initialized = true;
    }
    catch (mx::ExceptionShaderValidationError e)
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
static mx::OslValidatorPtr createOSLValidator(bool& orthographicView, std::ostream& log)
{
    bool initialized = false;
    orthographicView = true;
    bool initializeTestRender = false;

    mx::OslValidatorPtr validator = mx::OslValidator::create();
#ifdef MATERIALX_OSLC_EXECUTABLE
    validator->setOslCompilerExecutable(MATERIALX_OSLC_EXECUTABLE);
#endif
#ifdef MATERIALX_TESTSHADE_EXECUTABLE
    validator->setOslTestShadeExecutable(MATERIALX_TESTSHADE_EXECUTABLE);
#endif
#ifdef MATERIALX_TESTRENDER_EXECUTABLE
    validator->setOslTestRenderExecutable(MATERIALX_TESTRENDER_EXECUTABLE);
    initializeTestRender = true;
#endif
#ifdef MATERIALX_OSL_INCLUDE_PATH
    validator->setOslIncludePath(MATERIALX_OSL_INCLUDE_PATH);
#endif
    try
    {
        validator->initialize();
        validator->setImageHandler(nullptr);
        validator->setLightHandler(nullptr);
        initialized = true;

        // Pre-compile some required shaders for testrender
        if (initializeTestRender)
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
    catch(mx::ExceptionShaderValidationError e)
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
        output << "\tColor Management Files: ";
        for (auto cmsFile : cmsFiles)
        {
            output << cmsFile << " ";
        }
        output << std::endl;
        output << "\tRun GLSL Tests: " << runGLSLTests << std::endl;
        output << "\tRun OGSFX Tests: " << runOGSFXTests << std::endl;
        output << "\tRun OSL Tests: " << runOSLTests << std::endl;
        output << "\tCheck Implementation Usage Count: " << checkImplCount << std::endl;
        output << "\tDump GLSL Files: " << dumpGlslFiles << std::endl;
        output << "\tShader Interfaces: " << shaderInterfaces << std::endl;
        output << "\tCompile code: " << compileCode << std::endl;
        output << "\tRender Images: " << renderImages << std::endl;
        output << "\tSave Images: " << saveImages << std::endl;
        output << "\tDump GLSL Uniforms and Attributes  " << dumpGlslUniformsAndAttributes << std::endl;
        output << "\tGLSL Non-Shader Geometry: " << glslNonShaderGeometry.asString() << std::endl;
        output << "\tGLSL Shader Geometry: " << glslShaderGeometry.asString() << std::endl;
    }

    // Filter list of files to only run validation on.
    MaterialX::StringVec overrideFiles;

    // List of comma separated file names which require color management.
    std::set<std::string> cmsFiles;

    // Set to true to always dump glsl generated files to disk
    bool dumpGlslFiles = false;

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
            bool validated = false;
            try
            {
                AdditiveScopedTimer transpTimer(profileTimes.ogsfxTimes.transparencyTime, "OGSFX transparency time");
                options.hwTransparency = mx::isTransparentSurface(element, shaderGenerator);
                transpTimer.endTimer();
                AdditiveScopedTimer generationTimer(profileTimes.ogsfxTimes.generationTime, "OGSFX generation time");
                shader = shaderGenerator.generate(shaderName, element, options);
                generationTimer.endTimer();
            }
            catch (mx::ExceptionShaderGenError e)
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

            const std::string& sourceCode = shader->getSourceCode(mx::OgsFxShader::FINAL_FX_STAGE);
            validated = sourceCode.length() > 0;
            if (validated && testOptions.dumpGlslFiles)
            {
                AdditiveScopedTimer dumpTimer(profileTimes.ogsfxTimes.ioTime, "OGSFX io time");
                std::ofstream file;
                file.open(shaderPath + ".ogsfx");
                file << sourceCode;
                file.close();
            }
            CHECK(validated);          
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
            catch (mx::ExceptionShaderGenError e)
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

            if (testOptions.dumpGlslFiles)
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
                    geomHandler->setIdentifier(geomPath);
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
                    geomHandler->setIdentifier(geomPath);
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
                    program->printUniforms(log);
                    program->printAttributes(log);
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
            catch (mx::ExceptionShaderValidationError e)
            {
                if (!testOptions.dumpGlslFiles)
                {
                    // Dump shader stages on error
                    std::ofstream file;
                    file.open(shaderPath + "_vs.glsl");
                    file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
                    file.close();
                    file.open(shaderPath + "_ps.glsl");
                    file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
                    file.close();
                }

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
                             mx::ArnoldShaderGenerator& shaderGenerator, mx::DocumentPtr doc, std::ostream& log,
                             const ShaderValidTestOptions& testOptions, ShaderValidProfileTimes& profileTimes, const std::string& outputPath=".")
{
    AdditiveScopedTimer totalOSLTime(profileTimes.oslTimes.totalTime, "OSL total time");

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
            catch (mx::ExceptionShaderGenError e)
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
                    std::string elementType;
                    std::string sceneTemplateFile;
                    mx::string outputName = element->getName();

                    bool isShader = mx::elementRequiresShading(element);
                    if (isShader)
                    {
                        // TODO: Assume name is "out". This is the default value.
                        // We require shader generation to provide us an output name
                        // to the actual name.
                        outputName = "out";

                        // TODO: Asume type is closure color until we can
                        // get the actual output type from code generation
                        elementType = mx::OslValidator::OSL_CLOSURE_COLOR_STRING;

                        sceneTemplateFile.assign("closure_color_scene.xml");
                    }
                    else
                    {
                        elementType.assign(element->getType());
                        sceneTemplateFile.assign("constant_color_scene.xml");
                    }

                    // Set shader output name and type to use
                    //
                    // If the generator has already remapped the output type then indicate to
                    // not do so again during validation.
                    validator.setOslShaderOutputNameAndType(outputName, elementType, shaderGenerator.remappedShaderOutput());

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

                validated = true;
            }
            catch (mx::ExceptionShaderValidationError e)
            {
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
    options.overrideFiles.clear();
    options.cmsFiles.clear();

    MaterialX::DocumentPtr doc = MaterialX::createDocument();
    try {
        MaterialX::readFromXmlFile(doc, optionFile);

        MaterialX::NodeDefPtr optionDefs = doc->getNodeDef("ShaderValidTestOptions");
        if (optionDefs)
        {
            for (MaterialX::ParameterPtr p : optionDefs->getParameters())
            {
                const std::string& name = p->getName();
                MaterialX::ValuePtr val = p->getValue();
                if (val)
                {
                    if (name == "overrideFiles")
                    {
                        options.overrideFiles = MaterialX::splitString(p->getValueString(), ",");
                    }
                    if (name == "cmsFiles")
                    {
                        MaterialX::StringVec cmsStrings = MaterialX::splitString(p->getValueString(), ",");
                        for (auto cmsString : cmsStrings)
                        {
                            options.cmsFiles.insert(cmsString);
                        }
                    }
                    else if (name == "shaderInterfaces")
                    {
                        options.shaderInterfaces = val->asA<int>();
                    }
                    else if (name == "compileCode")
                    {
                        options.compileCode = val->asA<bool>();
                    }
                    else if (name == "renderImages")
                    {
                        options.renderImages = val->asA<bool>();
                    }
                    else if (name == "saveImages")
                    {
                        options.saveImages = val->asA<bool>();
                    }
                    else if (name == "dumpGlslUniformsAndAttributes")
                    {
                        options.dumpGlslUniformsAndAttributes = val->asA<bool>();
                    }
                    else if (name == "runOSLTests")
                    {
                        options.runOSLTests = val->asA<bool>();
                    }
                    else if (name == "runGLSLTests")
                    {
                        options.runGLSLTests = val->asA<bool>();
                    }
                    else if (name == "runOGSFXTests")
                    {
                        options.runOGSFXTests = val->asA<bool>();
                    }
                    else if (name == "checkImplCount")
                    {
                        options.checkImplCount = val->asA<bool>();
                    }
                    else if (name == "dumpGlslFiles")
                    {
                        options.dumpGlslFiles = val->asA<bool>();
                    }
                    else if (name == "glslNonShaderGeometry")
                    {
                        options.glslNonShaderGeometry = p->getValueString();
                    }
                    else if (name == "glslShaderGeometry")
                    {
                        options.glslShaderGeometry = p->getValueString();
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

        // If implementation count check is required, then at a minimum OSL and GLSL
        // code generation must execute to be able to check implementation usage.
        if (options.checkImplCount)
        {
            options.runGLSLTests = true;
            options.runOSLTests = true;
        }
        return true;
    }
    catch (mx::Exception e)
    {
    }
    return false;
}

void printRunLog(const ShaderValidProfileTimes &profileTimes, const ShaderValidTestOptions& options,
    std::set<std::string>& usedImpls, std::ostream& profilingLog, mx::DocumentPtr dependLib,
    mx::ArnoldShaderGeneratorPtr oslShaderGenerator, mx::GlslShaderGeneratorPtr glslShaderGenerator,
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
            "geomattrvalue_integer", "geomattrvalue_boolean", "geomattrvalue_string"
        };
        const std::string OSL_STRING("osl");
        const std::string SX_OSL_STRING("sx_osl");
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

            // See if we have a sx-osl implementation used
            // instead of the reference one
            if (libraryImpl->getLanguage() == OSL_STRING)
            {
                size_t endSize = implName.size() - 3;
                if (endSize > 0)
                {
                    std::string ending = implName.substr(endSize);
                    if (ending == OSL_STRING)
                    {
                        std::string sxImplName = implName.substr(0, endSize) + SX_OSL_STRING;
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
        // TODO: Add a CHECK when all implementations have been tested using unit tests.
        // CHECK(implementationUseCount == libraryCount);
    }
}

TEST_CASE("MaterialX documents", "[shadervalid]")
{
    bool runValidation = false;
#ifdef MATERIALX_BUILD_GEN_GLSL
    runValidation = true;
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    runValidation = true;
#endif
    if (!runValidation)
    {
        // No generators exist so there is nothing to test. Just return
        return;
    }

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
    std::ofstream docValidLogfile("shadervalid_validate_doc_log.txt");
    std::ostream& docValidLog(docValidLogfile);
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
    bool orthographicView = true;
#if defined(MATERIALX_BUILD_GEN_GLSL) || defined(MATERIALX_BUILD_GEN_OGSFX)
    mx::DefaultColorManagementSystemPtr glslColorManagementSystem = nullptr;
    mx::GlslValidatorPtr glslValidator = nullptr;
    mx::GlslShaderGeneratorPtr glslShaderGenerator = nullptr;
    mx::OgsFxShaderGeneratorPtr ogsfxShaderGenerator = nullptr;
#ifdef MATERIALX_BUILD_GEN_GLSL
    if (options.runGLSLTests)
    {
        AdditiveScopedTimer glslSetupTime(profileTimes.glslTimes.setupTime, "GLSL setup time");
        glslValidator = createGLSLValidator(orthographicView, "sphere.obj", glslLog);
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
    mx::ArnoldShaderGeneratorPtr oslShaderGenerator = nullptr;
    if (options.runOSLTests)
    {
        AdditiveScopedTimer oslSetupTime(profileTimes.oslTimes.setupTime, "OSL setup time");
        oslValidator = createOSLValidator(orthographicView, oslLog);
        oslShaderGenerator = std::static_pointer_cast<mx::ArnoldShaderGenerator>(mx::ArnoldShaderGenerator::create());
        oslShaderGenerator->setRemappedShaderOutput(false);
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
        excludeFiles.insert("stdlib_sx-glsl_impl.mtlx");
    }
    if (!options.runOSLTests)
    {
        excludeFiles.insert("stdlib_osl_impl.mtlx");
        excludeFiles.insert("stdlib_sx-osl_impl.mtlx");
    }
    if (options.cmsFiles.size() == 0)
    {
        excludeFiles.insert("cm_impl.mtlx");
    }
    const mx::StringVec libraries = { "stdlib", "sxpbrlib" };
    loadLibraries(libraries, searchPath, dependLib, &excludeFiles);
    ioTimer.endTimer();

    mx::CopyOptions importOptions;
    importOptions.skipDuplicateElements = true;

#if defined(MATERIALX_BUILD_GEN_GLSL) || defined(MATERIALX_BUILD_GEN_OGSFX)
    mx::HwLightHandlerPtr lightHandler = nullptr;
    if (options.runGLSLTests || options.runOGSFXTests)
    {
        AdditiveScopedTimer glslSetupLightingTimer(profileTimes.glslTimes.setupTime, "GLSL setup lighting time");

        // Add lights as a dependency
        mx::GenOptions genOptions;
        lightHandler = mx::HwLightHandler::create();
        createLightRig(dependLib, *lightHandler, *glslShaderGenerator, genOptions);

        // Clamp the number of light sources to the number bound
        size_t lightSourceCount = lightHandler->getLightSources().size();
        if (glslShaderGenerator)
        {
            glslShaderGenerator->setMaxActiveLightSources(lightSourceCount);
        }
        if (ogsfxShaderGenerator)
        {
            ogsfxShaderGenerator->setMaxActiveLightSources(lightSourceCount);
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
    for (auto dir : dirs)
    {
        ioTimer.startTimer();
        mx::StringVec files;
        mx::getFilesInDirectory(dir, files, MTLX_EXTENSION);
        ioTimer.endTimer();

        for (const std::string& file : files)
        {
            if (file == "_options.mtlx")
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
            mx::findRenderableElements(doc, elements);
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
                            runGLSLValidation(elementName, element, *glslValidator, *glslShaderGenerator, lightHandler, doc, glslLog, options, profileTimes, outputPath);
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
                            runOGSFXValidation(elementName, element, *ogsfxShaderGenerator, lightHandler, doc, ogsfxLog, options, profileTimes, outputPath);
                        }
                    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
                    if (options.runOSLTests)
                    {
                        // Skip files using CMS for OSL assuming there is no support available
                        if (options.cmsFiles.size() && options.cmsFiles.count(file))
                        {
                            continue;
                        }
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
