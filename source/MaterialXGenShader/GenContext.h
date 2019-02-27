#ifndef MATERIALX_GENCONTEXT_H
#define MATERIALX_GENCONTEXT_H

#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/GenOptions.h>

#include <MaterialXFormat/File.h>

#include <set>

namespace MaterialX
{

using GenUserDataPtr = std::shared_ptr<class GenUserData>;

class GenUserData : public std::enable_shared_from_this<GenUserData>
{
  public:
    virtual ~GenUserData() {}
    
    /// Return a shaderd pointer for this object.
    GenUserDataPtr getSelf()
    {
        return shared_from_this();
    }

    /// Return this object cast to a templated type.
    template<class T> shared_ptr<T> asA()
    {
        return std::dynamic_pointer_cast<T>(getSelf());
    }

    /// Return this object cast to a templated type.
    template<class T> shared_ptr<const T> asA() const
    {
        return std::dynamic_pointer_cast<const T>(getSelf());
    }

  protected:
    GenUserData() {}
};

using GenContextPtr = std::shared_ptr<class GenContext>;

/// A context class for shader generation.
/// Used for thread local storage of data needed during shader generation.
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

    /// Add to the search path used for finding source code.
    void registerSourceCodeSearchPath(const FilePath& path)
    {
        _sourceCodeSearchPath.append(path);
    }

    /// Resolve a file using the registered search paths.
    FilePath findSourceCode(const FilePath& filename) const
    {
        return _sourceCodeSearchPath.find(filename);
    }

    /// Return the source code search path.
    const FileSearchPath& sourceCodeSearchPath()
    {
        return _sourceCodeSearchPath;
    }

    /// Cache a shader node implementation.
    void addNodeImplementation(const string& name, const string& target, ShaderNodeImplPtr impl);

    /// Find and return a cached shader node implementation,
    /// or return nullptr if no implementation is found.
    ShaderNodeImplPtr findNodeImplementation(const string& name, const string& target);

    /// Add user data to the context to make it
    /// available during shader generator.
    void pushUserData(const string& name, GenUserDataPtr data)
    {
        auto it = _userData.find(name);
        if (it != _userData.end())
        {
            it->second.push_back(data);
        }
        else
        {
            _userData[name] = { data };
        }
    }

    /// Remove user data from the context.
    void popUserData(const string& name)
    {
        auto it = _userData.find(name);
        if (it != _userData.end())
        {
            it->second.pop_back();
        }
    }

    /// Return user data with given name,
    /// or nullptr if no data is found.
    template<class T>
    std::shared_ptr<T> getUserData(const string& name)
    {
        auto it = _userData.find(name);
        return it != _userData.end() && !it->second.empty() ? it->second.back()->asA<T>() : nullptr;
    }

    /// Add an input suffix to be used for the input in this context.
    /// @param input Node input
    /// @param suffix Suffix string
    void addInputSuffix(const ShaderInput* input, const string& suffix);

    /// Remove an input suffix to be used for the input in this context.
    /// @param input Node input
    void removeInputSuffix(const ShaderInput* input);

    /// Get an input suffix to be used for the input in this context.
    /// @param input Node input
    /// @param suffix Suffix string returned. Is empty if not found.
    void getInputSuffix(const ShaderInput* input, string& suffix) const;

    /// Add an output suffix to be used for the output in this context.
    /// @param output Node output
    /// @param suffix Suffix string
    void addOutputSuffix(const ShaderOutput* output, const string& suffix);

    /// Remove an output suffix to be used for the output in this context.
    /// @param output Node output
    void removeOutputSuffix(const ShaderOutput* output);

    /// Get an output suffix to be used for the output in this context.
    /// @param output Node output
    /// @param suffix Suffix string returned. Is empty if not found.
    void getOutputSuffix(const ShaderOutput* output, string& suffix) const;

protected:
    // Generation options.
    GenOptions _options;

    // Search path for finding source files.
    FileSearchPath _sourceCodeSearchPath;

    // Cached shader node implementations.
    std::unordered_map<string, ShaderNodeImplPtr> _nodeImpls;

    // User data
    std::unordered_map<string, vector<GenUserDataPtr>> _userData;

    // List of input suffixes
    std::unordered_map<const ShaderInput*, string> _inputSuffix;

    // List of output suffixes
    std::unordered_map<const ShaderOutput*, string> _outputSuffix;
};

} // namespace MaterialX

#endif
