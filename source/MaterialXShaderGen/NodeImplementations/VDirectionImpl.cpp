#include <MaterialXShaderGen/NodeImplementations/VDirectionImpl.h>
#include <MaterialXShaderGen/Shader.h>

namespace
{
    const char* kVDirectionFlipOSL =
        "void vdirection(vector texcoord, output vector result)\n"
        "{\n"
        "   result[0] = texcoord[0];\n"
        "   result[1] = 1.0 - texcoord[1];\n"
        "}\n";

    const char* kVDirectionNoOpOSL =
        "void vdirection(vector texcoord, output vector result)\n"
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

DEFINE_NODE_IMPLEMENTATION(VDirectionImplFlipOsl, "vdirection_flip", "osl", "")
DEFINE_NODE_IMPLEMENTATION(VDirectionImplNoOpOsl, "vdirection_noop", "osl", "")
DEFINE_NODE_IMPLEMENTATION(VDirectionImplFlipGlsl, "vdirection_flip", "glsl", "")
DEFINE_NODE_IMPLEMENTATION(VDirectionImplNoOpGlsl, "vdirection_noop", "glsl", "")

void VDirectionImplFlipOsl::emitCode(const SgNode&, ShaderGenerator&, Shader& shader)
{
    shader.addBlock(kVDirectionFlipOSL);
}

void VDirectionImplNoOpOsl::emitCode(const SgNode&, ShaderGenerator&, Shader& shader)
{
    shader.addBlock(kVDirectionNoOpOSL);
}

void VDirectionImplFlipGlsl::emitCode(const SgNode&, ShaderGenerator&, Shader& shader)
{
    shader.addBlock(kVDirectionFlipGLSL);
}

void VDirectionImplNoOpGlsl::emitCode(const SgNode&, ShaderGenerator&, Shader& shader)
{
    shader.addBlock(kVDirectionNoOpGLSL);
}

} // namespace MaterialX
