#include <MaterialXShaderGen/NodeImplementations/Constant.h>
#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/ShaderGenerator.h>

namespace MaterialX
{

DEFINE_NODE_IMPLEMENTATION(Constant, "constant", "", "")

void Constant::emitCode(const SgNode& sgnode, ShaderGenerator& shadergen, Shader& shader)
{
    const Node& node = sgnode.getNode();

    shader.beginLine();
    shadergen.emitOutput(node, true, shader);
    shader.addStr(" = ");
    shadergen.emitInput(*node.getParameter("value"), shader);
    shader.endLine();
}

} // namespace MaterialX
