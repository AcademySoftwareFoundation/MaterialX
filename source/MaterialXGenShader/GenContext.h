#ifndef MATERIALX_GENCONTEXT_H
#define MATERIALX_GENCONTEXT_H

#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/GenOptions.h>

#include <set>

namespace MaterialX
{

using GenContextPtr = std::shared_ptr<class GenContext>;

/// Class representing an shader generation context for a node.
///
/// For some shader generators a node might need customization to it's implementation 
/// depending on in which context the node is used. This class handles customizations
/// by adding a suffix to the function name to distinguish between the functions for 
/// different contexts. I also supports adding extra arguments to the node's function.
///
/// An example of where this is required if for BSDF and EDF nodes for HW targets 
/// where extra arguments are needed to give directions vectors for evaluation. 
/// For BSDF nodes another use-case is to distinguish between evaluation in a direct lighting
/// context and an indirect lighting context where different versions of the node's function
/// is required.
/// 
class GenContext
{
  public:
    /// Constructor.
    GenContext()
    {}

    /// Constructor.
    GenContext(const GenOptions& options)
        : _options(options)
    {}

    /// Return shader generation options.
    GenOptions& getOptions()
    {
        return _options;
    }

    /// Return shader generation options.
    const GenOptions& getOptions() const
    {
        return _options;
    }

    /// Add an input suffix to be used for the node function in this context.
    /// @param input Node input
    /// @param suffix Suffix string
    void addInputSuffix(const ShaderInput* input, const string& suffix);

    /// Remove an input suffix to be used for the node function in this context.
    /// @param input Node input
    void removeInputSuffix(const ShaderInput* input);

    /// Get an input suffix to be used for the node function in this context.
    /// @param input Node input
    /// @param suffix Suffix string returned. Is empty if not found.
    void getInputSuffix(const ShaderInput* input, string& suffix) const;

    /// Add an output suffix to be used for the node function in this context.
    /// @param output Node output
    /// @param suffix Suffix string
    void addOutputSuffix(const ShaderOutput* output, const string& suffix);

    /// Remove an output suffix to be used for the node function in this context.
    /// @param output Node output
    void removeOutputSuffix(const ShaderOutput* output);

    /// Get an output suffix to be used for the node function in this context.
    /// @param output Node output
    /// @param suffix Suffix string returned. Is empty if not found.
    void getOutputSuffix(const ShaderOutput* output, string& suffix) const;

    template<typename T>
    void pushUserData(const string& name, T* ptr)
    {
        auto it = _userData.find(name);
        if (it != _userData.end())
        {
            it->second.push_back((void*)ptr);
        }
        else
        {
            _userData[name] = {(void*)ptr};
        }
    }

    void popUserData(const string& name)
    {
        auto it = _userData.find(name);
        if (it != _userData.end())
        {
            it->second.pop_back();
        }
    }
    template<typename T>
    T* getUserData(const string& name)
    {
        auto it = _userData.find(name);
        return it != _userData.end() && !it->second.empty() ? (T*)it->second.back() : nullptr;
    }

protected:
    GenOptions _options;

    // List of input suffixes
    std::unordered_map<const ShaderInput*, string> _inputSuffix;

    // List of output suffixes
    std::unordered_map<const ShaderOutput*, string> _outputSuffix;

    // User data
    std::unordered_map<string, vector<void*>> _userData;
};

} // namespace MaterialX

#endif
