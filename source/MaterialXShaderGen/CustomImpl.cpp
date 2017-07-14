#include <MaterialXCore/Library.h>
#include <MaterialXShaderGen/CustomImpl.h>

namespace MaterialX
{

string CustomImpl::id(const string& node, const string& language, const string& target)
{
    return node + "_" + language + "_" + target;
}

} // namespace MaterialX
