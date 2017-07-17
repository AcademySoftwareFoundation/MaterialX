#include <MaterialXCore/Library.h>
#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

string NodeImplementation::id(const string& node, const string& language, const string& target)
{
    return node + "_" + language + "_" + target;
}

} // namespace MaterialX
