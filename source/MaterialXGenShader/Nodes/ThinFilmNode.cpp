//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Nodes/ThinFilmNode.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Nodes/LayerNode.h>

namespace MaterialX
{

namespace Type
{
    const TypeDesc* THINFILM = TypeDesc::registerType("thinfilm", TypeDesc::BASETYPE_NONE, TypeDesc::SEMANTIC_NONE, 1, false);
}

const string ThinFilmNode::THICKNESS      = "thickness";
const string ThinFilmNode::IOR            = "ior";
const string ThinFilmNode::THINFILM_INPUT = "tf";

ShaderNodeImplPtr ThinFilmNode::create()
{
    return std::make_shared<ThinFilmNode>();
}

void ThinFilmNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    BEGIN_SHADER_STAGE(stage, Stage::PIXEL)
        const ShaderGenerator& shadergen = context.getShaderGenerator();
        const Syntax& syntax = shadergen.getSyntax();

        const ShaderInput* thickness = node.getInput(THICKNESS);
        const ShaderInput* ior = node.getInput(IOR);
        const ShaderOutput* output = node.getOutput();
        if (!(thickness && ior && output))
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid thin_film_bsdf node");
        }

        shadergen.emitLine(syntax.getTypeName(Type::THINFILM) + " " + output->getVariable(), stage);
        shadergen.emitLine(output->getVariable() + "." + THICKNESS + " = " + shadergen.getUpstreamResult(thickness, context), stage);
        shadergen.emitLine(output->getVariable() + "." + IOR + " = " + shadergen.getUpstreamResult(ior, context), stage);

    END_SHADER_STAGE(stage, Stage::PIXEL)
}

void ThinFilmNode::addInputs(ShaderNode& node, GenContext&) const
{
    // Add layering support.
    LayerNode::addLayerSupport(node);
}

void ThinFilmNode::addThinFilmSupport(ShaderNode& node)
{
    // Add the input to hold thinfilm data.
    node.addInput(ThinFilmNode::THINFILM_INPUT, Type::THINFILM);
}

} // namespace MaterialX
