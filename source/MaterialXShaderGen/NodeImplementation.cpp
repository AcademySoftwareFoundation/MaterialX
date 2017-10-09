#include <MaterialXCore/Library.h>
#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

void NodeImplementation::emitFunction(const SgNode&, ShaderGenerator&, Shader&)
{
    // default implementation has no function definition
}

void NodeImplementation::emitFunctionCall(const SgNode&, ShaderGenerator&, Shader&)
{
    // default implementation has no source code
}

string NodeImplementation::id(const string& node, const string& language, const string& target)
{
    return node + "_" + language + "_" + target;
}

} // namespace MaterialX
