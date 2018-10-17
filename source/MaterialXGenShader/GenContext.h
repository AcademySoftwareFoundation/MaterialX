#ifndef MATERIALX_GENCONTEXT_H
#define MATERIALX_GENCONTEXT_H

#include <MaterialXGenShader/DagNode.h>

#include <set>

namespace MaterialX
{

/// A function argument for node implementation functions.
/// A argument is a pair of strings holding the 'type' and 'name' of the argument.
using Argument = std::pair<string, string>;
using Arguments = vector<Argument>;

using GenContextPtr = std::shared_ptr<class GenContext>;

/// Class representing an implementation context for a node.
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
    /// Constructor, set the identifier for this context.
    GenContext(int id) : _id(id) {}

    /// Return the identifier for this context.
    int id() const { return _id; }

    /// Add an extra argument to be used for the node function in this context.
    void addArgument(const Argument& arg) { _arguments.push_back(arg); }

    /// Return a list of extra argument to be used for the node function in this context.
    const Arguments& getArguments() const { return _arguments; }

    /// Set a function name suffix to be used for the node function in this context.
    void setFunctionSuffix(const string& suffix) { _functionSuffix = suffix; }

    /// Return the function name suffix to be used for the node function in this context.
    const string& getFunctionSuffix() const { return _functionSuffix; }

    /// Add an input suffix to be used for the node function in this context.
    /// @param input Node input
    /// @param suffix Suffix string
    void addInputSuffix(DagInput* input, const string& suffix);

    /// Remove an input suffix to be used for the node function in this context.
    /// @param input Node input
    void removeInputSuffix(DagInput* input);

    /// Get an input suffix to be used for the node function in this context.
    /// @param input Node input
    /// @param suffix Suffix string returned. Is empty if not found.
    void getInputSuffix(DagInput* input, string& suffix) const;

    /// Add an output suffix to be used for the node function in this context.
    /// @param input Node output
    /// @param suffix Suffix string
    void addOutputSuffix(DagOutput* output, const string& suffix);

    /// Remove an output suffix to be used for the node function in this context.
    /// @param output Node output
    void removeOutputSuffix(DagOutput* output);

    /// Get an output suffix to be used for the node function in this context.
    /// @param output Node output
    /// @param suffix Suffix string returned. Is empty if not found.
    void getOutputSuffix(DagOutput* output, string& suffix) const;

private:
    const int _id;
    Arguments _arguments;
    string _functionSuffix;

    // List of input suffixes
    std::unordered_map<DagInput*, string> _inputSuffix;

    // List of output suffixes
    std::unordered_map<DagOutput*, string> _outputSuffix;
};

} // namespace MaterialX

#endif
