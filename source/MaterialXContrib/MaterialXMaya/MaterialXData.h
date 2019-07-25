#ifndef MATERIALX_DATA_H
#define MATERIALX_DATA_H

/// @file
/// MaterialX Data wrapper

#include <MaterialXCore/Document.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXRender/ImageHandler.h>

namespace mx = MaterialX;

namespace MaterialXMaya
{

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
class MaterialXData
{
  public:
    /// The element and document that the element resides in are passed in
    /// as input arguments.
    MaterialXData(  mx::DocumentPtr document,
                    mx::ElementPtr element,
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
    const std::string& getFragmentName() const;

    /// Maps XML element paths of MaterialX inputs to their names in the generated shader
    const mx::StringMap& getPathInputMap() const;

    /// Return if the element to render represents a shader graph
    /// as opposed to a texture graph.
    bool elementIsAShader() const;

    /// Get image sampling properties for a given file parameter
    mx::ImageSamplingProperties getImageSamplngProperties(const std::string& fileParameterName) const;

    bool isTransparent() const { return _isTransparent; }

    /// OGS does not support matrix3. As such the matrix4 parameter name is computed from the matrix3 name.
    /// This utility performs this computation.
    static std::string getMatrix4Name(const std::string& matrix3Name);

  private:
    /// Create the OGS XML wrapper for shader fragments associated
    /// with the element set to render
    void generateFragment(const mx::FileSearchPath& librarySearchPath);

    /// References to the document and the element
    mx::DocumentPtr _document;
    mx::ElementPtr _element;

    /// XML fragment name
    std::string _fragmentName;

    // XML fragment source
    std::string _fragmentSource;

    /// Mapping from MaterialX Element paths to XML input names
    mx::StringMap _pathInputMap;

    /// MaterialX shader 
    mx::ShaderPtr _shader;

    bool _isTransparent = false;
};

} // namespace MaterialXMaya

#endif // MATERIALX_DATA_H
