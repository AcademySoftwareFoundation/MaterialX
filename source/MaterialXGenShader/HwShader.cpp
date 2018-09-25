#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

const string HwShader::LIGHT_DATA_BLOCK = "LightData";

HwShader::HwShader(const string& name) 
    : ParentClass(name)
    , _vertexData("VertexData", "vd")
    , _transparencyMethod(TRANSPARENCY_NONE)
{
    _stages.push_back(Stage("Vertex"));

    // Create default uniform blocks for vertex stage
    createUniformBlock(VERTEX_STAGE, PRIVATE_UNIFORMS, "prv");
    createUniformBlock(VERTEX_STAGE, PUBLIC_UNIFORMS, "pub");

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
    createUniform(PIXEL_STAGE, PRIVATE_UNIFORMS, Type::MATRIX44, "u_envMatrix", 
        EMPTY_STRING, Value::createValue<Matrix44>(yRotationPI));
    createUniform(PIXEL_STAGE, PRIVATE_UNIFORMS, Type::FILENAME, "u_envSpecular");
    createUniform(PIXEL_STAGE, PRIVATE_UNIFORMS, Type::FILENAME, "u_envIrradiance");
}

void HwShader::initialize(ElementPtr element, ShaderGenerator& shadergen, const SgOptions& options)
{
    ParentClass::initialize(element, shadergen, options);

    HwShaderGenerator& sg = static_cast<HwShaderGenerator&>(shadergen);

    // Find out if transparency should be used
    _transparencyMethod = options.hwTransparencyMethod;

    //
    // For image textures we need to convert filenames into uniforms (texture samplers).
    // Any unconnected filename input on file texture nodes needs to have a corresponding 
    // uniform.
    //

    // Start with top level graphs.
    std::deque<SgNodeGraph*> graphQueue;
    getTopLevelShaderGraphs(shadergen, graphQueue);

    Syntax::UniqueNameMap uniqueNames;

    while (!graphQueue.empty())
    {
        SgNodeGraph* graph = graphQueue.back();
        graphQueue.pop_back();

        for (SgNode* node : graph->getNodes())
        {
            if (node->hasClassification(SgNode::Classification::FILETEXTURE))
            {
                for (SgInput* input : node->getInputs())
                {
                    if (!input->connection && input->type == Type::FILENAME)
                    {
                        // Create the uniform and assing the name of the uniform to
                        // the input so we can reference it during code generation.
                        // Using the filename type will make this uniform into a texture sampler.
                        string name = node->getName() + "_" + input->name;
                        shadergen.getSyntax()->makeUnique(name, uniqueNames);
                        createUniform(HwShader::PIXEL_STAGE, PUBLIC_UNIFORMS, Type::FILENAME, name, EMPTY_STRING, input->value);
                        input->value = Value::createValue(std::string(name));
                    }
                }
            }
            // Push subgraphs on the queue to process these as well.
            SgNodeGraph* subgraph = node->getImplementation()->getNodeGraph();
            if (subgraph)
            {
                graphQueue.push_back(subgraph);
            }
        }
    }

    // For surface shaders we need light shaders
    if (_rootGraph->hasClassification(SgNode::Classification::SHADER | SgNode::Classification::SURFACE))
    {
        // Create variables for all bound light shaders
        for (auto lightShader : sg.getBoundLightShaders())
        {
            lightShader.second->createVariables(*SgNode::NONE, shadergen, *this);
        }
    }
}

void HwShader::createVertexData(const TypeDesc* type, const string& name, const string& semantic)
{
    if (_vertexData.variableMap.find(name) == _vertexData.variableMap.end())
    {
        VariablePtr variable = std::make_shared<Variable>(type, name, semantic);
        _vertexData.variableMap[name] = variable;
        _vertexData.variableOrder.push_back(variable.get());
    }
}

void HwShader::getTopLevelShaderGraphs(ShaderGenerator& shadergen, std::deque<SgNodeGraph*>& graphs) const
{
    // Get graphs for base class
    ParentClass::getTopLevelShaderGraphs(shadergen, graphs);

    // Get light shader graphs
    HwShaderGenerator& sg = static_cast<HwShaderGenerator&>(shadergen);
    for (auto lightShader : sg.getBoundLightShaders())
    {
        SgNodeGraph* lightGraph = lightShader.second->getNodeGraph();
        if (lightGraph)
        {
            graphs.push_back(lightGraph);
        }
    }
}

}
