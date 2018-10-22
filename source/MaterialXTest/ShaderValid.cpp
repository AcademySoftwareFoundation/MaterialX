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

#ifdef MATERIALX_BUILD_GEN_GLSL
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXRender/ShaderValidators/Glsl/GlslValidator.h>
#endif

#ifdef MATERIALX_BUILD_GEN_OSL
#include <MaterialXGenOsl/ArnoldShaderGenerator.h>
#include <MaterialXRender/ShaderValidators/Osl/OslValidator.h>
#endif

#include <MaterialXRender/Handlers/TinyEXRImageHandler.h>

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
    mx::TinyEXRImageHandlerPtr handler = mx::TinyEXRImageHandler::create();
    bool initialized = false;
    bool orthographicsView = true;
    try
    {
        validator->initialize();
        validator->setImageHandler(handler);
        // Set geometry to draw with
        const std::string geometryFile(mx::FilePath::getCurrentPath().asString() + "/documents/TestSuite/Geometry/sphere.obj");
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
        std::string vertexShaderPath = shaderName + ".vert";
        std::string pixelShaderPath = shaderName + ".frag";

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
    mx::TinyEXRImageHandlerPtr imageHandler = mx::TinyEXRImageHandler::create();
    try
    {
        validator->initialize();
        validator->setImageHandler(imageHandler);
        validator->setLightHandler(nullptr);
        mx::GeometryHandlerPtr geometryHandler = validator->getGeometryHandler();
        std::string geometryFile;
        if (fileName.length())
        {
            geometryFile =  mx::FilePath::getCurrentPath().asString() + "/documents/TestSuite/Geometry/" + fileName;
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
    mx::TinyEXRImageHandlerPtr imageHandler = mx::TinyEXRImageHandler::create();
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
static void runGLSLValidation(const std::string& shaderName, mx::ElementPtr element, mx::GlslValidator& validator,
                              mx::GlslShaderGenerator& shaderGenerator, bool orthographicView, mx::DocumentPtr doc,
                              std::ostream& log, bool outputMtlxDoc=true, const std::string& outputPath=".")
{
    mx::GenOptions options;

    if(element && doc)
    {
        log << "------------ Run validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        std::string shaderPath;
        // Note: mkdir will fail if the directory already exists which is ok.
        mx::makeDirectory(outputPath);
        shaderPath = mx::FilePath(outputPath) / mx::FilePath(shaderName);

        mx::ShaderPtr shader;
        try
        {
            shader = shaderGenerator.generate(shaderName, element, options);
        }
        catch(mx::ExceptionShaderGenError e)
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

        if (outputMtlxDoc)
        {
            mx::writeToXmlFile(doc, shaderPath + ".mtlx");
        }

        // Validate
        MaterialX::GlslProgramPtr program = validator.program();
        bool validated = false;
        try
        {
            validator.validateCreation(shader);
            validator.validateInputs();

            program->printUniforms(log);
            program->printAttributes(log);

            validator.validateRender(orthographicView);
            std::string fileName = shaderPath + ".exr";
            validator.save(fileName);

            validated = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            // Dump shader stages on error
            std::ofstream file;
            file.open(shaderPath + ".vert");
            file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
            file.close();
            file.open(shaderPath + ".frag");
            file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
            file.close();

            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }
            log << ">> Refer to shader code in dump files: " << shaderPath << "(.vert, .frag) files" << std::endl;
        }
        CHECK(validated);
    }
}
#endif

#ifdef MATERIALX_BUILD_GEN_OSL
static void runOSLValidation(const std::string& shaderName, mx::TypedElementPtr element, mx::OslValidator& validator,
                             mx::ArnoldShaderGenerator& shaderGenerator, mx::DocumentPtr doc, std::ostream& log,
                             bool outputMtlxDoc=true, const std::string& outputPath=".")
{
    mx::GenOptions options;

    if(element && doc)
    {
        log << "------------ Run validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        mx::ShaderPtr shader;
        try
        {
            shader = shaderGenerator.generate(shaderName, element, options);
        }
        catch(mx::ExceptionShaderGenError e)
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
        // Note: mkdir will fail if the directory already exists which is ok.
        mx::makeDirectory(outputPath);
        shaderPath = mx::FilePath(outputPath) / mx::FilePath(shaderName);

        if (outputMtlxDoc)
        {
            mx::writeToXmlFile(doc, shaderPath + ".mtlx");
        }

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
            validator.setOslOutputFilePath(outputPath);
            validator.setOslShaderName(shaderName);

            // Validate compilation
            validator.validateCreation(shader);

            std::string elementType(element->getType());
            mx::string outputName = element->getName();

            std::set<std::string> colorClosures =
            {
                "surfaceshader", "volumeshader", "lightshader",
                "BSDF", "EDF", "VDF"
            };
            bool isShader = element->isA<mx::ShaderRef>() ||
                colorClosures.count(elementType) > 0;

            std::string sceneTemplateFile;

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
        catch(mx::ExceptionShaderValidationError e)
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
#endif

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
    for (auto dir : dirs)
    {
        mx::StringVec files;
        mx::getFilesInDirectory(dir, files, MTLX_EXTENSION);
        for (const std::string& file : files)
        {
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

            std::vector<mx::NodeGraphPtr> nodeGraphs = doc->getNodeGraphs();
            std::vector<mx::OutputPtr> outputList = doc->getOutputs();
            std::unordered_set<mx::OutputPtr> outputSet(outputList.begin(), outputList.end());
            std::vector<mx::MaterialPtr> materials = doc->getMaterials();

            if (!materials.empty() || !nodeGraphs.empty() || !outputList.empty())
            {
#ifdef MATERIALX_BUILD_GEN_GLSL
                glslLog << "MTLX Filename: " << filename << std::endl;
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
                oslLog << "MTLX Filename: " << filename << std::endl;
#endif

                doc->importLibrary(dependLib, &importOptions);

                // Validate the test document
                std::string validationErrors;
                bool validDoc = doc->validate(&validationErrors);
                if (!validDoc)
                {
                    docValidLog << validationErrors << std::endl;
                }
                CHECK(validDoc);

                std::unordered_set<mx::OutputPtr> shaderrefOutputs;
                for (auto material : materials)
                {
                    for (auto shaderRef : material->getShaderRefs())
                    {
                        if (!shaderRef->hasSourceUri())
                        {
                            std::string outputPath = mx::FilePath(dir) / mx::FilePath(mx::removeExtension(file));
                            mx::string elementName = mx::replaceSubstrings(shaderRef->getNamePath(), pathMap);
#ifdef MATERIALX_BUILD_GEN_GLSL
                            runGLSLValidation(elementName, shaderRef, *glslValidator, *glslShaderGenerator, orthographicView, doc, glslLog, false, outputPath);
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
                            runOSLValidation(elementName, shaderRef, *oslValidator, *oslShaderGenerator, doc, oslLog, false, outputPath);
#endif

                            // Find all bindinputs which reference outputs and outputgraphs
                            for (auto bindInput : shaderRef->getBindInputs())
                            {
                                mx::OutputPtr outputPtr = bindInput->getConnectedOutput();
                                if (outputPtr)
                                {
                                    shaderrefOutputs.insert(outputPtr);
                                }
                            }
                        }
                    }
                }

                // Find node graph outputs
                for (mx::NodeGraphPtr nodeGraph : nodeGraphs)
                {
                    // Skip anything from an include file including libraries.
                    if (!nodeGraph->hasSourceUri())
                    {
                        std::vector<mx::OutputPtr> nodeGraphOutputs = nodeGraph->getOutputs();
                        for (mx::OutputPtr output : nodeGraphOutputs)
                        {
                            // For now we skip any outputs which are referenced elsewhere.
                            // TODO: We could add an option to also validate them.
                            if (shaderrefOutputs.count(output) == 0)
                            {
                                outputSet.insert(output);
                            }
                        }
                    }
                }

                // Run validation on the outputs
                for (mx::OutputPtr output : outputSet)
                {
                    // Skip anything from include files
                    if (!output->hasSourceUri())
                    {
                        std::string outputPath = mx::FilePath(dir) / mx::FilePath(mx::removeExtension(file));
                        mx::string elementName = mx::replaceSubstrings(output->getNamePath(), pathMap);

#ifdef MATERIALX_BUILD_GEN_GLSL
                        runGLSLValidation(elementName, output, *glslValidator, *glslShaderGenerator, orthographicView, doc, glslLog, false, outputPath);
#endif
#ifdef MATERIALX_BUILD_GEN_OSL
                        runOSLValidation(elementName, output, *oslValidator, *oslShaderGenerator, doc, oslLog, false, outputPath);
#endif
                    }
                }
            }
        }
    }
}

#endif
#endif
