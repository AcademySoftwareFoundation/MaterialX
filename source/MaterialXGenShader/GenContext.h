//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GENCONTEXT_H
#define MATERIALX_GENCONTEXT_H

/// @file
/// Context classes for shader generation

#include <MaterialXGenShader/Library.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/GenOptions.h>

#include <MaterialXFormat/File.h>

#include <set>

namespace MaterialX
{

class GenUserData;

/// Shared pointer to a GenUserData
using GenUserDataPtr = std::shared_ptr<GenUserData>;

/// Shared pointer to a constant GenUserData
using ConstGenUserDataPtr = std::shared_ptr<const GenUserData>;

/// @class GenUserData 
/// Base class for custom user data needed during shader generation.
class GenUserData : public std::enable_shared_from_this<GenUserData>
{
  public:
    virtual ~GenUserData() {}
    
    /// Return a shared pointer for this object.
    GenUserDataPtr getSelf()
    {
        return shared_from_this();
    }

    /// Return a shared pointer for this object.
    ConstGenUserDataPtr getSelf() const
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

/// @class GenContext 
/// A context class for shader generation.
/// Used for thread local storage of data needed during shader generation.
class GenContext
{
  public:
    /// Constructor.
    GenContext(ShaderGeneratorPtr sg);

    /// Return shader generatior.
    const ShaderGenerator& getShaderGenerator() const
    {
        return *_sg;
    }
    
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
    void registerSourceCodeSearchPath(const string& path)
    {
        _sourceCodeSearchPath.append(FilePath(path));
    }

    /// Add to the search path used for finding source code.
    void registerSourceCodeSearchPath(const FilePath& path)
    {
        _sourceCodeSearchPath.append(path);
    }

    /// Add to the search path used for finding source code.
    void registerSourceCodeSearchPath(const FileSearchPath& path)
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
    void addNodeImplementation(const string& name, ShaderNodeImplPtr impl);

    /// Find and return a cached shader node implementation,
    /// or return nullptr if no implementation is found.
    ShaderNodeImplPtr findNodeImplementation(const string& name);

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
    // Shader generator.
    ShaderGeneratorPtr _sg;

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
