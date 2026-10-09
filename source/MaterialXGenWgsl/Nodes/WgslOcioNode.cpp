//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifdef MATERIALX_BUILD_OCIO

#include <MaterialXGenWgsl/Nodes/WgslOcioNode.h>

#include <MaterialXGenShader/Exception.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>

#include <MaterialXFormat/File.h>

MATERIALX_NAMESPACE_BEGIN

namespace
{

const char OCIO_WGSL_LIBRARY_DIR[] = "stdlib/genwgsl/ocio/";

} // anonymous namespace

ShaderNodeImplPtr WgslOcioNode::create()
{
    return std::make_shared<WgslOcioNode>();
}

void WgslOcioNode::emitFunctionDefinition(const ShaderNode& /*node*/, GenContext& context, ShaderStage& stage) const
{
    if (stage.getName() != Stage::PIXEL)
    {
        return;
    }

    const ShaderGenerator& shadergen = context.getShaderGenerator();
    const string functionName = getFunctionName();
    const FilePath libraryPath(OCIO_WGSL_LIBRARY_DIR + functionName + ".wgsl");
    const FilePath libraryPrefix = context.getOptions().libraryPrefix;
    const FilePath fullLibraryPath = libraryPrefix.isEmpty() ? libraryPath : libraryPrefix / libraryPath;
    const FilePath resolved = context.resolveSourceFile(fullLibraryPath, FilePath());
    if (!resolved.exists())
    {
        throw ExceptionShaderGenError("Baked OCIO WGSL not found for " + getName() + " (expected " +
                                      fullLibraryPath.asString() +
                                      "). Regenerate with mxgenwgsl.py --bake-ocio.");
    }

    shadergen.emitLibraryInclude(libraryPath, context, stage);
}

void WgslOcioNode::emitFunctionCall(const ShaderNode& node, GenContext& context, ShaderStage& stage) const
{
    if (stage.getName() != Stage::PIXEL)
    {
        return;
    }

    const bool isColor3 = getName().back() == '3';
    const ShaderGenerator& shadergen = context.getShaderGenerator();
    const string functionName = getFunctionName();
    const ShaderOutput* output = node.getOutput();
    const ShaderInput* colorInput = node.getInput(0);

    shadergen.emitLineBegin(stage);
    shadergen.emitOutput(output, true, false, context, stage);
    shadergen.emitString(" = ", stage);
    shadergen.emitString(functionName + "(", stage);
    if (isColor3)
    {
        shadergen.emitString("vec4f(", stage);
    }
    shadergen.emitInput(colorInput, context, stage);
    if (isColor3)
    {
        shadergen.emitString(", 1.0)", stage);
    }
    shadergen.emitString(")", stage);
    if (isColor3)
    {
        shadergen.emitString(".xyz", stage);
    }
    shadergen.emitLineEnd(stage);
}

MATERIALX_NAMESPACE_END

#endif // MATERIALX_BUILD_OCIO
