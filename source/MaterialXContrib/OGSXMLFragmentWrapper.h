#ifndef MATERIALX_OGSX_ML_FRAGMENT_WRAPPER_H
#define MATERIALX_OGSX_ML_FRAGMENT_WRAPPER_H

/// @file
/// Contribution utility methods

#include <MaterialXGenShader/Library.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Element.h>

namespace MaterialX
{
class ShaderPort;

// Utility class to extract property information from a MaterialX shader port
class OGSXMLPropertyExtractor
{
public:
    void getFlags(const ShaderPort* port, string& flags, bool isGlobal) const;
    string getUniformSemantic(const ShaderPort* port) const;
    bool isGlobalUniform(const ShaderPort* port, const string& remappedName) const;
    void getStreamInformation(const ShaderPort* port, string& name, string& semantic) const;
};

/// @class OGSXMLFragmentWrapper
/// Class representing an OGS XML Fragment Wrapper
/// Interfaces allow the wrapper to be generated based on an input node and generator
/// The wrapper can contain fragment descriptions for more than one node if desired.
class OGSXMLFragmentWrapper
{
  public:
    /// Default constructor
    OGSXMLFragmentWrapper();

    /// Default desctructor
    ~OGSXMLFragmentWrapper();

    /// @name Generation
    /// @{

    /// Add a fragment wrapper for a shader generated from a given element
    void generate(const string& shaderName, ElementPtr node, GenContext& context);

    /// Get the contents of the cached XML document as a stream.
    void getXML(std::ostream& stream) const;

    /// Set to output vertex shader code (if any)
    void setOutputVertexShader(bool val)
    {
        _outputVertexShader = val;
    }

    /// @}
    /// @name Query
    /// @{ 

    /// Query if we want to output vertex shader code
    bool getOutputVertexShader() const
    {
        return _outputVertexShader;
    }

    /// Get list of global inputs which are not associated with any Element
    const StringVec&  getGlobalsList() const
    {
        return _globalsList;
    }

    /// Get list of Element paths and corresponding fragment input names
    const StringMap& getPathInputMap() const
    {
        return _pathInputMap;
    }

    /// Get list of Element paths and corresponding fragment output names
    /// If the output is a ShaderRef then the path is to that element
    /// as there are no associated child output Elements.
    const StringMap& getPathOutputMap() const
    {
        return _pathOutputMap;
    }

    /// Get fragment name
    const string& getFragmentName() const
    {
        return _fragmentName;
    }

    /// @}
    /// @name Load
    /// @{

    /// Utility to read in an XML fragment from an input stream and return in an output stream
    static void readDocument(std::istream& istream, std::ostream& ostream);

    /// @}

  protected:
    // Mapping from MTLX keywords to OGS fragment XML keywords
    StringMap _typeMap;

    bool _outputVertexShader = false;
    
    // Internally cache XML document
    void* _xmlDocument;

    // Fragment name
    string _fragmentName;

    // List of globals which are not associated with any Element.
    StringVec _globalsList;

    // Mapping from MaterialX Element paths to fragment input names
    StringMap _pathInputMap;

    // Mapping from MaterialX Element paths to fragment output names
    StringMap _pathOutputMap;

    // Context for generating shaders
    GenContext* _context;

    // Utility extractor
    OGSXMLPropertyExtractor _extractor;
};

} // namespace MaterialX

#endif
