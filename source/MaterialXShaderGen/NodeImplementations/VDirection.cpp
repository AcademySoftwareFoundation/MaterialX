#include <MaterialXShaderGen/NodeImplementations/VDirection.h>
#include <MaterialXShaderGen/Shader.h>

namespace
{
    const char* kVDirectionFlipOSL =
        "void vdirection(vector2 texcoord, output vector2 result)\n"
        "{\n"
        "   result.x = texcoord.x;\n"
        "   result.y = 1.0 - texcoord.y;\n"
        "}\n";

    const char* kVDirectionNoOpOSL =
        "void vdirection(vector2 texcoord, output vector2 result)\n"
        "{\n"
        "   result = texcoord;\n"
        "}\n";

    const char* kVDirectionFlipGLSL =
        "void vdirection(vec2 texcoord, out vec2 result)\n"
        "{\n"
        "   result.x = texcoord.x;\n"
        "   result.y = 1.0 - texcoord.y;\n"
        "}\n";

    const char* kVDirectionNoOpGLSL =
        "void vdirection(vec2 texcoord, out vec2 result)\n"
        "{\n"
        "   result = texcoord;\n"
        "}\n";
}

namespace MaterialX
{

DEFINE_NODE_IMPLEMENTATION(VDirectionFlipOsl, "vdirection_flip", "osl", "")
DEFINE_NODE_IMPLEMENTATION(VDirectionNoOpOsl, "vdirection_noop", "osl", "")
DEFINE_NODE_IMPLEMENTATION(VDirectionFlipGlsl, "vdirection_flip", "glsl", "")
DEFINE_NODE_IMPLEMENTATION(VDirectionNoOpGlsl, "vdirection_noop", "glsl", "")

void VDirectionFlipOsl::emitCode(const SgNode&, ShaderGenerator&, Shader& shader)
{
    shader.addBlock(kVDirectionFlipOSL);
}

void VDirectionNoOpOsl::emitCode(const SgNode&, ShaderGenerator&, Shader& shader)
{
    shader.addBlock(kVDirectionNoOpOSL);
}

void VDirectionFlipGlsl::emitCode(const SgNode&, ShaderGenerator&, Shader& shader)
{
    shader.addBlock(kVDirectionFlipGLSL);
}

void VDirectionNoOpGlsl::emitCode(const SgNode&, ShaderGenerator&, Shader& shader)
{
    shader.addBlock(kVDirectionNoOpGLSL);
}

} // namespace MaterialX
