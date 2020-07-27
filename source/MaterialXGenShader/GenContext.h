//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_GENCONTEXT_H
#define MATERIALX_GENCONTEXT_H

/// @file
/// Context classes for shader generation

#include <MaterialXGenShader/Library.h>

#include <MaterialXGenShader/GenOptions.h>
#include <MaterialXGenShader/GenUserData.h>
#include <MaterialXGenShader/ShaderNode.h>

#include <MaterialXFormat/File.h>

namespace MaterialX
{

/// @class GenContext 
/// A context class for shader generation.
/// Used for thread local storage of data needed during shader generation.
class GenContext
{
  public:
    /// Constructor.
    GenContext(ShaderGeneratorPtr sg);

    /// Return shader generatior.
    ShaderGenerator& getShaderGenerator()
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
    FilePath resolveSourceFile(const FilePath& filename) const
    {
        return _sourceCodeSearchPath.find(filename);
    }

    /// Add reserved words that should not be used as
    /// identifiers during code generation.
    void addReservedWords(const StringSet& names)
    {
        _reservedWords.insert(names.begin(), names.end());
    }

    /// Return the set of reserved words that should not be used
    /// as identifiers during code generation.
    const StringSet& getReservedWords() const
    {
        return _reservedWords;
    }

    /// Cache a shader node implementation.
    void addNodeImplementation(const string& name, ShaderNodeImplPtr impl);

    /// Find and return a cached shader node implementation,
    /// or return nullptr if no implementation is found.
    ShaderNodeImplPtr findNodeImplementation(const string& name) const;

    /// Get the names of all cached node implementations.
    void getNodeImplementationNames(StringSet& names);

    /// Clear all cached shader node implementation.
    void clearNodeImplementations();

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

    /// Clear all user data from the context.
    void clearUserData();

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
    GenContext() = delete;

    // Shader generator.
    ShaderGeneratorPtr _sg;

    // Generation options.
    GenOptions _options;

    // Search path for finding source files.
    FileSearchPath _sourceCodeSearchPath;

    // Set of globally reserved words.
    StringSet _reservedWords;

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

#endif // MATERIALX_GENCONTEXT_H
