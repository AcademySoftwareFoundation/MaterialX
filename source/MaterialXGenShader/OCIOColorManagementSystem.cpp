//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/OCIOColorManagementSystem.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Value.h>
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

// Input name on implementation
const string IN_PIXEL_STRING = "inPixel";

const size_t OCIO_RESOURCEMAP_SIZE = size_t(ColorManagementSystem::ResourceType::TEXTURE3D) + 1;

// OCIO related information
class OCIOInformation
{
  public:
    OCIOInformation::OCIOInformation()
    {
        for (size_t i = 0; i < OCIO_RESOURCEMAP_SIZE; i++)
        {
            resourceMap[i] = std::make_shared<ColorManagementResourceMap>();
        }
    }
    virtual OCIOInformation::~OCIOInformation()
    {
        resourceMap->reset();
    }

    // OCIO config file to use
    OCIO::ConstConfigRcPtr config = nullptr;
    // Target to language map
    OCIOLanguageMap languageMap;
    // Language used for code generation (is a OCIO::GpuLanguage)
    int language = OCIO_UNSUPPORT_LANGUAGE;
    // MaterialX target
    std::string target;

    // Uniform resources 
    void updateUniformResources(OCIO::GpuShaderDescRcPtr shaderDesc);

    // Texture resources 
    void updateTextureResources(OCIO::GpuShaderDescRcPtr shaderDesc);

    ColorManagementResourceMapPtr resourceMap[OCIO_RESOURCEMAP_SIZE];
};

void OCIOInformation::updateUniformResources(OCIO::GpuShaderDescRcPtr shaderDesc)
{
    ColorManagementResourceMap& rmap = *(resourceMap[(int)ColorManagementSystem::ResourceType::UNIFORM]);

    if (!shaderDesc)
    {
        return;
    }

    unsigned int uniformCount = shaderDesc->getNumUniforms();
    for (unsigned int i = 0; i < uniformCount; i++)
    {
        OCIO::GpuShaderDesc::UniformData uniformData;
        std::string uniformName = shaderDesc->getUniform(i, uniformData);
        switch (uniformData.m_type)
        {
            case OCIO::UniformDataType::UNIFORM_BOOL:
            {
                // Note: Booleans become float uniforms
                ValuePtr val = Value::createValue(uniformData.m_getBool() ? 1.0f : 0.0f);
                rmap[uniformName] = ColorSpaceConstant::create(uniformName, val);
                break;
            }
            case OCIO::UniformDataType::UNIFORM_DOUBLE:
            {
                ValuePtr val = Value::createValue(static_cast<float>(uniformData.m_getDouble()));
                rmap[uniformName] = ColorSpaceConstant::create(uniformName, val);
                break;
            }
            case OCIO::UniformDataType::UNIFORM_FLOAT3:
            {
                Vector3 vec3 = { static_cast<float>(uniformData.m_getFloat3()[0]),
                                 static_cast<float>(uniformData.m_getFloat3()[1]),
                                 static_cast<float>(uniformData.m_getFloat3()[2]) };
                ValuePtr val = Value::createValue(vec3);
                rmap[uniformName] = ColorSpaceConstant::create(uniformName, val);
                break;
            }
            case OCIO::UniformDataType::UNIFORM_VECTOR_FLOAT:
            {
                size_t vectorSize = static_cast<size_t>(uniformData.m_vectorFloat.m_getSize());
                const float* data = static_cast<const float*>(uniformData.m_vectorFloat.m_getVector());
                FloatVec vecarray(data, data+vectorSize);
                ValuePtr val = Value::createValue(vecarray);
                rmap[uniformName] = ColorSpaceConstant::create(uniformName, val);
                break;
            }
            case OCIO::UniformDataType::UNIFORM_VECTOR_INT:
            {
                size_t vectorSize = static_cast<size_t>(uniformData.m_vectorInt.m_getSize());
                const int* data = static_cast<const int*>(uniformData.m_vectorInt.m_getVector());

                IntVec vecarray(data, data+vectorSize);
                ValuePtr val = Value::createValue(vecarray);
                rmap[uniformName] = ColorSpaceConstant::create(uniformName, val);
                break;
            }
            default:
            {
                break;
            }
        };
    }
}

void OCIOInformation::updateTextureResources(OCIO::GpuShaderDescRcPtr shaderDesc)
{
    // Process 3d textures
    ColorManagementResourceMap& rmap3D = *(resourceMap[(int)ColorManagementSystem::ResourceType::TEXTURE3D]);

    unsigned int textureCount = shaderDesc->getNum3DTextures();
    for (unsigned idx = 0; idx < textureCount; ++idx)
    {
        const char * textureName = nullptr;
        const char * samplerName = nullptr;
        unsigned int edgeLength = 0;
        OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
        shaderDesc->get3DTexture(idx, textureName, samplerName, edgeLength, interpolation);

        if (!textureName || !*textureName
            || !samplerName || !*samplerName
            || edgeLength == 0)
        {
            throw Exception("OCIO 3D texture data is corrupted: " + std::string(textureName));
        }

        const float* data = nullptr;
        shaderDesc->get3DTextureValues(idx, data);
        if (!data)
        {
            throw Exception("OCIO 3D texture values are missing: " + std::string(textureName));
        }

        size_t offset = static_cast<size_t>(edgeLength * edgeLength* edgeLength);
        FloatVec vecarray(data, data + offset);
        ColorSpaceTexturePtr newTexture = ColorSpaceTexture::create(samplerName, vecarray);
        newTexture->width = newTexture->height = newTexture->depth = edgeLength;
        newTexture->channelCount = 3;
        if (interpolation == OCIO::INTERP_LINEAR)
        {
            newTexture->interpolation = ColorSpaceTexture::InterpolationType::LINEAR;
        }
        else if (interpolation == OCIO::INTERP_CUBIC)
        {
            newTexture->interpolation = ColorSpaceTexture::InterpolationType::CUBIC;
        }
        else
        {
            newTexture->interpolation = ColorSpaceTexture::InterpolationType::NEAREST;
        }
        rmap3D[samplerName] = newTexture;
    }

    // Process 1D and 2D Textures
    ColorManagementResourceMap& rmap1D = *(resourceMap[(int)ColorManagementSystem::ResourceType::TEXTURE1D]);
    ColorManagementResourceMap& rmap2D = *(resourceMap[(int)ColorManagementSystem::ResourceType::TEXTURE2D]);

    textureCount = shaderDesc->getNumTextures();
    for (unsigned idx = 0; idx < textureCount; ++idx)
    {
        const char * textureName = nullptr;
        const char * samplerName = nullptr;
        unsigned width = 0;
        unsigned height = 0;
        OCIO::Interpolation interpolation = OCIO::INTERP_LINEAR;
        OCIO::GpuShaderDesc::TextureType channel = OCIO::GpuShaderDesc::TEXTURE_RGB_CHANNEL;
        shaderDesc->getTexture(idx, textureName, samplerName, width, height, channel, interpolation);

        if (!textureName || !*textureName
            || !samplerName || !*samplerName
            || width == 0)
        {
            throw Exception("OCIO 1D texture data is corrupted: " + std::string(textureName));
        }

        const float* data = 0x0;
        shaderDesc->getTextureValues(idx, data);
        if (!data)
        {
            throw Exception("OCIO 1D texture values are missing: " + std::string(textureName));
        }

        size_t offset = static_cast<size_t>(width*height);
        FloatVec vecarray(data, data + offset);        

        ColorSpaceTexturePtr newTexture = ColorSpaceTexture::create(samplerName, vecarray);
        newTexture->width = width;
        newTexture->height = height;
        newTexture->depth = 1;
        newTexture->channelCount = (channel == OCIO::GpuShaderDesc::TEXTURE_RGB_CHANNEL) ? 3 : 1;

        if (height > 1)
        {
            rmap2D[samplerName] = newTexture;
        }
        else
        {
            rmap1D[samplerName] = newTexture;
        }
    }
}

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
        catch (const OCIO::Exception& /*e*/)
        {
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
    catch (const OCIO::Exception&e)
    {
        std::cout << "FAILED supports transform: " + transform.sourceSpace + " to: " + transform.targetSpace + std::string(e.what()) << std::endl;
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
        shaderDesc->setResourcePrefix(transformFunctionName.c_str());
        
        // Retrieve information
        gpu->extractGpuShaderInfo(shaderDesc);

        std::string fullFunction = shaderDesc->getShaderText();
        // TODO: Comment out uniforms as a workaround as the uniforms are always embedded as
        // part of the function string
        StringMap uniformDecl;
        uniformDecl["uniform"] = "//uniform";
        fullFunction = replaceSubstrings(fullFunction, uniformDecl);

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

        // Cache binding information
        _ocioInfo->updateUniformResources(shaderDesc);
        _ocioInfo->updateTextureResources(shaderDesc);
       
        // Create an implementation based on source code
        ImplementationPtr impl = _document->getImplementation(transformFunctionName);
        if (!impl)
        {
            impl = _document->addImplementation(transformFunctionName);

            // Note: There is only one input so we just use the default name: "inPixel".
            InputPtr pixelInput = impl->addInput(IN_PIXEL_STRING, typeName);
            pixelInput->setIsUniform(false);
            impl->addOutput(outputName, typeName);
            impl->setAttribute("sourcecode", fullFunction);
            impl->setAttribute("function", functionName);
            impl->setAttribute("target", _ocioInfo->target);

            // Add all of the additional uniforms as additional inputs on the impl so it can be accessed later one in the shader node ?
            for (auto rmap : _ocioInfo->resourceMap)
            {
                if (_ocioInfo->resourceMap[(int)ColorManagementSystem::ResourceType::UNIFORM] == rmap)
                {
                    for (auto rmapItem : *rmap)
                    {
                        ColorSpaceConstantPtr uniformItem = std::static_pointer_cast<ColorSpaceConstant>(rmapItem.second);
                        ValuePtr uniformValue = uniformItem->_value;
                        InputPtr newInput = impl->addInput(rmapItem.first, uniformValue->getTypeString());
                        if (newInput)
                        {
                            newInput->setValue(uniformValue->getValueString(), uniformValue->getTypeString());
                            newInput->setIsUniform(true);
                        }
                    }
                }
                else
                {
                    for (auto rmapItem : *rmap)
                    {
                        ColorSpaceTexturePtr uniformItem = std::static_pointer_cast<ColorSpaceTexture>(rmapItem.second);
                        const FloatVec& vecarray = uniformItem->data;
                        ValuePtr uniformValue = Value::createValue(vecarray);
                        InputPtr newInput = impl->addInput(rmapItem.first, uniformValue->getTypeString());
                        if (newInput)
                        {
                            newInput->setValue(uniformValue->getValueString(), uniformValue->getTypeString());
                            newInput->setAttribute("width", std::to_string(uniformItem->width));
                            newInput->setAttribute("height", std::to_string(uniformItem->height));
                            newInput->setAttribute("depth", std::to_string(uniformItem->depth));
                            newInput->setAttribute("channels", std::to_string(uniformItem->channelCount));
                            newInput->setIsUniform(true);
                        }
                    }
                }
            }
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

const ColorManagementResourceMapPtr OCIOColorManagementSystem::getResource(ResourceType resourceType) const
{
    if (_ocioInfo)
    {
        return _ocioInfo->resourceMap[(int)resourceType];
    }
    return nullptr;
}

void OCIOColorManagementSystem::clearResources()
{
    if (_ocioInfo)
    {
        _ocioInfo->resourceMap->reset();
    }
}


//
// OCIO source code node method
//

OCIOSourceCodeNode::OCIOSourceCodeNode()
    : _cmUniforms(ColorManagementSystem::COLOR_MANAGEMENT_UNIFORMS, EMPTY_STRING)
{
}

ShaderNodeImplPtr OCIOSourceCodeNode::create()
{
    return std::make_shared<OCIOSourceCodeNode>();
}

void OCIOSourceCodeNode::initialize(const InterfaceElement& element, GenContext& context)
{
    SourceCodeNode::initialize(element, context);

    // Cache a uniform block
    for (auto nodeInput : element.getInputs())
    {
        if (nodeInput->getName() != IN_PIXEL_STRING)
        {
            string uniformType = nodeInput->getType();
            if (uniformType == "floatarray" || uniformType == "intarray")
            {
                // This will create a sampler. 
                // NOTE: The value is never set here even though it is storable as is. 
                _cmUniforms.add(Type::FILENAME, nodeInput->getName());
            }
            else
            {
                _cmUniforms.add(TypeDesc::get(nodeInput->getType()), nodeInput->getName(), nodeInput->getValue());
            }
        }
    }
}

void OCIOSourceCodeNode::createVariables(const ShaderNode& /*node*/, GenContext& /*context*/, Shader& shader) const
{
    // Add to CM uniform block
    if (!_cmUniforms.empty())
    {
        ShaderStage& stage = shader.getStage(Stage::PIXEL);
        stage.createUniformBlock(ColorManagementSystem::COLOR_MANAGEMENT_UNIFORMS);
        VariableBlock& uniformBlock = stage.getUniformBlock(ColorManagementSystem::COLOR_MANAGEMENT_UNIFORMS);

        for (size_t i = 0; i < _cmUniforms.size(); ++i)
        {
            ShaderPort* u = const_cast<ShaderPort*>(_cmUniforms[i]);
            uniformBlock.add(u->getSelf());
        }
    }
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
