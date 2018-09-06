// Compile if module flags were set
#if defined(MATERIALX_TEST_VIEW) && defined(MATERIALX_BUILD_VIEW) && defined(MATERIALX_BUILD_GLSL)

// Run only on supported platforms
#include <MaterialXView/Window/HardwarePlatform.h>
#if defined(OSWin_) || defined(OSLinux_) || defined(OSMac_)

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXShaderGen/Util.h>
#include <MaterialXShaderGen/Nodes/Swizzle.h>
#include <MaterialXShaderGen/HwShader.h>
#include <MaterialXShaderGen/HwLightHandler.h>

#include <MaterialXGlsl/GlslShaderGenerator.h>

#include <MaterialXView/ShaderValidators/Glsl/GlslValidator.h>
#include <MaterialXView/Handlers/TinyEXRImageHandler.h>

#include <fstream>
#include <iostream>

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
        log << "------------ Run validation with element: " << element->getName() << std::endl;

        std::string shaderPath =  outputPath + "/" + shaderName;
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

            std::string stage = program->getStage(mx::HwShader::VERTEX_STAGE);
            log << ">> Failed vertex stage code:\n";
            log << stage;
            stage = program->getStage(mx::HwShader::PIXEL_STAGE);
            log << ">> Failed pixel stage code:\n";
            log << stage;
        }
        REQUIRE(validated);
    }
}

TEST_CASE("GLSL geometry", "[shadervalid]")
{
#ifdef LOG_TO_FILE
    std::ofstream logfile("log_shadervalid_glsl_geometry.txt");
    std::ostream& log(logfile);
#else
    std::ostream& log(std::cout);
#endif

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");

    // Stores our document
    mx::DocumentPtr doc = mx::createDocument();
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    // Stores our nodegraph
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("nodegraph");

    // Stores our output
    mx::OutputPtr output = nodeGraph->addOutput("output");

    // Create a validator
    bool orthographicView = true;
    mx::GlslValidatorPtr validator = createValidator(orthographicView, "shaderball.obj", log);

    // Set up shader generator
    mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
    shaderGenerator->registerSourceCodeSearchPath(searchPath);

    // Normal stream test
    nodeGraph->setName("normal_nodegraph");
    mx::NodePtr normal1 = nodeGraph->addNode("normal", "normal1", "vector3");
    normal1->setParameterValue("space", std::string("world"));
    output->setName("normal_output");
    output->setType("vector3");
    output->setConnectedNode(normal1);
    runValidation(".", "normal_attributes", output, validator, shaderGenerator, orthographicView, doc, log, true);
    nodeGraph->removeNode(normal1->getName());

    // Position stream test
    nodeGraph->setName("position_nodegraph");
    mx::NodePtr position1 = nodeGraph->addNode("position", "position1", "vector3");
    position1->setParameterValue("space", std::string("world"));
    output->setName("position_output");
    output->setType("vector3");
    output->setConnectedNode(position1);
    runValidation(".", "position_attributes", output, validator, shaderGenerator, orthographicView, doc, log, true);
    nodeGraph->removeNode(position1->getName());

    // Color stream test
    nodeGraph->setName("geomcolor_nodegraph");
    mx::NodePtr geomcolor1 = nodeGraph->addNode("geomcolor", "geomcolor_set0", "color3");
    geomcolor1->setParameterValue("index", 0, "integer");
    output->setName("geomcolor_output");
    output->setType("color3");
    output->setConnectedNode(geomcolor1);
    runValidation(".", "geomcolor_attributes", output, validator, shaderGenerator, orthographicView, doc, log, true);
    nodeGraph->removeNode(geomcolor1->getName());

    // Second color stream test
    nodeGraph->setName("geomcolor2_nodegraph");
    mx::NodePtr geomcolor2 = nodeGraph->addNode("geomcolor", "geomcolor_set1", "color3");
    geomcolor2->setParameterValue("index", 1, "integer");
    output->setName("geomcolor2_output");
    output->setType("color3");
    output->setConnectedNode(geomcolor2);
    runValidation(".", "geomcolor2_attributes", output, validator, shaderGenerator, orthographicView, doc, log, true);
    nodeGraph->removeNode(geomcolor2->getName());

    // Tangent stream test.
    nodeGraph->setName("tangent_nodegraph");
    mx::NodePtr tangent1 = nodeGraph->addNode("tangent", "tangent1", "vector3");
    tangent1->setParameterValue("index", 0, "integer");
    output->setName("tangent_output");
    output->setType("vector3");
    output->setConnectedNode(tangent1);
    runValidation(".", "tangent_attributes", output, validator, shaderGenerator, orthographicView, doc, log, true);
    nodeGraph->removeNode(tangent1->getName());

    // Bitangent stream test
    nodeGraph->setName("bitangent_nodegraph");
    mx::NodePtr bitangent1 = nodeGraph->addNode("bitangent", "bitangent1", "vector3");
    tangent1->setParameterValue("index", 0, "integer");
    output->setName("bitangent_output");
    output->setType("vector3");
    output->setConnectedNode(bitangent1);
    runValidation(".", "bitangent_attributes", output, validator, shaderGenerator, orthographicView, doc, log, true);
    nodeGraph->removeNode(bitangent1->getName());

    // UV stream test
    nodeGraph->setName("texcoord1_nodegraph");
    mx::NodePtr texcoord1 = nodeGraph->addNode("texcoord", "texcoord1", "vector2");
    texcoord1->setParameterValue("index", 0, "integer");
    mx::NodePtr swizzle1 = nodeGraph->addNode("swizzle", "uv_set0", "vector3");
    swizzle1->setConnectedNode("in", texcoord1);
    swizzle1->setParameterValue("channels", std::string("xy0"));
    output->setName("texcoord1_output");
    output->setType("vector3");
    output->setConnectedNode(swizzle1);
    runValidation(".", "texcoord1_attributes", output, validator, shaderGenerator, orthographicView, doc, log, true);
    nodeGraph->removeNode(texcoord1->getName());
    nodeGraph->removeNode(swizzle1->getName());

    // Second UV stream test
    nodeGraph->setName("texcoord2_nodegraph");
    mx::NodePtr texcoord2 = nodeGraph->addNode("texcoord", "texcoord2", "vector2");
    texcoord2->setParameterValue("index", 1, "integer");
    mx::NodePtr swizzle2 = nodeGraph->addNode("swizzle", "uv_set1", "vector3");
    swizzle2->setConnectedNode("in", texcoord2);
    swizzle2->setParameterValue("channels", std::string("xy0"));
    output->setName("texcoord2_output");
    output->setType("vector3");
    output->setConnectedNode(swizzle2);
    runValidation(".", "texcoord2_attributes", output, validator, shaderGenerator, orthographicView, doc, log, true);
    nodeGraph->removeNode(texcoord2->getName());
    nodeGraph->removeNode(swizzle2->getName());

    // Image test
    nodeGraph->setName("image_nodegraph");
    mx::NodePtr image1 = nodeGraph->addNode("image", "image1", "color3");
    mx::FilePath imagePath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Images/MaterialXLogo.exr");
    std::string imageName = imagePath.asString();
    image1->setParameterValue("file", imageName, "filename");
    output->setName("image1_output");
    output->setType("color3");
    output->setConnectedNode(image1);
    runValidation(".", "image_attributes", output, validator, shaderGenerator, orthographicView, doc, log, true);
    nodeGraph->removeNode(image1->getName());
}

bool endsWithCaseInsensitive(std::string str, std::string toMatch)
{
    auto it = toMatch.begin();
	  return str.size() >= toMatch.size() && std::all_of(std::next(str.begin(),str.size() - toMatch.size()), str.end(),
        [&it](const char & c)
        {
            return ::tolower(c) == ::tolower(*(it++))  ;
        });
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
      mx::GlslValidatorPtr validator = createValidator(orthographicView, "", log);

      // Set up shader generator
      mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
      shaderGenerator->registerSourceCodeSearchPath(searchPath);

      mx::FilePath path = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Tests");
      mx::StringVec files;
      mx::getDocumentsInDirectory(path, files);
      for (std::string file : files)
      {
          std::string filename = path / file;
          if (endsWithCaseInsensitive(filename, ".mtlx"))
          {
              log << "MTLX Filename: " << filename << std::endl;
              mx::DocumentPtr doc = mx::createDocument();
              readFromXmlFile(doc, filename);

              std::vector<mx::NodeGraphPtr> nodeGraphs = doc->getNodeGraphs();
              std::vector<mx::OutputPtr> outputList;
              for (mx::NodeGraphPtr nodeGraph : nodeGraphs)
              {
                  log << "NodeGraph: " << nodeGraph->getName() << std::endl;
                  std::vector<mx::OutputPtr> nodeGraphOutputs = nodeGraph->getOutputs();
                  for (mx::OutputPtr output : nodeGraphOutputs)
                  {
                      outputList.push_back(output);
                  }
              }
              loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);
              for (mx::OutputPtr output : outputList)
              {
                  log << "Output: " << output->getName() << std::endl;
                  runValidation(path, output->getName(), output, validator, shaderGenerator, orthographicView, doc, log);
              }
          }
      }
}

TEST_CASE("GLSL shading", "[shadervalid]")
{
#ifdef LOG_TO_FILE
    std::ofstream logfile("log_shadervalid_glsl_shading.txt");
    std::ostream& log(logfile);
#else
    std::ostream& log(std::cout);
#endif
    mx::DocumentPtr doc = mx::createDocument();
    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);
    // Create a validator
    bool orthographicView = true;
    mx::GlslValidatorPtr validator = createValidator(orthographicView, "shaderball.obj", log);
    // Create shader generator
    mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
    shaderGenerator->registerSourceCodeSearchPath(searchPath);
    // Set up lighting
    mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
    mx::HwShaderGenerator& hwGenerator = static_cast<mx::HwShaderGenerator&>(*shaderGenerator);
    createLightRig(doc, *lightHandler, hwGenerator);
    // Pre-clamp the number of light sources to the number bound
    size_t lightSourceCount = lightHandler->getLightSources().size();
    hwGenerator.setMaxActiveLightSources(lightSourceCount);
    mx::SgOptions options;
    //
    // Lighting test
    //
    {
        const std::string lightDoc = " \
        <?xml version=\"1.0\"?> \
        <materialx version=\"1.36\" require=\"\"> \
          <nodegraph name=\"lighting1\"> \
            <surface name=\"surface1\" type=\"surfaceshader\"> \
              <input name=\"bsdf\" type=\"BSDF\" value=\"\" nodename=\"diffusebsdf1\" /> \
              <input name=\"edf\" type=\"EDF\" value=\"\" /> \
              <input name=\"opacity\" type=\"float\" value=\"1.0\" /> \
            </surface>  \
            <diffusebsdf name=\"diffusebsdf1\" type=\"BSDF\"> \
              <input name=\"reflectance\" type=\"color3\" value=\"1.0, 1.0, 1.0\" />  \
              <input name=\"roughness\" type=\"float\" value=\"0.8\" /> \
              <input name=\"normal\" type=\"vector3\" /> \
            </diffusebsdf>  \
            <output name=\"lighting_output\" type=\"surfaceshader\" nodename=\"surface1\" /> \
          </nodegraph> \
        </materialx>";
        MaterialX::readFromXmlBuffer(doc, lightDoc.c_str());
        mx::NodeGraphPtr nodeGraph = doc->getNodeGraph("lighting1");
        mx::OutputPtr output = nodeGraph->getOutput("lighting_output");
        // Run validation
        runValidation(".", nodeGraph->getName(), output, validator, shaderGenerator, orthographicView, doc, log, true);
    }
    //
    // Materials test
    //
    {
        std::vector<mx::MaterialPtr> materials;
        createExampleMaterials(doc, materials);
        for (const mx::MaterialPtr& material : materials)
        {
            for (mx::ShaderRefPtr shaderRef : material->getShaderRefs())
            {
                const std::string name = material->getName() + "_" + shaderRef->getName();
                runValidation(".", name, shaderRef, validator, shaderGenerator, orthographicView, doc, log, true);
            }
        }
    }
}

#endif
#endif
