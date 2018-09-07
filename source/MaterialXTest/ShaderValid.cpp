// Compile if module flags were set
#if defined(MATERIALX_TEST_VIEW) && defined(MATERIALX_BUILD_VIEW)

// Run only on supported platforms
#include <MaterialXView/Window/HardwarePlatform.h>
#if defined(OSWin_) || defined(OSLinux_) || defined(OSMac_)

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>
#include <MaterialXShaderGen/Util.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Swizzle.h>
#include <MaterialXShaderGen/HwShader.h>
#include <MaterialXShaderGen/HwLightHandler.h>

#include <fstream>
#include <iostream>
#include <unordered_set>

namespace mx = MaterialX;

#include <MaterialXView/ShaderValidators/Glsl/GlslValidator.h>
#include <MaterialXView/Handlers/TinyEXRImageHandler.h>

#define LOG_TO_FILE

extern void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc);
extern void createLightRig(mx::DocumentPtr doc, mx::HwLightHandler& lightHandler, mx::HwShaderGenerator& shadergen);
extern void createExampleMaterials(mx::DocumentPtr doc, std::vector<mx::MaterialPtr>& materials);

#if 0
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

    mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
    shaderGenerator->registerSourceCodeSearchPath(searchPath);

    mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
    createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

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
        const std::string geometryFile(mx::FilePath::getCurrentPath().asString() + "/documents/Geometry/sphere.obj");
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
#endif

//
// Create a validator with an image and geometry handler
// If a filename is supplied then a stock geometry of that name will be used if it can be loaded.
// By default if the file can be loaded it is assumed that rendering is done using a perspective
// view vs an orthographic view. This flag argument is updated and returned.
//
static mx::GlslValidatorPtr createValidator(bool& orthographicView, const std::string& fileName,
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
            geometryFile =  mx::FilePath::getCurrentPath().asString() + "/documents/Geometry/" + fileName;
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
static void runValidation(const std::string& outputPath, const std::string& shaderName, mx::ElementPtr element,
                          mx::GlslValidatorPtr validator, mx::ShaderGeneratorPtr shaderGenerator,
                          bool orthographicView, mx::DocumentPtr doc, std::ostream& log, bool outputMtlxDoc=false)
{
    mx::SgOptions options;

    if(element && doc)
    {
        log << "------------ Run validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        std::string shaderPath =  mx::FilePath(outputPath) / shaderName;
        mx::ShaderPtr shader = shaderGenerator->generate(shaderName, element, options);
        mx::HwShaderPtr hwShader = std::dynamic_pointer_cast<mx::HwShader>(shader);
        REQUIRE(hwShader != nullptr);
        REQUIRE(hwShader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(hwShader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        if (outputMtlxDoc)
        {
            mx::writeToXmlFile(doc, shaderPath + ".mtlx");
        }

        std::ofstream file;
        file.open(shaderPath + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(shaderPath + ".frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();

        // Validate
        MaterialX::GlslProgramPtr program = validator->program();
        bool validated = false;
        try
        {
            validator->validateCreation(hwShader);
            validator->validateInputs();

            program->printUniforms(log);
            program->printAttributes(log);

            validator->validateRender(orthographicView);
            std::string fileName = shaderPath + ".exr";
            validator->save(fileName);

            validated = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }
            log << ">> Refer to shader code in dump files: " << shaderPath << "(.vert, .frag) files" << std::endl;
        }
        CHECK(validated);
    }
}

TEST_CASE("GLSL MaterialX documents", "[shadervalid]")
{
#ifdef LOG_TO_FILE
    std::ofstream logfile("log_shadervalid_glsl_materialx_documents.txt");
    std::ostream& log(logfile);
#else
    std::ostream& log(std::cout);
#endif

    // Library search path
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");

    // Create a validator
    bool orthographicView = true;
    mx::GlslValidatorPtr validator = createValidator(orthographicView, "shaderball.obj", log);

    // Set up GLSL shader generator.
    // TODO : Need to add in other generators
    mx::ShaderGeneratorPtr glslShaderGenerator = mx::GlslShaderGenerator::create();
    glslShaderGenerator->registerSourceCodeSearchPath(searchPath);

    // Map to replace "/" in Element path names with "_".
    mx::StringMap pathMap;
    pathMap["/"] = "_";

    mx::FilePath path = mx::FilePath::getCurrentPath() / mx::FilePath("documents/TestSuite");
    mx::StringVec dirs;
    std::string baseDirectory = path;
    mx::getSubDirectories(baseDirectory, dirs);
    for (auto dir : dirs)
    {
        mx::StringVec files;
        mx::getDocumentsInDirectory(dir, files);
        for (std::string file : files)
        {
            mx::DocumentPtr doc = mx::createDocument();
            loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

            mx::FilePath filePath = mx::FilePath(dir) / mx::FilePath(file);
            std::string filename = filePath;
            readFromXmlFile(doc, filename);

            std::vector<mx::NodeGraphPtr> nodeGraphs = doc->getNodeGraphs();
            std::vector<mx::OutputPtr> outputList = doc->getOutputs();
            std::unordered_set<mx::OutputPtr> outputSet(outputList.begin(), outputList.end());
            std::vector<mx::MaterialPtr> materials = doc->getMaterials();

            if (!materials.empty() || !nodeGraphs.empty() || !outputList.empty())
            {
                log << "MTLX Filename: " << filename << std::endl;

                std::unordered_set<mx::OutputPtr> shaderrefOutputs;
                for (auto material : materials)
                {
                    for (auto shaderRef : material->getShaderRefs())
                    {
                        if (!shaderRef->hasSourceUri())
                        {
                            mx::HwShaderGenerator& hwGenerator = static_cast<mx::HwShaderGenerator&>(*glslShaderGenerator);

                            // Set up lighting
                            mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
                            createLightRig(doc, *lightHandler, hwGenerator);
                            // Pre-clamp the number of light sources to the number bound
                            size_t lightSourceCount = lightHandler->getLightSources().size();
                            hwGenerator.setMaxActiveLightSources(lightSourceCount);

                            mx::string elementName = mx::replaceSubstrings(shaderRef->getNamePath(), pathMap);
                            runValidation(dir, elementName, shaderRef, validator, glslShaderGenerator, orthographicView, doc, log);

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

                // Assume no lighting required here
                mx::HwShaderGenerator& hwGenerator = dynamic_cast<mx::HwShaderGenerator&>(*glslShaderGenerator);
                hwGenerator.setMaxActiveLightSources(0);

                // Validate node graph outputs
                // - Skip anything from an include file including libraries.
                // - Skip graphs connected to shaders.
                for (mx::NodeGraphPtr nodeGraph : nodeGraphs)
                {
                    if (!nodeGraph->hasSourceUri())
                    {
                        std::vector<mx::OutputPtr> nodeGraphOutputs = nodeGraph->getOutputs();
                        for (mx::OutputPtr output : nodeGraphOutputs)
                        {
                            outputSet.insert(output);
                        }
                    }
                }

                // For now we skip any outputs which are referenced elsewhere. This could be an option 
                // to also validate them.
                if (shaderrefOutputs.size())
                {
                    for (auto ref : shaderrefOutputs)
                    {
                        outputSet.erase(ref);
                    }
                }

                // Validate top level outputs
                // Skip anything from an include file including libraries
                // Skip outputs connected to shaders.
                for (mx::OutputPtr output : outputSet)
                {
                    // Skip surfaceshaders for now as they do no compile properly in GLSL
                    if (!output->hasSourceUri() && output->getType() != "surfaceshader")
                    {
                        mx::string elementName = mx::replaceSubstrings(output->getNamePath(), pathMap);
                        runValidation(dir, elementName, output, validator, glslShaderGenerator, orthographicView, doc, log);
                    }
                }
            }
        }
    }
}

#endif
#endif
