#ifndef MATERIALX_DATA_H
#define MATERIALX_DATA_H

/// @file
/// MaterialX Data wrapper

#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenOgsXml/OgsXmlGenerator.h>
#include <MaterialXGenShader/Shader.h>

namespace mx = MaterialX;

/// @class MaterialXData
/// Wrapper for MaterialX associated data. 
///
/// Keeps track of an element to render and it's associated document.
///
/// Can optionally create and cache an XML wrapper instance 
/// which wraps up the interface and shader code for code generated based 
/// on the specified element to render.
/// Currently the only language target available is GLSL.
///
struct MaterialXData
{
  public:
    /// The element path and document that the element resides in are passed in
    /// as input arguments.
    /// If the element specified is an empty string then an attempt will be
    /// made to find the first renderable element.
    MaterialXData(  mx::DocumentPtr document,
                    const std::string& elementPath,
                    const mx::FileSearchPath& librarySearchPath);

    MaterialXData(const MaterialXData&) = delete;
    MaterialXData(MaterialXData&&) = delete;

    ~MaterialXData();

    /// Returns the path of the element to render
    std::string getElementPath() const
    {
        return _element ? _element->getNamePath() : mx::EMPTY_STRING;
    }

    MaterialXData& operator=(const MaterialXData&) = delete;
    MaterialXData& operator=(MaterialXData&&) = delete;

    /// Return MaterialX document 
    mx::DocumentPtr getDocument() const
    {
        return _document;
    }

    /// Return the source of the OGS fragment as a string
    const std::string& getFragmentSource() const;

    /// Return name of shader fragment
    // TODO: move out of this class, it should be Maya-agnostic
    const std::string& getFragmentName() const;

    /// Maps XML element paths of MaterialX inputs to their names in the generated shader
    const mx::StringMap& getPathInputMap() const;

    /// Return if the element to render represents a shader graph
    /// as opposed to a texture graph.
    bool elementIsAShader() const;

  private:
    /// Create the OGS XML wrapper for shader fragments associated
    /// with the element set to render
    void generateFragment(const mx::FileSearchPath& librarySearchPath);

    // Reference document and element
    mx::DocumentPtr _document;
    mx::ElementPtr _element;

    // XML fragment name
    std::string _fragmentName;

    // XML fragment wrapper
    std::string _fragmentSource;

    // Mapping from MaterialX Element paths to XML input names
    mx::StringMap _pathInputMap;
};

#endif // MATERIALX_DATA_H
