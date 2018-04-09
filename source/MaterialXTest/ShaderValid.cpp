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

#include <fstream>

namespace mx = MaterialX;

#include <iostream>
#include <MaterialXView/ShaderValidators/Glsl/GlslValidator.h>
#include <MaterialXView/Handlers/TinyEXRImageHandler.h>
#include <MaterialXView/Handlers/LightHandler.h>

extern void loadLibraries(const mx::StringVec& libraryNames, const mx::FilePath& searchPath, mx::DocumentPtr doc);
extern void createLightRig(mx::DocumentPtr doc, mx::LightHandler& lightHandler, mx::HwShaderGenerator& shadergen);

TEST_CASE("GLSL Source", "[shadervalid]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::creator();
    shaderGenerator->registerSourceCodeSearchPath(searchPath);

    mx::LightHandlerPtr lightHandler = mx::LightHandler::creator();
    createLightRig(doc, *lightHandler, static_cast<mx::HwShaderGenerator&>(*shaderGenerator));

    // Initialize a GLSL validator and set image handler.
    // Validator initiazation will create a offscreen
    // window and offscreen OpenGL context for usage.
    mx::GlslValidatorPtr validator = mx::GlslValidator::creator();
    mx::TinyEXRImageHandlerPtr handler = mx::TinyEXRImageHandler::creator();
    bool initialized = false;
    bool orthographicsView = true;
    try
    {
        validator->initialize();
        validator->setImageHandler(handler);
        // Set geometry to draw with
        const std::string geometryFile("documents/sphere.obj");
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
            std::cout << e.what() << " " << error << std::endl;
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
        "subgraph_ex2"
    };

    for (auto shaderName : shaderNames)
    {
        std::cout << "------------ Validate shader from source: " << shaderName << std::endl;
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

        if (shaderName == "subgraph_ex2")
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
                std::cout << e.what() << " " << error << std::endl;
            }

            std::string stage = program->getStage(mx::HwShader::VERTEX_STAGE);
            std::cout << ">> Failed vertex stage code:\n";
            std::cout << stage;
            stage = program->getStage(mx::HwShader::PIXEL_STAGE);
            std::cout << ">> Failed pixel stage code:\n";
            std::cout << stage;
        }
        REQUIRE(programCompiled);

        // Check getting uniforms list
        bool uniformsParsed = false;
        try 
        {
            program->printUniforms(std::cout);
            uniformsParsed = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                std::cout << e.what() << " " << error << std::endl;
            }
        }
        REQUIRE(uniformsParsed);

        // Check getting attributes list
        bool attributesParsed = false;
        try
        {
            program->printAttributes(std::cout);
            attributesParsed = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                std::cout << e.what() << " " << error << std::endl;
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
                std::cout << e.what() << " " << error << std::endl;
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
                std::cout << e.what() << " " << error << std::endl;
            }
        }
    }
}

TEST_CASE("GLSL Shader", "[shadervalid]")
{
    mx::DocumentPtr doc = mx::createDocument();

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    loadLibraries({ "stdlib", "sxpbrlib" }, searchPath, doc);

    mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::creator();
    shaderGenerator->registerSourceCodeSearchPath(searchPath);

    // Create a nonsensical graph testing some geometric nodes
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
    mx::LightHandlerPtr lightHandler = mx::LightHandler::creator();
    mx::HwShaderGenerator& hwGenerator = static_cast<mx::HwShaderGenerator&>(*shaderGenerator);
    createLightRig(doc, *lightHandler, hwGenerator);
    // Pre-clamp the number of light sources to the number bound
    size_t lightSourceCount = lightHandler->getLightSources().size();
    hwGenerator.setMaxActiveLightSources(lightSourceCount);

    bool initialized = false;
    bool orthographicsView = true;
    mx::GlslValidatorPtr validator = mx::GlslValidator::creator();
    mx::TinyEXRImageHandlerPtr imageHandler = mx::TinyEXRImageHandler::creator();
    try
    {
        validator->initialize();
        validator->setImageHandler(imageHandler);
        validator->setLightHandler(lightHandler);
        const std::string geometryFile("documents/shaderball.obj");
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
            std::cout << e.what() << " " << error << std::endl;
        }
    }
    REQUIRE(initialized);

    for (auto nodePtr : attributeList)
    {
        std::cout << "------------ Validate with output node as input: " << nodePtr->getName() << std::endl;
        output1->setConnectedNode(nodePtr);

        mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output1);
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

            program->printUniforms(std::cout);
            program->printAttributes(std::cout);

            validator->validateRender(orthographicsView);
            std::string fileName = nodePtr->getName() + ".exr";
            validator->save(fileName);

            validated = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                std::cout << e.what() << " " << error << std::endl;
            }
            
            std::string stage = program->getStage(mx::HwShader::VERTEX_STAGE);
            std::cout << ">> Failed vertex stage code:\n";
            std::cout << stage;
            stage = program->getStage(mx::HwShader::PIXEL_STAGE);
            std::cout << ">> Failed pixel stage code:\n";
            std::cout << stage;
        }
        REQUIRE(validated);
    }

    //
    // Lighting test
    //
    {
        const std::string lightDoc = " \
        <?xml version=\"1.0\"?> \
        <materialx version=\"1.35\" require=\"\"> \
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

        std::cout << "------------- Validating lighting shader" << std::endl;

        MaterialX::readFromXmlBuffer(doc, lightDoc.c_str());
        mx::writeToXmlFile(doc, "lighting.mtlx");
        nodeGraph = doc->getNodeGraph("lighting1");
        mx::ElementPtr output = nodeGraph->getChild("out");

        // Test shader generation from nodegraph output
        mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output);
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
        try
        {
            validator->validateCreation(hwShader);
            validator->validateInputs();

            MaterialX::GlslProgramPtr program = validator->program();
            program->printUniforms(std::cout);
            program->printAttributes(std::cout);

            validator->validateRender(orthographicsView);
            const std::string fileName = "lighting1.exr";
            validator->save(fileName);

            validated = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                std::cout << e.what() << " " << error << std::endl;
            }
        }
        REQUIRE(validated);
    }
}

#endif
#endif
