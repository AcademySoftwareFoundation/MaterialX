//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMdl/Nodes/ImageNodeMdl.h>
#include <MaterialXGenMdl/Nodes/HeightToNormalNodeMdl.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/GenContext.h>

MATERIALX_NAMESPACE_BEGIN

const string ImageNodeMdl::FLIP_V = "flip_v";

ShaderNodeImplPtr ImageNodeMdl::create()
{
    return std::make_shared<ImageNodeMdl>();
}

void ImageNodeMdl::addInputs(ShaderNode& node, GenContext& context) const
{
    BASE::addInputs(node, context);
    node.addInput(ImageNodeMdl::FLIP_V, Type::BOOLEAN)->setUniform();
}

bool ImageNodeMdl::isEditable(const ShaderInput& input) const
{
    if (input.getName() == ImageNodeMdl::FLIP_V)
    {
        return false;
    }
    return BASE::isEditable(input);
}

void ImageNodeMdl::emitFunctionCall(const ShaderNode& _node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        ShaderNode& node = const_cast<ShaderNode&>(_node);

        ShaderOutput* output = node.getOutput();
        string outputSuffix;
        context.getOutputSuffix(output, outputSuffix);
        if (outputSuffix.empty() && output && !output->getConnections().empty())
        {
            bool sampledOnly = true;
            for (const ShaderInput* connection : output->getConnections())
            {
                const ShaderNodeImpl& downstreamImpl = connection->getNode()->getImplementation();
                if (!dynamic_cast<const HeightToNormalNodeMdl*>(&downstreamImpl))
                {
                    sampledOnly = false;
                    break;
                }
            }
            if (sampledOnly)
            {
                return;
            }
        }

        ShaderInput* flipUInput = node.getInput(ImageNodeMdl::FLIP_V);
        ValuePtr value = TypedValue<bool>::createValue(context.getOptions().fileTextureVerticalFlip);
        if (flipUInput)
        {
            flipUInput->setValue(value);
        }
        BASE::emitFunctionCall(_node, context, stage);
    }
}

MATERIALX_NAMESPACE_END
