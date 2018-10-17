#include <MaterialXGenShader/GenContext.h>

namespace MaterialX
{

void GenContext::addInputSuffix(DagInput* input, const string& suffix)
{
    _inputSuffix[input] = suffix;
}

void GenContext::removeInputSuffix(DagInput* input)
{
    _inputSuffix.erase(input);
}

void GenContext::getInputSuffix(DagInput* input, string& suffix) const
{
    suffix.clear();
    std::unordered_map<DagInput*, string>::const_iterator iter = _inputSuffix.find(input);
    if (iter != _inputSuffix.end())
    {
        suffix = iter->second;
    }
}

void GenContext::addOutputSuffix(DagOutput* output, const string& suffix)
{
    _outputSuffix[output] = suffix;
}

void GenContext::removeOutputSuffix(DagOutput* output)
{
    _outputSuffix.erase(output);
}

void GenContext::getOutputSuffix(DagOutput* output, string& suffix) const
{
    suffix.clear();
    std::unordered_map<DagOutput*, string>::const_iterator iter = _outputSuffix.find(output);
    if (iter != _outputSuffix.end())
    {
        suffix = iter->second;
    }
}

} // namespace MaterialX
