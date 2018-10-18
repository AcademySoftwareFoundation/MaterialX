#include <MaterialXGenShader/GenContext.h>

namespace MaterialX
{

void GenContext::addInputSuffix(ShaderInput* input, const string& suffix)
{
    _inputSuffix[input] = suffix;
}

void GenContext::removeInputSuffix(ShaderInput* input)
{
    _inputSuffix.erase(input);
}

void GenContext::getInputSuffix(ShaderInput* input, string& suffix) const
{
    suffix.clear();
    std::unordered_map<ShaderInput*, string>::const_iterator iter = _inputSuffix.find(input);
    if (iter != _inputSuffix.end())
    {
        suffix = iter->second;
    }
}

void GenContext::addOutputSuffix(ShaderOutput* output, const string& suffix)
{
    _outputSuffix[output] = suffix;
}

void GenContext::removeOutputSuffix(ShaderOutput* output)
{
    _outputSuffix.erase(output);
}

void GenContext::getOutputSuffix(ShaderOutput* output, string& suffix) const
{
    suffix.clear();
    std::unordered_map<ShaderOutput*, string>::const_iterator iter = _outputSuffix.find(output);
    if (iter != _outputSuffix.end())
    {
        suffix = iter->second;
    }
}

} // namespace MaterialX
