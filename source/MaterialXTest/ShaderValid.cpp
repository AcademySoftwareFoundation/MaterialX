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

#ifdef MATERIALX_BUILD_GEN_OSL
#include <MaterialXGenOsl/ArnoldShaderGenerator.h>
#include <MaterialXRender/ShaderValidators/Osl/OslValidator.h>
#endif

#include <MaterialXRender/Handlers/TinyEXRImageLoader.h>
#include <MaterialXRender/Handlers/stbImageLoader.h>

#include <fstream>
#include <iostream>
#include <unordered_set>

namespace mx = MaterialX;

#define LOG_TO_FILE

extern void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc);
extern void createLightRig(mx::DocumentPtr doc, mx::HwLightHandler& lightHandler, mx::HwShaderGenerator& shadergen);
extern void createExampleMaterials(mx::DocumentPtr doc, std::vector<mx::MaterialPtr>& materials);

TEST_CASE("GLSL Source", "[shadervalid]")
{
#ifdef LOG_TO_FILE
    std::ofstream logfile("log_shadervalid_glsl_source.txt");
    std::ostream& log(logfile);
#else
    std::ostream& log(std::cout);
#endif

    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    mx::GlslShaderGeneratorPtr glslShaderGenerator = std::static_pointer_cast<mx::GlslShaderGenerator>(mx::GlslShaderGenerator::create());
    glslShaderGenerator->registerSourceCodeSearchPath(searchPath);

    mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
    createLightRig(doc, *lightHandler, *glslShaderGenerator);

    // Initialize a GLSL validator and set image handler.
    // Validator initiazation will create a offscreen
    // window and offscreen OpenGL context for usage.
    mx::GlslValidatorPtr validator = mx::GlslValidator::create();
    mx::TinyEXRImageLoaderPtr exrLoader = mx::TinyEXRImageLoader::create();
    mx::GLTextureHandlerPtr handler = mx::GLTextureHandler::create(exrLoader);
    mx::stbImageLoaderPtr stbLoader = mx::stbImageLoader::create();
    handler->addLoader(stbLoader);
    bool initialized = false;
    bool orthographicsView = true;
    try
    {
        validator->initialize();
        validator->setImageHandler(handler);
        // Set geometry to draw with
        const std::string geometryFile(mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite/Geometry/sphere.obj"));
        mx::GeometryHandlerPtr geometryHandler = validator->getGeometryHandler();
        geometryHandler->setIdentifier(geometryFile);
        if (geometryHandler->getIdentifier() == geometryFile)
        {
            orthographicsView = false;
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

    // Test through set of fragment and vertex shader stage pairs
    // of files
    const std::vector<std::string> shaderNames =
    {
        "conditionals",
        "hello_world_graph",
        "hello_world_node",
        "hello_world_shaderref",
        "geometric_nodes",
        "subgraph_ex1",
        "subgraph_ex2",
        "test_noise2d",
        "test_noise3d",
        "test_cellnoise2d",
        "test_cellnoise3d",
        "test_fractal3d",
        "example1_surface",
        "example2_surface",
        "example3_surface",
        "example4_surface"
    };

    const std::set<std::string> shadersUseLighting =
    {
        "subgraph_ex2",
        "example1_surface",
        "example2_surface",
        "example3_surface",
        "example4_surface"
    };

    for (auto shaderName : shaderNames)
    {
        log << "------------ Validate shader from source: " << shaderName << std::endl;
        std::string vertexShaderPath = shaderName + "_vs.glsl";
        std::string pixelShaderPath = shaderName + "_ps.glsl";

        unsigned int stagesFound = 0;
        std::stringstream vertexShaderStream;
        std::stringstream pixelShaderStream;
        std::ifstream shaderFile;
        shaderFile.open(vertexShaderPath);
        if (shaderFile.is_open())
        {
            vertexShaderStream << shaderFile.rdbuf();
            shaderFile.close();
            stagesFound++;
        }
        shaderFile.open(pixelShaderPath);
        if (shaderFile.is_open())
        {
            pixelShaderStream << shaderFile.rdbuf();
            shaderFile.close();
            stagesFound++;
        }

        // To do: Make the dependence on ShaderGen test generated files more explicit
        // so as to avoid the possibility of failure here. For now skip tests if files not
        // found.
        //REQUIRE(stagesFound == 2);
        if (stagesFound != 2)
        {
            continue;
        }

        if (shadersUseLighting.count(shaderName))
        {
            validator->setLightHandler(lightHandler);
        }
        else
        {
            validator->setLightHandler(nullptr);
        }

        // Check program compilation
        bool programCompiled = false;
        mx::GlslProgramPtr program = validator->program();
        try {
            // Set stages and validate.
            // Note that pixel stage is first, then vertex stage
            std::vector<std::string> stages;
            stages.push_back(pixelShaderStream.str());
            stages.push_back(vertexShaderStream.str());

            validator->validateCreation(stages);
            validator->validateInputs();

            programCompiled = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }

            std::string stage = program->getStage(mx::HwShader::VERTEX_STAGE);
            log << ">> Failed vertex stage code:\n";
            log << stage;
            stage = program->getStage(mx::HwShader::PIXEL_STAGE);
            log << ">> Failed pixel stage code:\n";
            log << stage;
        }
        REQUIRE(programCompiled);

        // Check getting uniforms list
        bool uniformsParsed = false;
        try
        {
            program->printUniforms(log);
            uniformsParsed = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }
        }
        REQUIRE(uniformsParsed);

        // Check getting attributes list
        bool attributesParsed = false;
        try
        {
            program->printAttributes(log);
            attributesParsed = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }
        }
        REQUIRE(attributesParsed);

        // Check rendering which includes checking binding
        bool renderSucceeded = false;
        try
        {
            validator->validateRender(orthographicsView);
            renderSucceeded = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }
        }
        REQUIRE(renderSucceeded);

        try
        {
            std::string fileName = shaderName + ".exr";
            validator->save(fileName);
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }
        }
    }
}

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
    // Filter list of files to only run validation on.
    MaterialX::StringVec overrideFiles;

    // Set to true to always dump glsl generated files to disk
    bool dumpGlslFiles = false;

    // Execute GLSL tests
    bool runGLSLTests = true;

    // Execute OSL tests
    bool runOSLTests = true;

    // Run using a set of interfaces:
    // - 3 = run complete + reduced.
    // - 2 = run complete only (default)
    // - 1 = run reduced only.
    int shaderInterfaces = 2;

    // Non-shader GLSL geometry file
    MaterialX::FilePath glslNonShaderGeometry = "sphere.obj";

    // Shader GLSL geometry file
    MaterialX::FilePath glslShaderGeometry = "shaderball.obj";
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
                              std::ostream& log, const ShaderValidTestOptions& testOptions, const std::string& outputPath=".")
{
    std::vector<mx::GenOptions> optionsList;
    getGenerationOptions(testOptions, optionsList);

    if(element && doc)
    {
        log << "------------ Run validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (auto options : optionsList)
        {
            std::string shaderPath;
            mx::FilePath outputFilePath = outputPath;
            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
            }

            // Note: mkdir will fail if the directory already exists which is ok.
            mx::makeDirectory(outputFilePath);
            shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);

            mx::ShaderPtr shader;
            try
            {
                options.hwTransparency = mx::isTransparentSurface(element, shaderGenerator);
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
            CHECK(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
            CHECK(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

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

                validator.validateCreation(shader);
                validator.validateInputs();

                program->printUniforms(log);
                program->printAttributes(log);

                validator.validateRender(!isShader);
                std::string fileName = shaderPath + ".exr";
                validator.save(fileName);

                if (testOptions.dumpGlslFiles)
                {
                    std::ofstream file;
                    file.open(shaderPath + "_vs.glsl");
                    file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
                    file.close();
                    file.open(shaderPath + "_ps.glsl");
                    file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
                    file.close();
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
                log << ">> Refer to shader code in dump files: " << shaderPath << "(_vs.glsl, _ps.glsl) files" << std::endl;
            }
            CHECK(validated);
        }
    }
}
#endif

#ifdef MATERIALX_BUILD_GEN_OSL
static void runOSLValidation(const std::string& shaderName, mx::TypedElementPtr element, mx::OslValidator& validator,
                             mx::ArnoldShaderGenerator& shaderGenerator, mx::DocumentPtr doc, std::ostream& log,
                             const ShaderValidTestOptions& testOptions, const std::string& outputPath=".")
{
    std::vector<mx::GenOptions> optionsList;
    getGenerationOptions(testOptions, optionsList);

    if(element && doc)
    {
        log << "------------ Run validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (auto options : optionsList)
        {
            mx::ShaderPtr shader;
            try
            {
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
            mx::makeDirectory(outputFilePath);
            shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);

            // Write out osl file
            std::ofstream file;
            file.open(shaderPath + ".osl");
            file << shader->getSourceCode();
            file.close();

            // Validate
            validator.initialize();
            bool validated = false;
            try
            {
                // Set output path and shader name
                validator.setOslOutputFilePath(outputFilePath);
                validator.setOslShaderName(shaderName);

                // Validate compilation
                validator.validateCreation(shader);

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
                validator.validateRender();

                // TODO: Call additional validation routines here when they are available
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
    options.dumpGlslFiles = false;

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
                    else if (name == "shaderInterfaces")
                    {
                        options.shaderInterfaces = val->asA<int>();
                    }
                    else if (name == "runOSLTests")
                    {
                        options.runOSLTests = val->asA<bool>();
                    }
                    else if (name == "runGLSLTests")
                    {
                        options.runGLSLTests = val->asA<bool>();
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
        return true;
    }
    catch (mx::Exception e)
    {
    }
    return false;
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

#ifdef LOG_TO_FILE
    #ifdef MATERIALX_BUILD_GEN_GLSL
    std::ofstream glslLogfile("log_shadervalid_glsl_materialx_documents.txt");
    std::ostream& glslLog(glslLogfile);
    #endif
    #ifdef MATERIALX_BUILD_GEN_OSL
    std::ofstream oslLogfile("log_shadervalid_osl_materialx_documents.txt");
    std::ostream& oslLog(oslLogfile);
    #endif
    std::ofstream docValidLogfile("log_docvalid_materialx_documents.txt");
    std::ostream& docValidLog(docValidLogfile);
#else
    #ifdef MATERIALX_BUILD_GEN_GLSL
    std::ostream& glslLog(std::cout);
    #endif
    #ifdef MATERIALX_BUILD_GEN_OSL
    std::ostream& oslLog(std::cout);
    #endif
    std::ostream& docValidLog(std::cout);
#endif

    // For debugging, add files to this set to override
    // which files in the test suite are being tested.
    // Add only the test suite filename not the full path.
    std::set<std::string> testfileOverride;

    // Library search path
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");

    // Create validators and generators
    bool orthographicView = true;
#ifdef MATERIALX_BUILD_GEN_GLSL
    mx::GlslValidatorPtr glslValidator = createGLSLValidator(orthographicView, "sphere.obj", glslLog);
    mx::GlslShaderGeneratorPtr glslShaderGenerator = std::static_pointer_cast<mx::GlslShaderGenerator>(mx::GlslShaderGenerator::create());
    glslShaderGenerator->registerSourceCodeSearchPath(searchPath);
    mx::DefaultColorManagementSystemPtr glslColorManagementSystem = mx::DefaultColorManagementSystem::create(*glslShaderGenerator);
    glslShaderGenerator->setColorManagementSystem(glslColorManagementSystem);
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
    mx::OslValidatorPtr oslValidator = createOSLValidator(orthographicView, oslLog);
    mx::ArnoldShaderGeneratorPtr oslShaderGenerator = std::static_pointer_cast<mx::ArnoldShaderGenerator>(mx::ArnoldShaderGenerator::create());
    oslShaderGenerator->setRemappedShaderOutput(false);
    oslShaderGenerator->registerSourceCodeSearchPath(searchPath);
    oslShaderGenerator->registerSourceCodeSearchPath(searchPath / mx::FilePath("stdlib/osl"));
#endif

    // Load in the library dependencies once
    // This will be imported in each test document below
    mx::DocumentPtr dependLib = mx::createDocument();
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, dependLib);
#ifdef MATERIALX_BUILD_GEN_GLSL
    glslColorManagementSystem->loadLibrary(dependLib);
#endif

    mx::CopyOptions importOptions;
    importOptions.skipDuplicateElements = true;

    // Add lights as a dependency
    mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
    createLightRig(dependLib, *lightHandler, *glslShaderGenerator);

    // Clamp the number of light sources to the number bound
    size_t lightSourceCount = lightHandler->getLightSources().size();
    glslShaderGenerator->setMaxActiveLightSources(lightSourceCount);

    // Map to replace "/" in Element path names with "_".
    mx::StringMap pathMap;
    pathMap["/"] = "_";

    mx::FilePath path = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite");
    mx::StringVec dirs;
    std::string baseDirectory = path;
    mx::getSubDirectories(baseDirectory, dirs);

    const std::string MTLX_EXTENSION("mtlx");

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

    for (auto dir : dirs)
    {
        mx::StringVec files;
        mx::getFilesInDirectory(dir, files, MTLX_EXTENSION);
        for (const std::string& file : files)
        {
            if (file == "_options.mtlx")
            {
                continue;
            }

            // Check if a file override set is used and ignore all files
            // not part of the override set
            if (testfileOverride.size() && testfileOverride.count(file) == 0)
            {
                continue;
            }

            const mx::FilePath filePath = mx::FilePath(dir) / mx::FilePath(file);
            const std::string filename = filePath;

            mx::DocumentPtr doc = mx::createDocument();
            readFromXmlFile(doc, filename);
            doc->importLibrary(dependLib, &importOptions);

            std::cout << "Validating MTLX file: " << filename << std::endl;
#ifdef MATERIALX_BUILD_GEN_GLSL
            glslLog << "MTLX Filename: " << filename << std::endl;
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
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
            CHECK(validDoc);

            std::vector<mx::TypedElementPtr> elements;
            mx::findRenderableElements(doc, elements);

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
                    if (options.runGLSLTests && nodeDef->getImplementation(glslShaderGenerator->getTarget(), glslShaderGenerator->getLanguage()))
                    {
                        runGLSLValidation(elementName, element, *glslValidator, *glslShaderGenerator, lightHandler, doc, glslLog, options, outputPath);
                    }
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
                    if (file == "color_management.mtlx")
                    {
                        continue;
                    }
                    if (options.runOSLTests && nodeDef->getImplementation(oslShaderGenerator->getTarget(), oslShaderGenerator->getLanguage()))
                    {
                        runOSLValidation(elementName, element, *oslValidator, *oslShaderGenerator, doc, oslLog, options, outputPath);
                    }
#endif
                }
            }
        }
    }
}

#endif
#endif
