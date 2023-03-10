//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMdl/Nodes/SwizzleNodeMdl.h>

#include <MaterialXGenMdl/Nodes/CompoundNodeMdl.h>

#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <MaterialXGenShader/GenContext.h>


MATERIALX_NAMESPACE_BEGIN

static const string IN_STRING("in");
static const string CHANNELS_STRING("channels");

ShaderNodeImplPtr SwizzleNodeMdl::create()
{
    return std::make_shared<SwizzleNodeMdl>();
}

void SwizzleNodeMdl::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    DEFINE_SHADER_STAGE(stage, Stage::PIXEL)
    {
        const ShaderGenerator& shadergen = context.getShaderGenerator();

        const ShaderInput* in = node.getInput(IN_STRING);
        const ShaderInput* channels = node.getInput(CHANNELS_STRING);
        if (!in || !channels)
        {
            throw ExceptionShaderGenError("Node '" + node.getName() + "' is not a valid swizzle node");
        }
        if (!in->getConnection() && !in->getValue())
        {
            throw ExceptionShaderGenError("No connection or value found to swizzle on node '" + node.getName() + "'");
        }

        const string& swizzle = channels->getValue() ? channels->getValue()->getValueString() : EMPTY_STRING;

        string variableName;
        if (in->getConnection())
        {
            // Allow swizzles also on custom types, like UsdUVTexture.
            // Special handling is required because struct field names follow a special naming scheme in this generator.
            const ShaderNode* upstreamNode = in->getConnection()->getNode();
            const CompoundNodeMdl* upstreamNodeMdl = dynamic_cast<const CompoundNodeMdl*>(&upstreamNode->getImplementation());
            if (upstreamNodeMdl && upstreamNodeMdl->isReturnStruct())
            {
                // apply the same channel mask to the names of the struct fields
                variableName = in->getConnection()->getVariable();
                size_t pos = variableName.find_last_of('_');
                if (pos != std::string::npos)
                {
                    std::string channelMask = variableName.substr(pos);
                    variableName = upstreamNode->getName() + "_result.mxp" + channelMask;
                }
            }
            else
            {
                variableName = in->getConnection()->getVariable();
            }
        }
        else
        {
            variableName = in->getVariable();
        }

        // If the input is unconnected we must declare a variable
        // for it first, in order to swizzle it below.
        if (!in->getConnection())
        {
            string variableValue = in->getValue() ? shadergen.getSyntax().getValue(in->getType(), *in->getValue()) : shadergen.getSyntax().getDefaultValue(in->getType());
            shadergen.emitLine(shadergen.getSyntax().getTypeName(in->getType()) + " " + variableName + " = " + variableValue, stage);
        }

        if (!swizzle.empty())
        {
            const TypeDesc* type = in->getConnection() ? in->getConnection()->getType() : in->getType();
            variableName = shadergen.getSyntax().getSwizzledVariable(variableName, type, swizzle, node.getOutput()->getType());
        }

        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = " + variableName, stage);
        shadergen.emitLineEnd(stage);
    }
}

MATERIALX_NAMESPACE_END

