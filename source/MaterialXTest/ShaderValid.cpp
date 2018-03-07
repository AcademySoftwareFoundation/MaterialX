#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GlslShaderGenerator.h>
#include <MaterialXShaderGen/Util.h>
#include <MaterialXShaderGen/ShaderGenerators/Common/Swizzle.h>
#include <MaterialXShaderGen/HwShader.h>

#include <fstream>

namespace mx = MaterialX;

// Compile if module flags were set
#if defined(MATERIALX_TEST_VIEW) && defined(MATERIALX_BUILD_VIEW)
// Run only on windows for now
#if defined(_WIN32)
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
        initialized = true;
    }
    catch (mx::ExceptionShaderValidationError e)
    {
        for (auto error : e._errorLog)
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
        //"subgraph_ex2" -- cannot validate as lighting information isn't complete yet.
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
            for (auto error : e._errorLog)
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
                unsigned int type = input.second->_type;
                int location = input.second->_location;
                int size = input.second->_size;
                std::cout << "Program Uniform: \"" << input.first << "\". Location=" << location << ". Type=" << std::hex << type << ". Size=" << size << "." << std::endl;
            }
            uniformsParsed = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e._errorLog)
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
                unsigned int type = input.second->_type;
                int location = input.second->_location;
                int size = input.second->_size;
                std::cout << "Program Attribute: \"" << input.first << "\". Location=" << location << ". Type=" << std::hex << type << ". Size=" << size << "." << std::endl;
            }
            attributesParsed = true;
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e._errorLog)
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
            for (auto error : e._errorLog)
            {
                std::cout << e.what() << " " << error << std::endl;
            }
        }
        REQUIRE(renderSucceeded);

        try
        {
            mx::TinyEXRImageHandlerPtr handler = mx::TinyEXRImageHandler::creator();
            std::string fileName = shaderName + ".exr";
            validator.save(fileName, handler);
        }
        catch (mx::ExceptionShaderValidationError e)
        {
            for (auto error : e._errorLog)
            {
                std::cout << e.what() << " " << error << std::endl;
            }
        }
    }
}


TEST_CASE("GLSL Validation from HwShader", "[shadervalid]")
{
    mx::DocumentPtr doc = mx::createDocument();

    // Load standard libraries
    std::vector<std::string> filenames =
    {
        "documents/Libraries/stdlib/mx_stdlib_defs.mtlx",
        "documents/Libraries/stdlib/impl/shadergen/glsl/impl.mtlx"
    };

    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    mx::FilePath searchPath = mx::FilePath::getCurrentPath() / mx::FilePath("documents/Libraries");
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

        validator.setStages(hwShader);
        unsigned int programId = validator.createProgram();
        REQUIRE(programId > 0);
        const mx::GlslValidator::ProgramInputMap& uniforms = validator.getUniformsList();
        for (auto input : uniforms)
        {
            unsigned int type = input.second->_type;
            int location = input.second->_location;
            int size = input.second->_size;
            std::cout << "Program Uniform: \"" << input.first << "\". Location=" << location << ". Type=" << std::hex << type << ". Size=" << size << "." << std::endl;
        }
        const mx::GlslValidator::ProgramInputMap& attributes = validator.getAttributesList();
        for (auto input : attributes)
        {
            unsigned int type = input.second->_type;
            int location = input.second->_location;
            int size = input.second->_size;
            std::cout << "Program Attribute: \"" << input.first << "\". Location=" << location << ". Type=" << std::hex << type << ". Size=" << size << "." << std::endl;
        }
        validator.render();
        std::string fileName = shader->getName() + "_" + nodePtr->getName() + ".exr";
        validator.save(fileName, handler);
    }
}

#endif
#endif
