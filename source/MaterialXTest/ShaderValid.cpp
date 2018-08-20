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

namespace mx = MaterialX;

#include <iostream>
#include <MaterialXView/ShaderValidators/Glsl/GlslValidator.h>
#include <MaterialXView/Handlers/TinyEXRImageHandler.h>

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

TEST_CASE("GLSL Shader", "[shadervalid]")
{
#ifdef LOG_TO_FILE
    std::ofstream logfile("log_shadervalid_glsl_shader.txt");
    std::ostream& log(logfile);
#else
    std::ostream& log(std::cout);
#endif

    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::create();
    shaderGenerator->registerSourceCodeSearchPath(searchPath);

    // Create a graph testing some geometric nodes
    mx::NodeGraphPtr nodeGraph = doc->addNodeGraph("geometry_attributes");

    std::vector<mx::NodePtr> attributeList;

    // normal output test
    mx::NodePtr normal1 = nodeGraph->addNode("normal", "normal1", "vector3");
    normal1->setParameterValue("space", std::string("world"));
    attributeList.push_back(normal1);

    // position output test
    mx::NodePtr position1 = nodeGraph->addNode("position", "position1", "vector3");
    position1->setParameterValue("space", std::string("world"));
    mx::NodePtr constant = nodeGraph->addNode("constant");
    mx::ParameterPtr v = constant->addParameter("value", "vector3");
    v->setValueString("0.003921568627451, 0.003921568627451, 0.003921568627451");
    mx::NodePtr multiply1 = nodeGraph->addNode("multiply", "multiply1", "vector3");
    multiply1->setConnectedNode("in1", position1);
    multiply1->setConnectedNode("in2", constant);
    //attributeList.push_back(multiply1); -- error : Could not find a nodedef for node 'multiply1' ?
    attributeList.push_back(position1);

    // color output test
    mx::NodePtr geomcolor1 = nodeGraph->addNode("geomcolor", "geomcolor_set0", "color3");
    geomcolor1->setParameterValue("index", 0, "integer");
    attributeList.push_back(geomcolor1);

    // color output set 2 test
    mx::NodePtr geomcolor2 = nodeGraph->addNode("geomcolor", "geomcolor_set1", "color3");
    geomcolor2->setParameterValue("index", 1, "integer");
    attributeList.push_back(geomcolor2);

    // tangent output test
    mx::NodePtr tangent = nodeGraph->addNode("tangent", "tangent1", "vector3");
    //tangent->setParameterValue("index", 0, "integer"); -- not supported yet in core
    attributeList.push_back(tangent);

    // bitangent output test
    mx::NodePtr bitangent = nodeGraph->addNode("bitangent", "bitangent1", "vector3");
    //bitangent->setParameterValue("index", 0, "integer"); -- not supported yet in core
    attributeList.push_back(bitangent);

    // uv output test
    mx::NodePtr texcoord1 = nodeGraph->addNode("texcoord", "texcoord1", "vector2");
    texcoord1->setParameterValue("index", 0, "integer");
    mx::NodePtr swizzle1 = nodeGraph->addNode("swizzle", "uv_set0", "vector3");
    swizzle1->setConnectedNode("in", texcoord1);
    swizzle1->setParameterValue("channels", std::string("xy0"));
    attributeList.push_back(swizzle1);

    // uv set 2 output test
    mx::NodePtr texcoord2 = nodeGraph->addNode("texcoord", "texcoord2", "vector2");
    texcoord2->setParameterValue("index", 1, "integer");
    mx::NodePtr swizzle2 = nodeGraph->addNode("swizzle", "uv_set1", "vector3");
    swizzle2->setConnectedNode("in", texcoord2);
    swizzle2->setParameterValue("channels", std::string("xy0"));
    attributeList.push_back(swizzle2);

    // image.
    mx::FilePath imagePath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Images/MaterialXLogo.exr");
    std::string imageName = imagePath.asString();
    mx::NodePtr image = nodeGraph->addNode("image", "image1", "color3");
    image->setParameterValue("file", imageName, "filename");
    attributeList.push_back(image);

    // Connected to output.
    mx::OutputPtr output1 = nodeGraph->addOutput(mx::EMPTY_STRING, "vector3");

    // Setup lighting
    mx::HwLightHandlerPtr lightHandler = mx::HwLightHandler::create();
    mx::HwShaderGenerator& hwGenerator = static_cast<mx::HwShaderGenerator&>(*shaderGenerator);
    createLightRig(doc, *lightHandler, hwGenerator);
    // Pre-clamp the number of light sources to the number bound
    size_t lightSourceCount = lightHandler->getLightSources().size();
    hwGenerator.setMaxActiveLightSources(lightSourceCount);

    bool initialized = false;
    bool orthographicsView = true;
    mx::GlslValidatorPtr validator = mx::GlslValidator::create();
    mx::TinyEXRImageHandlerPtr imageHandler = mx::TinyEXRImageHandler::create();
    try
    {
        validator->initialize();
        validator->setImageHandler(imageHandler);
        validator->setLightHandler(lightHandler);
        const std::string geometryFile(mx::FilePath::getCurrentPath().asString() + "/documents/Geometry/shaderball.obj");
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

    mx::SgOptions options;

    for (auto nodePtr : attributeList)
    {
        log << "------------ Validate with output node as input: " << nodePtr->getName() << std::endl;
        output1->setConnectedNode(nodePtr);

        mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output1, options);
        mx::HwShaderPtr hwShader = std::dynamic_pointer_cast<mx::HwShader>(shader);
        REQUIRE(hwShader != nullptr);
        REQUIRE(hwShader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(hwShader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        mx::writeToXmlFile(doc, nodePtr->getName() + ".mtlx");

        std::ofstream file;
        file.open(nodePtr->getName() + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(nodePtr->getName() + ".frag");
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

            validator->validateRender(orthographicsView);
            std::string fileName = nodePtr->getName() + ".exr";
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
            <output name=\"out\" type=\"surfaceshader\" nodename=\"surface1\" /> \
          </nodegraph> \
        </materialx>";

        log << "------------- Validating lighting shader" << std::endl;

        MaterialX::readFromXmlBuffer(doc, lightDoc.c_str());
        mx::writeToXmlFile(doc, "lighting.mtlx");
        nodeGraph = doc->getNodeGraph("lighting1");
        mx::ElementPtr output = nodeGraph->getChild("out");

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output, options);
        mx::HwShaderPtr hwShader = std::dynamic_pointer_cast<mx::HwShader>(shader);
        REQUIRE(hwShader != nullptr);
        REQUIRE(hwShader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(hwShader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        std::ofstream file;
        file.open("lighting1.vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open("lighting1.frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();

        /////////////////////////////////////////////////////
        bool validated = false;
        MaterialX::GlslProgramPtr program = nullptr;
        try
        {
            validator->validateCreation(hwShader);
            validator->validateInputs();

            program = validator->program();
            program->printUniforms(log);
            program->printAttributes(log);

            validator->validateRender(orthographicsView);
            const std::string fileName = "lighting1.exr";
            validator->save(fileName);

            validated = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                log << e.what() << " " << error << std::endl;
            }

            if (program)
            {
                std::string stage = program->getStage(mx::HwShader::VERTEX_STAGE);
                log << ">> Failed vertex stage code:\n";
                log << stage;
                stage = program->getStage(mx::HwShader::PIXEL_STAGE);
                log << ">> Failed pixel stage code:\n";
                log << stage;
            }
        }
        REQUIRE(validated);
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
                mx::ShaderPtr shader = shaderGenerator->generate(name, shaderRef, options);
                REQUIRE(shader != nullptr);
                REQUIRE(shader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
                REQUIRE(shader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

                // Validate
                MaterialX::GlslProgramPtr program = validator->program();
                bool validated = false;
                try
                {
                    validator->validateCreation(shader);
                    validator->validateInputs();

                    program->printUniforms(log);
                    program->printAttributes(log);

                    validator->validateRender(orthographicsView);
                    std::string fileName = name + ".exr";
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
    }


}

#endif
#endif
