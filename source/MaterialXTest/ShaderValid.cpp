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
#include <MaterialXView/Image/TinyEXRImageHandler.h>

TEST_CASE("GLSL Validation from Source", "[shadervalid]")
{
    // Initialize a GLSL validator. Will initialize 
    // window and context as well for usage
    mx::GlslValidator validator;
    bool initialized = false;
    try
    {
        validator.initialize();
        mx::TinyEXRImageHandlerPtr handler = mx::TinyEXRImageHandler::creator();
        validator.setImageHandler(handler);
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

        unsigned int stagesSet = 0;
        std::stringstream vertexShaderStream;
        std::stringstream pixelShaderStream;
        std::ifstream shaderFile;
        shaderFile.open(vertexShaderPath);
        if (shaderFile.is_open())
        {
            vertexShaderStream << shaderFile.rdbuf();
            validator.setStage(vertexShaderStream.str(), mx::HwShader::VERTEX_STAGE);
            shaderFile.close();
            stagesSet++;
        }
        shaderFile.open(pixelShaderPath);
        if (shaderFile.is_open())
        {
            pixelShaderStream << shaderFile.rdbuf();
            validator.setStage(pixelShaderStream.str(), mx::HwShader::PIXEL_STAGE);
            shaderFile.close();
            stagesSet++;
        }

        // To do: Make the dependence on ShaderGen test generated files more explicit
        // so as to avoid the possibility of failure here. For now skip tests if files not
        // found.
        //REQUIRE(stagesSet == 2);
        if (stagesSet != 2)
        {
            continue;
        }

        // Check program compilation
        bool programCompiled = false;
        try
        {
            validator.createProgram();
            programCompiled = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e.errorLog())
            {
                std::cout << e.what() << " " << error << std::endl;
            }
        }
        REQUIRE(programCompiled);

        // Check getting uniforms list
        bool uniformsParsed = false;
        try {
            const mx::GlslValidator::ProgramInputMap& uniforms = validator.getUniformsList();
            for (auto input : uniforms)
            {
                unsigned int gltype = input.second->gltype;
                int location = input.second->location;
                int size = input.second->size;
                std::string type = input.second->typeString;
                std::string value = input.second->value ? input.second->value->getValueString() : "<none>";
                std::cout << "Program Uniform: \"" << input.first
                    << "\". Location=" << location 
                    << ". Type=" << std::hex << gltype 
                    << ". Size=" << std::dec << size
                    << ". TypeString=" << type 
                    << ". Value=" << value << "."
                    << std::endl;
            }
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
            const mx::GlslValidator::ProgramInputMap& attributes = validator.getAttributesList();
            for (auto input : attributes)
            {
                unsigned int gltype = input.second->gltype;
                int location = input.second->location;
                int size = input.second->size;
                std::string type = input.second->typeString;
                std::string value = input.second->value ? input.second->value->getValueString() : "<none>";
                std::cout << "Program Attribute: \"" << input.first
                    << "\". Location=" << location
                    << ". Type=" << std::hex << gltype
                    << ". Size=" << std::dec << size
                    << ". TypeString=" << type 
                    << ". Value=" << value << "."
                    << std::endl;
            }
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
            validator.render();
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
            validator.save(fileName);
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

// Utility methods from ShaderGen tests
extern mx::InterfaceElementPtr findImplementation(mx::DocumentPtr document, const std::string& name,
    const std::string& language, const std::string& target);
extern void bindLightShaders(mx::DocumentPtr document, mx::HwShaderGenerator& shadergen);

TEST_CASE("GLSL Validation from HwShader", "[shadervalid]")
{
    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/stdlib/impl/shadergen/glsl/impl.mtlx",
        "documents/Libraries/sx/sx_defs.mtlx",
        "documents/Libraries/sx/impl/shadergen/glsl/impl.mtlx",
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
    mx::ShaderGeneratorPtr shaderGenerator = mx::GlslShaderGenerator::creator();
    shaderGenerator->registerSourceCodeSearchPath(searchPath);
    // Make sure to specify lighting definitions.
    mx::HwShaderGenerator* hwShaderGenerator = static_cast<mx::HwShaderGenerator*>(shaderGenerator.get());
    hwShaderGenerator->setMaxActiveLightSources(2);
    bindLightShaders(doc, *hwShaderGenerator);

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
    mx::NodePtr geomcolor1 = nodeGraph->addNode("geomcolor", "geomcolor1", "color3");
    geomcolor1->setParameterValue("index", 0, "integer");
    attributeList.push_back(geomcolor1);

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
    mx::NodePtr swizzle1 = nodeGraph->addNode("swizzle", "swizzle_uv", "vector3");
    swizzle1->setConnectedNode("in", texcoord1);
    swizzle1->setParameterValue("channels", std::string("xy0"));
    attributeList.push_back(swizzle1);

    // image
    mx::FilePath imagePath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Images/MaterialXLogo.exr");
    std::string imageName = imagePath.asString();
    mx::NodePtr image = nodeGraph->addNode("image", "image1", "color3");
    image->setParameterValue("file", imageName, "filename");
    attributeList.push_back(image);

    // Connected to output.
    mx::OutputPtr output1 = nodeGraph->addOutput(mx::EMPTY_STRING, "vector3");

    bool initialized = false;
    mx::GlslValidator validator;
    try
    {
        validator.initialize();
        mx::TinyEXRImageHandlerPtr handler = mx::TinyEXRImageHandler::creator();
        validator.setImageHandler(handler);
        initialized = true;
    }
    catch (mx::ExceptionShaderValidationError e)
    {
    }
    REQUIRE(initialized);

    mx::TinyEXRImageHandlerPtr handler = mx::TinyEXRImageHandler::creator();

    for (auto nodePtr : attributeList)
    {
        std::cout << "------------ Validate with output node: " << nodePtr->getName() << std::endl;
        output1->setConnectedNode(nodePtr);

        mx::ShaderPtr shader = shaderGenerator->generate(nodeGraph->getName(), output1);
        mx::HwShaderPtr hwShader = std::dynamic_pointer_cast<mx::HwShader>(shader);
        REQUIRE(hwShader != nullptr);
        REQUIRE(hwShader->getSourceCode(mx::HwShader::PIXEL_STAGE).length() > 0);
        REQUIRE(hwShader->getSourceCode(mx::HwShader::VERTEX_STAGE).length() > 0);

        std::ofstream file;
        file.open(nodePtr->getName() + ".vert");
        file << shader->getSourceCode(mx::HwShader::VERTEX_STAGE);
        file.close();
        file.open(nodePtr->getName() + ".frag");
        file << shader->getSourceCode(mx::HwShader::PIXEL_STAGE);
        file.close();


        validator.setStages(hwShader);
        unsigned int programId = validator.createProgram();
        REQUIRE(programId > 0);
        const mx::GlslValidator::ProgramInputMap& uniforms = validator.getUniformsList();
        for (auto input : uniforms)
        {
            unsigned int gltype = input.second->gltype;
            int location = input.second->location;
            int size = input.second->size;
            std::string type = input.second->typeString;
            std::string value = input.second->value ? input.second->value->getValueString() : "<none>";
            std::cout << "Program Uniform: \"" << input.first
                << "\". Location=" << location
                << ". Type=" << std::hex << gltype
                << ". Size=" << std::dec << size
                << ". TypeString=" << type
                << ". Value=" << value << "."
                << std::endl;

        }
        const mx::GlslValidator::ProgramInputMap& attributes = validator.getAttributesList();
        for (auto input : attributes)
        {
            unsigned int gltype = input.second->gltype;
            int location = input.second->location;
            int size = input.second->size;
            std::string type = input.second->typeString;
            std::string value = input.second->value ? input.second->value->getValueString() : "<none>";
            std::cout << "Program Attribute: \"" << input.first
                << "\". Location=" << location
                << ". Type=" << std::hex << gltype
                << ". Size=" << std::dec << size
                << ". TypeString=" << type 
                << ". Value=" << value << "."
                << std::endl;
        }
        validator.render();
        std::string fileName = nodePtr->getName() + ".exr";
        validator.save(fileName);
    }
#if 0
    /////////////////////////////////
    {
        const std::string lightDoc = " \
        <?xml version=\"1.0\"?> \
        <materialx version=\"1.35\" require=\"\"> \
          <nodegraph name=\"lighting1\" type=\"\" xpos=\"10.9034\" ypos=\"15.42\" adskDisplayMode=\"1\"> \
            <surface name=\"surface1\" type=\"surfaceshader\" xpos=\"4.08694\" ypos=\"8.7068\" adskDisplayMode=\"2\"> \
              <input name=\"bsdf\" type=\"BSDF\" value=\"\" nodename=\"layeredbsdf1\" channels=\"\" /> \
              <input name=\"edf\" type=\"EDF\" value=\"\" /> \
              <input name=\"opacity\" type=\"float\" value=\"1.0\" /> \
            </surface>  \
            <diffusebsdf name=\"diffusebsdf1\" type=\"BSDF\" xpos=\"2.3949\" ypos=\"9.00162\" adskDisplayMode=\"2\"> \
              <input name=\"reflectance\" type=\"color3\" value=\"0.2, 0.3, 1.0\" />  \
              <input name=\"roughness\" type=\"float\" value=\"1.0\" /> \
              <input name=\"normal\" type=\"vector3\" value=\"0.0, 0.0, 0.0\" /> \
            </diffusebsdf>  \
            <output name=\"out\" type=\"surfaceshader\" nodename=\"surface1\" channels=\"\" /> \
            <coatingbsdf name=\"coatingbsdf1\" type=\"BSDF\" xpos=\"2.31043\" ypos=\"15.1291\" adskDisplayMode=\"2\"> \
              <input name=\"reflectance\" type=\"color3\" value=\"1.0, 1.0, 1.0\" /> \
              <input name=\"ior\" type=\"float\" value=\"1.52\" /> \
              <input name=\"roughness\" type=\"float\" value=\"0.2\" /> \
              <input name=\"anisotropy\" type=\"float\" value=\"0.0\" /> \
              <input name=\"normal\" type=\"vector3\" value=\"0.0, 0.0, 0.0\" /> \
              <input name=\"tangent\" type=\"vector3\" value=\"0.0, 0.0, 0.0\" /> \
              <input name=\"distribution\" type=\"string\" value=\"ggx\" /> \
              <input name=\"base\" type=\"BSDF\" value=\"\" /> \
            </coatingbsdf> \
            <layeredbsdf name=\"layeredbsdf1\" type=\"BSDF\" xpos=\"4.12788\" ypos=\"15.5871\" adskDisplayMode=\"2\"> \
              <input name=\"top\" type=\"BSDF\" value=\"\" nodename=\"coatingbsdf1\" channels=\"\" /> \
              <input name=\"base\" type=\"BSDF\" value=\"\" nodename=\"diffusebsdf1\" channels=\"\" /> \
              <input name=\"weight\" type=\"float\" value=\"0.5000\" /> \
            </layeredbsdf> \
          </nodegraph> \
        </materialx>";

        std::cout << "------------- Validating lighting shader" << std::endl;

        MaterialX::readFromXmlBuffer(doc, lightDoc.c_str());
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
        validator.setStages(hwShader);
        unsigned int programId = validator.createProgram();
        REQUIRE(programId > 0);
        const mx::GlslValidator::ProgramInputMap& uniforms = validator.getUniformsList();
        for (auto input : uniforms)
        {
            unsigned int gltype = input.second->gltype;
            int location = input.second->location;
            int size = input.second->size;
            std::string type = input.second->typeString;
            std::string value = input.second->value ? input.second->value->getValueString() : "<none>";
            std::cout << "Program Uniform: \"" << input.first
                << "\". Location=" << location
                << ". Type=" << std::hex << gltype
                << ". Size=" << std::dec << size
                << ". TypeString=" << type
                << ". Value=" << value << "."
                << std::endl;

        }
        const mx::GlslValidator::ProgramInputMap& attributes = validator.getAttributesList();
        for (auto input : attributes)
        {
            unsigned int gltype = input.second->gltype;
            int location = input.second->location;
            int size = input.second->size;
            std::string type = input.second->typeString;
            std::string value = input.second->value ? input.second->value->getValueString() : "<none>";
            std::cout << "Program Attribute: \"" << input.first
                << "\". Location=" << location
                << ". Type=" << std::hex << gltype
                << ". Size=" << std::dec << size
                << ". TypeString=" << type
                << ". Value=" << value << "."
                << std::endl;
        }
        validator.render();
        std::string fileName = "lighting1.exr";
        validator.save(fileName);
    }
#endif
}

#endif
#endif
