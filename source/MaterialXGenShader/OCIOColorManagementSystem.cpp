//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/OCIOColorManagementSystem.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <iostream>

#ifdef OpenColorIO_SKIP_IMPORTS
#error "OCIO has been build as a non-static library"
#endif
#pragma warning(disable: 4275)
#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

namespace MaterialX
{

const string OCIOColorManagementSystem::CMS_NAME = "ocio_cms";

// Defines an unsupported language.
const int OCIO_UNSUPPORT_LANGUAGE = -1;

// Mapping from supported MaterialX targets to OCIO shading languages
using OCIOLanguageMap = std::unordered_map<std::string, OCIO::GpuLanguage>;

// OCIO related information
class OCIOInformation
{
  public:
    // OCIO config file to use
    OCIO::ConstConfigRcPtr config = nullptr;
    // Target to language map
    OCIOLanguageMap languageMap;
    // Language used for code generation (is a OCIO::GpuLanguage)
    int language = OCIO_UNSUPPORT_LANGUAGE;
    // MaterialX target
    std::string target;
};

OCIOColorManagementSystemPtr OCIOColorManagementSystem::create(const string& target)
{
    return OCIOColorManagementSystemPtr(new OCIOColorManagementSystem(target));
}

OCIOColorManagementSystem::OCIOColorManagementSystem(const string& target) :
    _target(target)
{
    _ocioInfo = new OCIOInformation();
    _ocioInfo->language = OCIO_UNSUPPORT_LANGUAGE;
    _ocioInfo->config = nullptr;
    _ocioInfo->target = EMPTY_STRING;
    _ocioInfo->languageMap["genglsl"] = OCIO::GPU_LANGUAGE_GLSL_4_0;
}

OCIOColorManagementSystem::~OCIOColorManagementSystem()
{
    delete _ocioInfo;
}

bool OCIOColorManagementSystem::isValid() const
{
    return (_document && _ocioInfo->config && _ocioInfo->language != OCIO_UNSUPPORT_LANGUAGE);
}

void OCIOColorManagementSystem::loadLibrary(DocumentPtr document) 
{
    _ocioInfo->language = OCIO_UNSUPPORT_LANGUAGE;
    _ocioInfo->target = EMPTY_STRING;

    _document = document;
    if (!_document)
    {
        throw ExceptionShaderGenError("No library loaded for color management system");
    }

    TargetDefPtr targetDef = _document->getTargetDef(_target);
    if (targetDef)
    { 
        for (const string& target : targetDef->getMatchingTargets())
        {
            if (_ocioInfo->languageMap.count(target))
            {
                // Note that we store the "base" target instead of any derived target
                // as this is the actual shading language.
                _ocioInfo->language = static_cast<int>(OCIO::GPU_LANGUAGE_GLSL_4_0);
                _ocioInfo->target = target;
                break;
            }
        }
    }
}

bool OCIOColorManagementSystem::readConfigFile(const FilePath& configFile)
{
    if (_configFile != configFile && configFile.exists())
    {
        _configFile = EMPTY_STRING;
        _ocioInfo->config = nullptr;

        try
        {
            _ocioInfo->config = OCIO::Config::CreateFromFile(configFile.asString().c_str());
            if (_ocioInfo->config)
            {
                _configFile = configFile;
                return (_ocioInfo->config->getNumColorSpaces() > 0);
            }
        }
        catch (const OCIO::Exception& e)
        {
            std::cout << "Error reading config file: '" + configFile.asString() + "' Error : " + e.what() << std::endl;
            // Do not continue to throw an exception but instead use return status code
        }
    }
    return false;
}

/// Returns whether this color management system supports a provided transform
bool OCIOColorManagementSystem::supportsTransform(const ColorSpaceTransform& transform) const
{
    if (!isValid())
    {
        return false;
    }

    OCIO::ConstProcessorRcPtr processor = nullptr;
    try
    {
        processor = _ocioInfo->config->getProcessor(transform.sourceSpace.c_str(), transform.targetSpace.c_str());
    }
    catch (const OCIO::Exception&)
    {
        // Do not throw an exception here but just return a status
        return false;
    }
    return (nullptr != processor);
}

/// Create a node to use to perform the given color space transformation.
ShaderNodePtr OCIOColorManagementSystem::createNode(const ShaderGraph* parent, const ColorSpaceTransform& transform, 
                                                    const string& name, GenContext& context) const
{
    if (!isValid())
    {
        return nullptr;
    }

    if (transform.type != Type::COLOR3 && transform.type != Type::COLOR4)
    {
        throw ExceptionShaderGenError("Invalid type specified to color transform: '" + transform.type->getName() + "'");
    }

    ImplementationPtr impl = getImplementation(transform);

    if (!impl)
    {
        throw ExceptionShaderGenError("No implementation found for transform: ('" + transform.sourceSpace + "', '" + transform.targetSpace + "').");
    }

    // Check if it's created and cached already, otherwise create and cache it.
    ShaderNodeImplPtr nodeImpl = context.findNodeImplementation(impl->getName());
    if (!nodeImpl)
    {
        // Note: Use custom OCIO "source code node".
        nodeImpl = OCIOSourceCodeNode::create();
        nodeImpl->initialize(*impl, context);
        context.addNodeImplementation(impl->getName(), nodeImpl);
    }

    // Create the node.
    ShaderNodePtr shaderNode = ShaderNode::create(parent, name, nodeImpl, ShaderNode::Classification::TEXTURE);

    // Create ports on the node.
    ShaderInput* input = shaderNode->addInput("in", Type::COLOR4);
    input->setValue(Value::createValue(Color4(0.0f, 0.0f, 0.0f, 1.0)));
    shaderNode->addOutput("out", Type::COLOR4);

    return shaderNode;
}

ImplementationPtr OCIOColorManagementSystem::getImplementation(const ColorSpaceTransform& transform) const
{
    if (!isValid())
    {
        return false;
    }

    OCIO::ConstConfigRcPtr config = _ocioInfo->config;

    // Set up process for a color space transform
    try
    {
        OCIO::ConstProcessorRcPtr processor = config->getProcessor(transform.sourceSpace.c_str(), transform.targetSpace.c_str());
        if (!processor)
        {
            return nullptr;
        }

        // Get a GPU processor
        OCIO::ConstGPUProcessorRcPtr gpu = processor->getDefaultGPUProcessor();
        if (!gpu)
        {
            return nullptr;
        }

        // Create a shader descriptor
        //
        OCIO::GpuShaderDescRcPtr shaderDesc = OCIO::GpuShaderDesc::CreateShaderDesc();
        if (!shaderDesc)
        {
            return nullptr;
        }

        shaderDesc->setLanguage(static_cast<OCIO::GpuLanguage>(_ocioInfo->language));
        const std::string& typeName = transform.type->getName();
        // Note that the source and target space strings may contain invalid characters so we need to ensure that
        // a valid function name is created here
        std::string transformFunctionName = "IM_" + createValidName(transform.sourceSpace) + "_to_" + createValidName(transform.targetSpace) + "_" + typeName + "_ocio";
        shaderDesc->setFunctionName(transformFunctionName.c_str());
        
        // Retrieve information
        gpu->extractGpuShaderInfo(shaderDesc);

        std::string fullFunction = shaderDesc->getShaderText();
        std::string functionName = shaderDesc->getFunctionName();
        std::string outputName = shaderDesc->getPixelName();

        // The following is an example of output returned in "fullFunction" 
        //
        // In this example "acescg" to "lin_rec709" are the source and target space strings passed in.
        // - setFunctionName() is used to in place of the default function name
        // - There is no need to replace the output variable name.
        // - getShaderText() returns the contents shown here verbatim
        //  - This includes all comment strings.
        /*
        // Declaration of the OCIO shader function

        vec4 IM_acescg_to_lin_rec709_color3_ocio(in vec4 inPixel)
        {
            vec4 outColor = inPixel;

            // Add a Matrix processing

            outColor = mat4(1.7049986809609248, -0.13025866763271018, -0.024007514741958086, 0., -0.62174765641381313,
            1.1408020008115505, -0.12898266957287285, 0., -0.083251260888346112, -0.010544731974266384, 1.1529906045288774, 0., 0., 0., 0., 1.)
            * outColor;

            return outColor;
        }
        */

        // Create an implementation based on source code
        ImplementationPtr impl = _document->getImplementation(transformFunctionName);
        if (!impl)
        {
            impl = _document->addImplementation(transformFunctionName);

            // Note: There is only one input so we just use the default name: "inPixel".
            impl->addInput("inPixel", typeName);
            impl->addOutput(outputName, typeName);
            impl->setAttribute("sourcecode", fullFunction);
            impl->setAttribute("function", functionName);
            impl->setAttribute("target", _ocioInfo->target);
        }
        return impl;
    }
    catch (const OCIO::Exception& e)
    {
        throw ExceptionShaderGenError("Code generation failed for transform: '" + 
            transform.sourceSpace + "' to '" + transform.targetSpace + "'. Type: '" + transform.type->getName()  + "' : " + e.what());
    }

    return nullptr;
}

//
// OCIO source code node method
//

ShaderNodeImplPtr OCIOSourceCodeNode::create()
{
    return std::make_shared<OCIOSourceCodeNode>();
}

void OCIOSourceCodeNode::emitFunctionDefinition(const ShaderNode&, GenContext& context, ShaderStage& stage) const
{
    // Write source code as is
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
    if (!_functionSource.empty())
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        shadergen.emitBlock(_functionSource, context, stage);
        shadergen.emitLineBreak(stage);
    }
    END_SHADER_STAGE(stage, Stage::PIXEL)
}

void OCIOSourceCodeNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    // The main variation from the default "hardware source code node" logic is that OCIO always returns
    // a value from function invocation, so modify caller logic to declare a local variable and assign it
    // the return value from the OCIO shader function.
    //
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)

    const ShaderGenerator& shadergen = context.getShaderGenerator();

    // Declare the output variable
    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(node.getOutput(0), true, false, context, stage);
    shadergen.emitString(" = ", stage);

    // Emit function name. 
    shadergen.emitString(_functionName + "(", stage);

    // Emit input. 
    ShaderInput* input = node.getInputs()[0];
    shadergen.emitInput(input, context, stage);

    // End function call
    shadergen.emitString(")", stage);
    shadergen.emitLineEnd(stage);

    END_SHADER_STAGE(stage, Stage::PIXEL)
}

} // namespace MaterialX
