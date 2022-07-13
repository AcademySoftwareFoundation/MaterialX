//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <MaterialXGenMdl/MdlSyntax.h>
#include <MaterialXGenMdl/Nodes/CompoundNodeMdl.h>
#include <MaterialXGenMdl/Nodes/SourceCodeNodeMdl.h>
#include <MaterialXGenMdl/Nodes/MaterialNodeMdl.h>
#include <MaterialXGenMdl/Nodes/SurfaceNodeMdl.h>
#include <MaterialXGenMdl/Nodes/HeightToNormalNodeMdl.h>
#include <MaterialXGenMdl/Nodes/BlurNodeMdl.h>
#include <MaterialXGenMdl/Nodes/CombineNodeMdl.h>
#include <MaterialXGenMdl/Nodes/ClosureLayerNodeMdl.h>
#include <MaterialXGenMdl/Nodes/ClosureCompoundNodeMdl.h>
#include <MaterialXGenMdl/Nodes/ClosureSourceCodeNodeMdl.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/Nodes/SwizzleNode.h>
#include <MaterialXGenShader/Nodes/ConvertNode.h>
#include <MaterialXGenShader/Nodes/SwitchNode.h>
#include <MaterialXGenShader/Nodes/ClosureCompoundNode.h>
#include <MaterialXGenShader/Nodes/ClosureSourceCodeNode.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{
    std::unordered_map<string, string> GEOMPROP_DEFINITIONS =
    {
        {"Pobject", "base::transform_point(base::coordinate_internal, base::coordinate_object, state::position())"},
        {"Pworld", "base::transform_point(base::coordinate_internal, base::coordinate_world, state::position())"},
        {"Nobject", "base::transform_normal(base::coordinate_internal, base::coordinate_object, state::normal())"},
        {"Nworld", "base::transform_normal(base::coordinate_internal, base::coordinate_world, state::normal())"},
        {"Tobject", "base::transform_vector(base::coordinate_internal, base::coordinate_object, state::texture_tangent_u(0))"},
        {"Tworld", "base::transform_vector(base::coordinate_internal, base::coordinate_world, state::texture_tangent_u(0))"},
        {"Bobject", "base::transform_vector(base::coordinate_internal, base::coordinate_object, state::texture_tangent_v(0))"},
        {"Bworld", "base::transform_vector(base::coordinate_internal, base::coordinate_world, state::texture_tangent_v(0))"},
        {"UV0", "float2(state::texture_coordinate(0).x, state::texture_coordinate(0).y)"},
        {"Vworld", "state::direction()"}
    };

    const string MDL_VERSION = "1.6";

    const vector<string> DEFAULT_IMPORTS = {
        "import ::df::*",
        "import ::base::*",
        "import ::math::*",
        "import ::state::*",
        "import ::anno::*",
        "import ::tex::*",
        "import ::mx::swizzle::*",
        "import ::mx::cm::*",
        "using ::mx::core import *",
        "using ::mx::stdlib import *",
        "using ::mx::pbrlib import *",
        "using ::mx::sampling import *",
    };
}

const string MdlShaderGenerator::TARGET = "genmdl";

//
// MdlShaderGenerator methods
//

MdlShaderGenerator::MdlShaderGenerator() :
    ShaderGenerator(MdlSyntax::create())
{
    // Register build-in implementations

    // <!-- <surfacematerial> -->
    registerImplementation("IM_surfacematerial_" + MdlShaderGenerator::TARGET, MaterialNodeMdl::create);

    // <!-- <surface> -->
    registerImplementation("IM_surface_" + MdlShaderGenerator::TARGET, SurfaceNodeMdl::create);

    // <!-- <swizzle> -->
    // <!-- from type : float -->
    registerImplementation("IM_swizzle_float_color3_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_color4_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector2_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector3_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_float_vector4_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : color3 -->
    registerImplementation("IM_swizzle_color3_float_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color3_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_color4_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector2_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector3_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color3_vector4_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : color4 -->
    registerImplementation("IM_swizzle_color4_float_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color3_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_color4_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector2_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector3_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_color4_vector4_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : vector2 -->
    registerImplementation("IM_swizzle_vector2_float_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color3_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_color4_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector2_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector3_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector2_vector4_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : vector3 -->
    registerImplementation("IM_swizzle_vector3_float_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color3_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_color4_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector2_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector3_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector3_vector4_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    // <!-- from type : vector4 -->
    registerImplementation("IM_swizzle_vector4_float_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color3_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_color4_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector2_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector3_" + MdlShaderGenerator::TARGET, SwizzleNode::create);
    registerImplementation("IM_swizzle_vector4_vector4_" + MdlShaderGenerator::TARGET, SwizzleNode::create);

    // <!-- <convert> -->
    registerImplementation("IM_convert_float_color3_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_float_color4_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_float_vector2_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_float_vector3_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_float_vector4_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector2_vector3_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector3_vector2_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector3_color3_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector3_vector4_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector4_vector3_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_vector4_color4_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_color3_vector3_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_color4_vector4_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_color3_color4_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_color4_color3_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_boolean_float_" + MdlShaderGenerator::TARGET, ConvertNode::create);
    registerImplementation("IM_convert_integer_float_" + MdlShaderGenerator::TARGET, ConvertNode::create);

    // <!-- <combine> -->
    registerImplementation("IM_combine2_vector2_" + MdlShaderGenerator::TARGET, CombineNodeMdl::create);
    registerImplementation("IM_combine2_color4CF_" + MdlShaderGenerator::TARGET, CombineNodeMdl::create);
    registerImplementation("IM_combine2_vector4VF_" + MdlShaderGenerator::TARGET, CombineNodeMdl::create);
    registerImplementation("IM_combine2_vector4VV_" + MdlShaderGenerator::TARGET, CombineNodeMdl::create);
    registerImplementation("IM_combine3_color3_" + MdlShaderGenerator::TARGET, CombineNodeMdl::create);
    registerImplementation("IM_combine3_vector3_" + MdlShaderGenerator::TARGET, CombineNodeMdl::create);
    registerImplementation("IM_combine4_color4_" + MdlShaderGenerator::TARGET, CombineNodeMdl::create);
    registerImplementation("IM_combine4_vector4_" + MdlShaderGenerator::TARGET, CombineNodeMdl::create);

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

    // <!-- <thin_film_bsdf> -->
    registerImplementation("IM_thin_film_bsdf_" + MdlShaderGenerator::TARGET, LayerableNodeMdl::create);

    // <!-- <dielectric_bsdf> -->
    registerImplementation("IM_dielectric_bsdf_" + MdlShaderGenerator::TARGET, LayerableNodeMdl::create);

    // <!-- <generalized_schlick_bsdf> -->
    registerImplementation("IM_generalized_schlick_bsdf_" + MdlShaderGenerator::TARGET, LayerableNodeMdl::create);

    // <!-- <sheen_bsdf> -->
    registerImplementation("IM_sheen_bsdf_" + MdlShaderGenerator::TARGET, LayerableNodeMdl::create);
}

ShaderPtr MdlShaderGenerator::generate(const string& name, ElementPtr element, GenContext& context) const
{
    // For MDL we cannot cache node implementations between generation calls,
    // because this generator needs to do edits to subgraphs implementations
    // depending on the context in which a node is used.
    context.clearNodeImplementations();

    ShaderPtr shader = createShader(name, element, context);

    ShaderGraph& graph = shader->getGraph();
    ShaderStage& stage = shader->getStage(Stage::PIXEL);

    // Emit version
    emitLine("mdl " + MDL_VERSION, stage);
    emitLineBreak(stage);

    emitLine("using mx = materialx", stage);

    // Emit module imports
    for (const string& module : DEFAULT_IMPORTS)
    {
        emitLine(module, stage);
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
    emitShaderInputs(stage.getInputBlock(MDL::INPUTS), stage);

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

    if (graph.hasClassification(ShaderNode::Classification::TEXTURE))
    {
        emitLine("color finalOutput__ = mk_color3(" + result + ")", stage);

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
            "    )\n"
            ");";
        emitBlock(textureMaterial, FilePath(), context, stage);
    }
    else
    {
        emitLine(_syntax->getTypeSyntax(outputSocket->getType()).getName() +  " finalOutput__ = " + result, stage);

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

    const TypeDesc* outputType = TypeDesc::get(outputs[0]->getType());

    if (implElement->isA<NodeGraph>())
    {
        // Use a compound implementation.
        if (outputType->isClosure())
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
            // Fall back to source code implementation.
            if (outputType->isClosure())
            {
                impl = ClosureSourceCodeNodeMdl::create();
            }
            else
            {
                impl = SourceCodeNodeMdl::create();
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

    string variable;
    const ShaderNode* upstreamNode = upstreamOutput->getNode();
    if (upstreamNode->numOutputs() > 1)
    {
        variable = upstreamNode->getName() + "_result.mxp_" + upstreamOutput->getName();
    }
    else
    {
        variable = upstreamOutput->getVariable();
    }

    if (!input->getChannels().empty())
    {
        variable = _syntax->getSwizzledVariable(variable, input->getConnection()->getType(), input->getChannels(), input->getType());
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

    // Disconnect any incomming connections to transmission IOR
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
}

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
    for (ShaderNode* node : graph->getNodes())
    {
        node->getImplementation().createVariables(*node, context, *shader);
    }

    // Create inputs for the published graph interface.
    for (ShaderGraphInputSocket* inputSocket : graph->getInputSockets())
    {
        // Only for inputs that are connected/used internally,
        // and are editable by users.
        if (inputSocket->getConnections().size() && graph->isEditable(*inputSocket))
        {
            inputs->add(inputSocket->getSelf());
        }
    }

    // Create outputs from the graph interface.
    for (ShaderGraphOutputSocket* outputSocket : graph->getOutputSockets())
    {
        outputs->add(outputSocket->getSelf());
    }

    // MDL does not allow varying data connected to transmission IOR.
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
    if (graph->hasClassification(ShaderNode::Classification::SHADER) ||
        graph->hasClassification(ShaderNode::Classification::CLOSURE))
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
        // the internal connection to transmssion IOR.
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

void MdlShaderGenerator::emitShaderInputs(const VariableBlock& inputs, ShaderStage& stage) const
{
    const string uniformPrefix = _syntax->getUniformQualifier() + " ";
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        const ShaderPort* input = inputs[i];

        const string& qualifier = input->isUniform() || input->getType()==Type::FILENAME ? uniformPrefix : EMPTY_STRING;
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

        if (i < inputs.size() - 1)
        {
            emitString(",", stage);
        }

        emitLineEnd(stage, false);
    }
}

namespace MDL
{
    // Identifiers for MDL variable blocks
    const string INPUTS   = "i";
    const string OUTPUTS  = "o";
}

MATERIALX_NAMESPACE_END
