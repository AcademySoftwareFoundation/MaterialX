#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

const string HwShader::VERTEX_STAGE = "vertex";
const string HwShader::LIGHT_DATA_BLOCK = "LightData";

HwShader::HwShader(const string& name) 
    : ParentClass(name)
    , _vertexData("VertexData", "vd")
    , _transparency(false)
{
    // Create the vertex stage
    createStage(VERTEX_STAGE);

    // Create default uniform blocks for vertex stage
    createUniformBlock(VERTEX_STAGE, PRIVATE_UNIFORMS, "prvUniform");
    createUniformBlock(VERTEX_STAGE, PUBLIC_UNIFORMS, "pubUniform");

    // Create light data uniform block with the required field for light type
    createUniformBlock(PIXEL_STAGE, LIGHT_DATA_BLOCK, "u_lightData");
    createUniform(PIXEL_STAGE, LIGHT_DATA_BLOCK, Type::INTEGER, "type");

    // Create uniforms for environment lighting
    // Note: Generation of the rotation matrix using floating point math can result
    // in values which when output can't be consumed by a h/w shader, so
    // just setting explicit values here for now since the matrix is simple.
    // In general the values will need to be "sanitized" for hardware.
    const Matrix44 yRotationPI(-1, 0, 0, 0,
                                0, 1, 0, 0,
                                0, 0, -1, 0,
                                0, 0, 0, 1);
    createUniform(PIXEL_STAGE, PRIVATE_UNIFORMS, Type::MATRIX44, "u_envMatrix", EMPTY_STRING, 
        EMPTY_STRING, Value::createValue<Matrix44>(yRotationPI));
    createUniform(PIXEL_STAGE, PRIVATE_UNIFORMS, Type::FILENAME, "u_envSpecular");
    createUniform(PIXEL_STAGE, PRIVATE_UNIFORMS, Type::FILENAME, "u_envIrradiance");
}

void HwShader::initialize(ElementPtr element, ShaderGenerator& shadergen, const GenOptions& options)
{
    ParentClass::initialize(element, shadergen, options);

    HwShaderGenerator& sg = static_cast<HwShaderGenerator&>(shadergen);

    // Find out if transparency should be used
    _transparency = options.hwTransparency;

    //
    // For image textures we need to convert filenames into uniforms (texture samplers).
    // Any unconnected filename input on file texture nodes needs to have a corresponding 
    // uniform.
    //

    // Start with top level graphs.
    std::deque<ShaderGraph*> graphQueue;
    getTopLevelShaderGraphs(shadergen, graphQueue);

    while (!graphQueue.empty())
    {
        ShaderGraph* graph = graphQueue.back();
        graphQueue.pop_back();

        for (ShaderNode* node : graph->getNodes())
        {
            if (node->hasClassification(ShaderNode::Classification::FILETEXTURE))
            {
                for (ShaderInput* input : node->getInputs())
                {
                    if (!input->connection && input->type == Type::FILENAME)
                    {
                        // Create the uniform using the filename type to make this uniform into a texture sampler.
                        createUniform(HwShader::PIXEL_STAGE, PUBLIC_UNIFORMS, Type::FILENAME, input->variable, input->path, EMPTY_STRING, input->value);

                        // Assing the uniform name to the input value
                        // so we can reference it duing code generation.
                        input->value = Value::createValue(input->variable);
                    }
                }
            }
            // Push subgraphs on the queue to process these as well.
            ShaderGraph* subgraph = node->getImplementation()->getGraph();
            if (subgraph)
            {
                graphQueue.push_back(subgraph);
            }
        }
    }

    // For surface shaders we need light shaders
    if (_rootGraph->hasClassification(ShaderNode::Classification::SHADER | ShaderNode::Classification::SURFACE))
    {
        // Create variables for all bound light shaders
        for (auto lightShader : sg.getBoundLightShaders())
        {
            lightShader.second->createVariables(*ShaderNode::NONE, shadergen, *this);
        }
    }
}

void HwShader::createVertexData(const TypeDesc* type, const string& name, const string& semantic)
{
    if (_vertexData.variableMap.find(name) == _vertexData.variableMap.end())
    {
        VariablePtr variable = Variable::create(type, name, EMPTY_STRING, semantic, nullptr);
        _vertexData.variableMap[name] = variable;
        _vertexData.variableOrder.push_back(variable.get());
    }
}

void HwShader::getTopLevelShaderGraphs(ShaderGenerator& shadergen, std::deque<ShaderGraph*>& graphs) const
{
    // Get graphs for base class
    ParentClass::getTopLevelShaderGraphs(shadergen, graphs);

    // Get light shader graphs
    HwShaderGenerator& sg = static_cast<HwShaderGenerator&>(shadergen);
    for (auto lightShader : sg.getBoundLightShaders())
    {
        ShaderGraph* lightGraph = lightShader.second->getGraph();
        if (lightGraph)
        {
            graphs.push_back(lightGraph);
        }
    }
}

}
