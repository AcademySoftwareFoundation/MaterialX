//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <MaterialXGenMdl/MdlSyntax.h>
#include <MaterialXGenMdl/Nodes/CompoundNodeMdl.h>
#include <MaterialXGenMdl/Nodes/SourceCodeNodeMdl.h>
#include <MaterialXGenMdl/Nodes/MaterialNodeMdl.h>
#include <MaterialXGenMdl/Nodes/SurfaceNodeMdl.h>
#include <MaterialXGenMdl/Nodes/HeightToNormalNodeMdl.h>
#include <MaterialXGenMdl/Nodes/BlurNodeMdl.h>
#include <MaterialXGenMdl/Nodes/ClosureLayerNodeMdl.h>
#include <MaterialXGenMdl/Nodes/ClosureCompoundNodeMdl.h>
#include <MaterialXGenMdl/Nodes/ClosureSourceCodeNodeMdl.h>
#include <MaterialXGenMdl/Nodes/CustomNodeMdl.h>
#include <MaterialXGenMdl/Nodes/ImageNodeMdl.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Util.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

const vector<string> DEFAULT_IMPORTS =
{
    "import ::df::*",
    "import ::base::*",
    "import ::math::*",
    "import ::state::*",
    "import ::anno::*",
    "import ::tex::*",
    "using ::materialx::core import *",
    "using ::materialx::sampling import *",
};

const vector<string> DEFAULT_VERSIONED_IMPORTS =
{
    "using ::materialx::stdlib_",
    "using ::materialx::pbrlib_",
};

const string IMPORT_ALL = " import *";

const string MDL_VERSION_1_6 = "1.6";
const string MDL_VERSION_1_7 = "1.7";
const string MDL_VERSION_1_8 = "1.8";
const string MDL_VERSION_1_9 = "1.9";
const string MDL_VERSION_1_10 = "1.10";
const string MDL_VERSION_SUFFIX_1_6 = "1_6";
const string MDL_VERSION_SUFFIX_1_7 = "1_7";
const string MDL_VERSION_SUFFIX_1_8 = "1_8";
const string MDL_VERSION_SUFFIX_1_9 = "1_9";
const string MDL_VERSION_SUFFIX_1_10 = "1_10";

} // anonymous namespace

const string MdlShaderGenerator::TARGET = "genmdl";
const string GenMdlOptions::GEN_CONTEXT_USER_DATA_KEY = "genmdloptions";

const std::unordered_map<string, string> MdlShaderGenerator::GEOMPROP_DEFINITIONS =
{
    { "Pobject", "state::transform_point(state::coordinate_internal, state::coordinate_object, state::position())" },
    { "Pworld", "state::transform_point(state::coordinate_internal, state::coordinate_world, state::position())" },
    { "Nobject", "state::transform_normal(state::coordinate_internal, state::coordinate_object, state::normal())" },
    { "Nworld", "state::transform_normal(state::coordinate_internal, state::coordinate_world, state::normal())" },
    { "Tobject", "state::transform_vector(state::coordinate_internal, state::coordinate_object, state::texture_tangent_u(0))" },
    { "Tworld", "state::transform_vector(state::coordinate_internal, state::coordinate_world, state::texture_tangent_u(0))" },
    { "Bobject", "state::transform_vector(state::coordinate_internal, state::coordinate_object, state::texture_tangent_v(0))" },
    { "Bworld", "state::transform_vector(state::coordinate_internal, state::coordinate_world, state::texture_tangent_v(0))" },
    { "UV0", "float2(state::texture_coordinate(0).x, state::texture_coordinate(0).y)" },
    { "Vworld", "state::direction()" }
};

//
// MdlShaderGenerator methods
//

MdlShaderGenerator::MdlShaderGenerator(TypeSystemPtr typeSystem) :
    ShaderGenerator(typeSystem, MdlSyntax::create(typeSystem))
{
    // Register custom types to handle enumeration output
    _typeSystem->registerType(Type::MDL_COORDINATESPACE);
    _typeSystem->registerType(Type::MDL_ADDRESSMODE);
    _typeSystem->registerType(Type::MDL_FILTERLOOKUPMODE);
    _typeSystem->registerType(Type::MDL_FILTERTYPE);
    _typeSystem->registerType(Type::MDL_DISTRIBUTIONTYPE);
    _typeSystem->registerType(Type::MDL_SCATTER_MODE);

    // Register build-in implementations

    // <!-- <surfacematerial> -->
    registerImplementation("IM_surfacematerial_" + MdlShaderGenerator::TARGET, MaterialNodeMdl::create);

    // <!-- <surface> -->
    registerImplementation("IM_surface_" + MdlShaderGenerator::TARGET, SurfaceNodeMdl::create);

    // <!-- <blur> -->
    registerImplementation("IM_blur_float_" + MdlShaderGenerator::TARGET, BlurNodeMdl::create);
    registerImplementation("IM_blur_color3_" + MdlShaderGenerator::TARGET, BlurNodeMdl::create);
    registerImplementation("IM_blur_color4_" + MdlShaderGenerator::TARGET, BlurNodeMdl::create);
    registerImplementation("IM_blur_vector2_" + MdlShaderGenerator::TARGET, BlurNodeMdl::create);
    registerImplementation("IM_blur_vector3_" + MdlShaderGenerator::TARGET, BlurNodeMdl::create);
    registerImplementation("IM_blur_vector4_" + MdlShaderGenerator::TARGET, BlurNodeMdl::create);

    // <!-- <heighttonormal> -->
    registerImplementation("IM_heighttonormal_vector3_" + MdlShaderGenerator::TARGET, HeightToNormalNodeMdl::create);

    // <!-- <layer> -->
    registerImplementation("IM_layer_bsdf_" + MdlShaderGenerator::TARGET, ClosureLayerNodeMdl::create);
    registerImplementation("IM_layer_vdf_" + MdlShaderGenerator::TARGET, ClosureLayerNodeMdl::create);

    // <!-- <dielectric_bsdf> -->
    registerImplementation("IM_dielectric_bsdf_" + MdlShaderGenerator::TARGET, LayerableNodeMdl::create);

    // <!-- <generalized_schlick_bsdf> -->
    registerImplementation("IM_generalized_schlick_bsdf_" + MdlShaderGenerator::TARGET, LayerableNodeMdl::create);

    // <!-- <sheen_bsdf> -->
    registerImplementation("IM_sheen_bsdf_" + MdlShaderGenerator::TARGET, LayerableNodeMdl::create);

    // <!-- <image> -->
    registerImplementation("IM_image_float_" + MdlShaderGenerator::TARGET, ImageNodeMdl::create);
    registerImplementation("IM_image_color3_" + MdlShaderGenerator::TARGET, ImageNodeMdl::create);
    registerImplementation("IM_image_color4_" + MdlShaderGenerator::TARGET, ImageNodeMdl::create);
    registerImplementation("IM_image_vector2_" + MdlShaderGenerator::TARGET, ImageNodeMdl::create);
    registerImplementation("IM_image_vector3_" + MdlShaderGenerator::TARGET, ImageNodeMdl::create);
    registerImplementation("IM_image_vector4_" + MdlShaderGenerator::TARGET, ImageNodeMdl::create);
}

ShaderPtr MdlShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    // For MDL we cannot cache node implementations between generation calls,
    // because this generator needs to do edits to subgraphs implementations
    // depending on the context in which a node is used.
    context.clearNodeImplementations();

    ShaderPtr shader = createShader(name, element, context);

    // Request fixed floating-point notation for consistency across targets.
    ScopedFloatFormatting fmt(Value::FloatFormatFixed);

    ShaderGraph& graph = shader->getGraph();
    ShaderStage& stage = shader->getStage(Stage::PIXEL);

    // Emit version
    emitMdlVersionNumber(context, stage);
    emitLineBreak(stage);

    // Emit module imports
    for (const string& module : DEFAULT_IMPORTS)
    {
        emitLine(module, stage);
    }
    for (const string& module : DEFAULT_VERSIONED_IMPORTS)
    {
        emitString(module, stage);
        emitMdlVersionFilenameSuffix(context, stage);
        emitString(IMPORT_ALL, stage);
        emitLineEnd(stage, true);
    }

    // Emit custom node imports for nodes in the graph
    for (ShaderNode* node : graph.getNodes())
    {
        const ShaderNodeImpl& impl = node->getImplementation();
        const CustomCodeNodeMdl* customNode = dynamic_cast<const CustomCodeNodeMdl*>(&impl);
        if (customNode)
        {
            const string& importName = customNode->getQualifiedModuleName();
            if (!importName.empty())
            {
                emitString("import ", stage);
                emitString(importName, stage);
                emitString("::*", stage);
                emitLineEnd(stage, true);
            }
        }
    }

    // Add global constants and type definitions
    emitTypeDefinitions(context, stage);

    // Emit function definitions for all nodes
    emitFunctionDefinitions(graph, context, stage);

    // Emit shader type, determined from the first
    // output if there are multiple outputs.
    const ShaderGraphOutputSocket* outputSocket = graph.getOutputSocket(0);
    emitString("export material ", stage);

    // Begin shader signature. Note that makeIdentifier() will sanitize the name.
    string functionName = shader->getName();
    _syntax->makeIdentifier(functionName, graph.getIdentifierMap());
    setFunctionName(functionName, stage);
    emitLine(functionName, stage, false);
    emitScopeBegin(stage, Syntax::PARENTHESES);

    // Emit shader inputs
    emitShaderInputs(element->getDocument(), stage.getInputBlock(MDL::INPUTS), stage);

    // End shader signature
    emitScopeEnd(stage);

    // Begin shader body
    emitLine("= let", stage, false);
    emitScopeBegin(stage);

    // Emit constants
    const VariableBlock& constants = stage.getConstantBlock();
    if (constants.size())
    {
        emitVariableDeclarations(constants, _syntax->getConstantQualifier(), Syntax::SEMICOLON, context, stage);
        emitLineBreak(stage);
    }

    // Emit shader inputs that have been filtered during printing of the public interface
    const string uniformPrefix = _syntax->getUniformQualifier() + " ";
    for (ShaderGraphInputSocket* inputSocket : graph.getInputSockets())
    {
        if (inputSocket->getConnections().size() &&
            (inputSocket->getType().getSemantic() == TypeDesc::SEMANTIC_SHADER ||
             inputSocket->getType().getSemantic() == TypeDesc::SEMANTIC_CLOSURE ||
             inputSocket->getType().getSemantic() == TypeDesc::SEMANTIC_MATERIAL))
        {
            const string& qualifier = inputSocket->isUniform() || inputSocket->getType() == Type::FILENAME 
                ? uniformPrefix 
                : EMPTY_STRING;
            const string& type = _syntax->getTypeName(inputSocket->getType());

            emitLineBegin(stage);
            emitString(qualifier + type + " " + inputSocket->getVariable() + " = ", stage);
            emitString(_syntax->getDefaultValue(inputSocket->getType(), true), stage);
            emitLineEnd(stage, true);
        }
    }

    // Emit all texturing nodes. These are inputs to any
    // closure/shader nodes and need to be emitted first.
    emitFunctionCalls(graph, context, stage, ShaderNode::Classification::TEXTURE);

    // Emit function calls for "root" closure/shader nodes.
    // These will internally emit function calls for any dependent closure nodes upstream.
    for (ShaderGraphOutputSocket* socket : graph.getOutputSockets())
    {
        if (socket->getConnection())
        {
            const ShaderNode* upstream = socket->getConnection()->getNode();
            if (upstream->getParent() == &graph &&
                (upstream->hasClassification(ShaderNode::Classification::CLOSURE) ||
                 upstream->hasClassification(ShaderNode::Classification::SHADER)))
            {
                emitFunctionCall(*upstream, context, stage);
            }
        }
    }

    // Get final result
    const string result = getUpstreamResult(outputSocket, context);

    const TypeDesc outputType = outputSocket->getType();
    // Try to return some meaningful color in case the output is not a material
    if (graph.hasClassification(ShaderNode::Classification::TEXTURE))
    {
        if (outputType == Type::DISPLACEMENTSHADER)
        {
            emitLine("float3 displacement__ = " + result + ".geometry.displacement", stage);
            emitLine("color finalOutput__ = mk_color3("
                     "r: math::dot(displacement__, state::texture_tangent_u(0)),"
                     "g: math::dot(displacement__, state::texture_tangent_v(0)),"
                     "b: math::dot(displacement__, state::normal()))",
                     stage);
        }
        else
        {
            emitLine("float3 displacement__ = float3(0.0)", stage);
            std::string finalOutput = "mk_color3(0.0)";
            if (outputType == Type::BOOLEAN)
                finalOutput = result + " ? mk_color3(0.0, 1.0, 0.0) : mk_color3(1.0, 0.0, 0.0)";
            else if (outputType == Type::INTEGER)
                finalOutput = "mk_color3(" + result + " / 100)"; // arbitrary
            else if (outputType == Type::FLOAT)
                finalOutput = "mk_color3(" + result + ")";
            else if (outputType == Type::VECTOR2)
                finalOutput = "mk_color3(" + result + ".x, " + result + ".y, 0.0)";
            else if (outputType == Type::VECTOR3)
                finalOutput = "mk_color3(" + result + ")";
            else if (outputType == Type::VECTOR4)
                finalOutput = "mk_color3(" + result + ".x, " + result + ".y, " + result + ".z)";
            else if (outputType == Type::COLOR3)
                finalOutput = result;
            else if (outputType == Type::COLOR4)
                finalOutput = result + ".rgb";
            else if (outputType == Type::MATRIX33 || outputType == Type::MATRIX44)
                finalOutput = "mk_color3(" + result + "[0][0], " + result + "[1][1], " + result + "[2][2])";

            emitLine("color finalOutput__ = " + finalOutput, stage);
        }

        // End shader body
        emitScopeEnd(stage);

        static const string textureMaterial =
            "in material\n"
            "(\n"
            "    surface: material_surface(\n"
            "        emission : material_emission(\n"
            "            emission : df::diffuse_edf(),\n"
            "            intensity : finalOutput__ * math::PI,\n"
            "            mode : intensity_radiant_exitance\n"
            "        )\n"
            "    ),\n"
            "    geometry: material_geometry(\n"
            "       displacement : displacement__\n"
            "    )\n"
            ");";
        emitBlock(textureMaterial, FilePath(), context, stage);
    }
    else
    {
        emitLine(_syntax->getTypeSyntax(outputType).getName() + " finalOutput__ = " + result, stage);

        // End shader body
        emitScopeEnd(stage);

        static const string shaderMaterial = "in material(finalOutput__);";
        emitBlock(shaderMaterial, FilePath(), context, stage);
    }

    // Perform token substitution
    replaceTokens(_tokenSubstitutions, stage);

    return shader;
}

ShaderNodeImplPtr MdlShaderGenerator::getImplementation(const NodeDef& nodedef, GenContext& context) const
{
    InterfaceElementPtr implElement = nodedef.getImplementation(getTarget());
    if (!implElement)
    {
        return nullptr;
    }

    const string& name = implElement->getName();

    // Check if it's created and cached already.
    ShaderNodeImplPtr impl = context.findNodeImplementation(name);
    if (impl)
    {
        return impl;
    }

    vector<OutputPtr> outputs = nodedef.getActiveOutputs();
    if (outputs.empty())
    {
        throw ExceptionShaderGenError("NodeDef '" + nodedef.getName() + "' has no outputs defined");
    }

    const TypeDesc outputType = _typeSystem->getType(outputs[0]->getType());

    if (implElement->isA<NodeGraph>())
    {
        // Use a compound implementation.
        if (outputType.isClosure())
        {
            impl = ClosureCompoundNodeMdl::create();
        }
        else
        {
            impl = CompoundNodeMdl::create();
        }
    }
    else if (implElement->isA<Implementation>())
    {
        // Try creating a new in the factory.
        impl = _implFactory.create(name);
        if (!impl)
        {
            // When `file` and `function` are provided we consider this node a user node
            const string file = implElement->getTypedAttribute<string>("file");
            const string function = implElement->getTypedAttribute<string>("function");
            // Or, if `sourcecode` is provided we consider this node a user node with inline implementation
            // inline implementations are not supposed to have replacement markers
            const string sourcecode = implElement->getTypedAttribute<string>("sourcecode");
            if ((!file.empty() && !function.empty()) || (!sourcecode.empty() && sourcecode.find("{{") == string::npos))
            {
                impl = CustomCodeNodeMdl::create();
            }
            else if (file.empty() && sourcecode.empty())
            {
                throw ExceptionShaderGenError("No valid MDL implementation found for '" + name + "'");
            }
            else
            {
                // Fall back to source code implementation.
                if (outputType.isClosure())
                {
                    impl = ClosureSourceCodeNodeMdl::create();
                }
                else
                {
                    impl = SourceCodeNodeMdl::create();
                }
            }
        }
    }
    if (!impl)
    {
        return nullptr;
    }

    impl->initialize(*implElement, context);

    // Cache it.
    context.addNodeImplementation(name, impl);

    return impl;
}

string MdlShaderGenerator::getUpstreamResult(const ShaderInput* input, GenContext& context) const
{
    const ShaderOutput* upstreamOutput = input->getConnection();

    if (!upstreamOutput || upstreamOutput->getNode()->isAGraph())
    {
        return ShaderGenerator::getUpstreamResult(input, context);
    }

    const MdlSyntax& mdlSyntax = static_cast<const MdlSyntax&>(getSyntax());
    string variable;
    const ShaderNode* upstreamNode = upstreamOutput->getNode();
    if (!upstreamNode->isAGraph() && upstreamNode->numOutputs() > 1)
    {
        const CompoundNodeMdl* upstreamNodeMdl = dynamic_cast<const CompoundNodeMdl*>(&upstreamNode->getImplementation());
        if (upstreamNodeMdl && upstreamNodeMdl->unrollReturnStructMembers())
        {
            variable = upstreamNode->getName() + "__" + upstreamOutput->getName();
        }
        else
        {
            const string& fieldName = upstreamOutput->getName();
            const CustomCodeNodeMdl* upstreamCustomNodeMdl = dynamic_cast<const CustomCodeNodeMdl*>(&upstreamNode->getImplementation());
            if (upstreamCustomNodeMdl)
            {
                // Prefix the port name depending on the CustomCodeNode
                variable = upstreamNode->getName() + "_result." + upstreamCustomNodeMdl->modifyPortName(fieldName, mdlSyntax);
            }
            else
            {
                // Existing implementations and none user defined structs will keep the prefix always to not break existing content
                variable = upstreamNode->getName() + "_result." + mdlSyntax.modifyPortName(upstreamOutput->getName());
            }
        }
    }
    else
    {
        variable = upstreamOutput->getVariable();
    }

    // Look for any additional suffix to append
    string suffix;
    context.getInputSuffix(input, suffix);
    if (!suffix.empty())
    {
        variable += suffix;
    }

    return variable;
}

namespace
{

// [TODO]
// Here we assume this bit of the port flags is unused.
// Change this to a more general and safe solution.
class ShaderPortFlagMdl
{
  public:
    static const uint32_t TRANSMISSION_IOR_DEPENDENCY = 1u << 31;
};

// Check if a graph has inputs with dependencies on transmission IOR on the inside.
// Track all subgraphs found that has such a dependency, as well as subgraphs that are
// found to have a varying connection to transmission IOR.
// Returns true if uniform ior dependencies are found.
bool checkTransmissionIorDependencies(ShaderGraph* g, std::set<ShaderGraph*>& graphsWithIorDependency, std::set<ShaderGraph*>& graphsWithIorVarying)
{
    bool result = false;
    for (ShaderNode* node : g->getNodes())
    {
        ShaderGraph* subgraph = node->getImplementation().getGraph();
        if (subgraph)
        {
            // Check recursively if this subgraph has IOR dependencies.
            if (checkTransmissionIorDependencies(subgraph, graphsWithIorDependency, graphsWithIorVarying))
            {
                for (ShaderOutput* socket : subgraph->getInputSockets())
                {
                    if (socket->getFlag(ShaderPortFlagMdl::TRANSMISSION_IOR_DEPENDENCY))
                    {
                        ShaderInput* input = node->getInput(socket->getName());
                        ShaderOutput* source = input ? input->getConnection() : nullptr;
                        if (source)
                        {
                            // Check if this is a graph interface connection.
                            if (source->getNode() == g)
                            {
                                graphsWithIorDependency.insert(g);
                                source->setFlag(ShaderPortFlagMdl::TRANSMISSION_IOR_DEPENDENCY, true);
                                result = true;
                            }
                            else if (source->getNode()->hasClassification(ShaderNode::Classification::CONSTANT))
                            {
                                // If the connection is to a constant node we can
                                // handled that here since it's just a uniform value.
                                ShaderInput* value = source->getNode()->getInput(ValueElement::VALUE_ATTRIBUTE);
                                if (value && value->getValue())
                                {
                                    input->setValue(value->getValue());
                                }
                                input->breakConnection();
                            }
                            else
                            {
                                // If we get here we have to assume this is a varying connection.
                                // Save the graph as a varying graph so we later can break its
                                // internal connections to transmission IOR.
                                graphsWithIorVarying.insert(subgraph);
                                return false; // no need to continue with this subgraph
                            }
                        }
                    }
                }
            }
        }
        else
        {
            // Check for transmission BSDF node.
            if (node->hasClassification(ShaderNode::Classification::BSDF_T))
            {
                // Check if IOR is connected.
                ShaderInput* ior = node->getInput("ior");
                ShaderOutput* source = ior ? ior->getConnection() : nullptr;
                if (source)
                {
                    // Check if this is a graph interface connection.
                    if (source->getNode() == g)
                    {
                        graphsWithIorDependency.insert(g);
                        source->setFlag(ShaderPortFlagMdl::TRANSMISSION_IOR_DEPENDENCY, true);
                        result = true;
                    }
                    else if (source->getNode()->hasClassification(ShaderNode::Classification::CONSTANT))
                    {
                        // If the connection is to a constant node we can
                        // handled that here since it's just a uniform value.
                        ShaderInput* value = source->getNode()->getInput(ValueElement::VALUE_ATTRIBUTE);
                        if (value && value->getValue())
                        {
                            ior->setValue(value->getValue());
                        }
                        ior->breakConnection();
                    }
                    else
                    {
                        // If we get here we have to assume this is a varying connection
                        // and we can break it immediately here.
                        ior->breakConnection();
                    }
                }
            }
        }
    }
    return result;
}

// Disconnect any incoming connections to transmission IOR
// inside a graph.
void disconnectTransmissionIor(ShaderGraph* g)
{
    for (ShaderNode* node : g->getNodes())
    {
        ShaderGraph* subgraph = node->getImplementation().getGraph();
        if (subgraph && (subgraph->hasClassification(ShaderNode::Classification::SHADER) ||
                         subgraph->hasClassification(ShaderNode::Classification::CLOSURE)))
        {
            disconnectTransmissionIor(subgraph);
        }
        else if (node->hasClassification(ShaderNode::Classification::BSDF_T))
        {
            ShaderInput* ior = node->getInput("ior");
            if (ior)
            {
                ior->breakConnection();
            }
        }
    }
}

} // anonymous namespace

ShaderPtr MdlShaderGenerator::createShader(const string& name, ElementPtr element, GenContext& context) const
{
    // Create the root shader graph
    ShaderGraphPtr graph = ShaderGraph::create(nullptr, name, element, context);
    ShaderPtr shader = std::make_shared<Shader>(name, graph);

    // Create our stage.
    ShaderStagePtr stage = createStage(Stage::PIXEL, *shader);
    VariableBlockPtr inputs = stage->createInputBlock(MDL::INPUTS);
    VariableBlockPtr outputs = stage->createOutputBlock(MDL::OUTPUTS);

    // Create shader variables for all nodes that need this.
    createVariables(graph, context, *shader);

    // Create inputs for the published graph interface.
    for (ShaderGraphInputSocket* inputSocket : graph->getInputSockets())
    {
        // Only for inputs that are connected/used internally,
        // and are editable by users.
        if (inputSocket->getConnections().size() && graph->isEditable(*inputSocket))
        {
            if (inputSocket->getType().getSemantic() == TypeDesc::SEMANTIC_SHADER ||
                inputSocket->getType().getSemantic() == TypeDesc::SEMANTIC_CLOSURE ||
                inputSocket->getType().getSemantic() == TypeDesc::SEMANTIC_MATERIAL)
            {
                continue;
            }

            inputs->add(inputSocket->getSelf());
        }
    }

    // Create outputs from the graph interface.
    for (ShaderGraphOutputSocket* outputSocket : graph->getOutputSockets())
    {
        outputs->add(outputSocket->getSelf());
    }

    // MDL does not allow varying data connected to transmission IOR until MDL 1.9.
    // We must find all uses of transmission IOR and make sure we don't
    // have a varying connection to it. If a varying connection is found
    // we break that connection and revert to using default value on that
    // instance of IOR, so that other uses of the same varying input still
    // works in other places.
    // As a result if a varying connections is set on transmission IOR
    // it just reverts to default value. Varying data on transmission IOR
    // is very rare so this is normally not a problem in practice.
    // One use-case where this fix is important is for shading models with
    // a single IOR input, that gets connected to both reflection and
    // transmission IOR inside the shading model graph. For such cases
    // this fix will disconnect the transmission IOR on the inside, but
    // still support the connection to reflection IOR.
    //
    GenMdlOptions::MdlVersion version = getMdlVersion(context);
    bool uniformIorRequired =
        version == GenMdlOptions::MdlVersion::MDL_1_6 ||
        version == GenMdlOptions::MdlVersion::MDL_1_7 ||
        version == GenMdlOptions::MdlVersion::MDL_1_8;
    if (uniformIorRequired && (
        graph->hasClassification(ShaderNode::Classification::SHADER) ||
        graph->hasClassification(ShaderNode::Classification::CLOSURE)))
    {
        // Find dependencies on transmission IOR.
        std::set<ShaderGraph*> graphsWithIorDependency;
        std::set<ShaderGraph*> graphsWithIorVarying;
        checkTransmissionIorDependencies(graph.get(), graphsWithIorDependency, graphsWithIorVarying);

        // For any graphs found that has a varying connection
        // to transmission IOR we need to break that connection.
        for (ShaderGraph* g : graphsWithIorVarying)
        {
            disconnectTransmissionIor(g);
            graphsWithIorDependency.erase(g);
        }

        // For graphs that has a dependency with transmission IOR on the inside,
        // we can declare the corresponding inputs as being uniform and preserve
        // the internal connection to transmission IOR.
        for (ShaderGraph* g : graphsWithIorDependency)
        {
            for (ShaderOutput* socket : g->getInputSockets())
            {
                if (socket->getFlag(ShaderPortFlagMdl::TRANSMISSION_IOR_DEPENDENCY))
                {
                    socket->setUniform();
                }
            }
        }
    }

    return shader;
}

namespace
{

void emitInputAnnotations(const MdlShaderGenerator& _this, ConstDocumentPtr doc, const ShaderPort* variable, ShaderStage& stage)
{
    // allows to relate between MaterialX and MDL parameters when looking at the MDL code.
    const std::string mtlxParameterPathAnno = "materialx::core::origin(\"" + variable->getPath() + "\")";

    _this.emitLineEnd(stage, false);
    _this.emitLine("[[", stage, false);
    _this.emitLine("\t" + mtlxParameterPathAnno, stage, false);
    _this.emitLineBegin(stage);
    _this.emitString("]]", stage); // line ending follows by caller
}

} // anonymous namespace

void MdlShaderGenerator::emitShaderInputs(ConstDocumentPtr doc, const VariableBlock& inputs, ShaderStage& stage) const
{
    const string uniformPrefix = _syntax->getUniformQualifier() + " ";
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        const ShaderPort* input = inputs[i];

        const string& qualifier = input->isUniform() || input->getType() == Type::FILENAME ? uniformPrefix : EMPTY_STRING;
        const string& type = _syntax->getTypeName(input->getType());

        string value = input->getValue() ? _syntax->getValue(input->getType(), *input->getValue(), true) : EMPTY_STRING;
        const string& geomprop = input->getGeomProp();
        if (!geomprop.empty())
        {
            auto it = GEOMPROP_DEFINITIONS.find(geomprop);
            if (it != GEOMPROP_DEFINITIONS.end())
            {
                value = it->second;
            }
        }
        if (value.empty())
        {
            value = _syntax->getDefaultValue(input->getType(), true);
        }

        emitLineBegin(stage);
        emitString(qualifier + type + " " + input->getVariable() + " = " + value, stage);
        emitInputAnnotations(*this, doc, input, stage);

        if (i < inputs.size() - 1)
        {
            emitString(",", stage);
        }

        emitLineEnd(stage, false);
    }
}

GenMdlOptions::MdlVersion MdlShaderGenerator::getMdlVersion(GenContext& context) const
{
    GenMdlOptionsPtr options = context.getUserData<GenMdlOptions>(GenMdlOptions::GEN_CONTEXT_USER_DATA_KEY);
    return options ? options->targetVersion : GenMdlOptions::MdlVersion::MDL_LATEST;
}

void MdlShaderGenerator::emitMdlVersionNumber(GenContext& context, ShaderStage& stage) const
{
    GenMdlOptions::MdlVersion version = getMdlVersion(context);

    emitLineBegin(stage);
    emitString("mdl ", stage);
    switch (version)
    {
        case GenMdlOptions::MdlVersion::MDL_1_6:
            emitString(MDL_VERSION_1_6, stage);
            break;
        case GenMdlOptions::MdlVersion::MDL_1_7:
            emitString(MDL_VERSION_1_7, stage);
            break;
        case GenMdlOptions::MdlVersion::MDL_1_8:
            emitString(MDL_VERSION_1_8, stage);
            break;
        case GenMdlOptions::MdlVersion::MDL_1_9:
            emitString(MDL_VERSION_1_9, stage);
            break;
        default:
            // GenMdlOptions::MdlVersion::MDL_1_10
            // GenMdlOptions::MdlVersion::MDL_LATEST
            emitString(MDL_VERSION_1_10, stage);
            break;
    }
    emitLineEnd(stage, true);
}


const string& MdlShaderGenerator::getMdlVersionFilenameSuffix(GenContext& context) const
{
    GenMdlOptions::MdlVersion version = getMdlVersion(context);

    switch (version)
    {
        case GenMdlOptions::MdlVersion::MDL_1_6:
            return MDL_VERSION_SUFFIX_1_6;
        case GenMdlOptions::MdlVersion::MDL_1_7:
            return MDL_VERSION_SUFFIX_1_7;
        case GenMdlOptions::MdlVersion::MDL_1_8:
            return MDL_VERSION_SUFFIX_1_8;
        case GenMdlOptions::MdlVersion::MDL_1_9:
            return MDL_VERSION_SUFFIX_1_9;
        default:
            // GenMdlOptions::MdlVersion::MDL_1_10
            // GenMdlOptions::MdlVersion::MDL_LATEST
            return MDL_VERSION_SUFFIX_1_10;
    }
}

void MdlShaderGenerator::emitMdlVersionFilenameSuffix(GenContext& context, ShaderStage& stage) const
{
    emitString(getMdlVersionFilenameSuffix(context), stage);
}

namespace MDL
{
// Identifiers for MDL variable blocks
const string INPUTS = "i";
const string OUTPUTS = "o";
} // namespace MDL

MATERIALX_NAMESPACE_END
