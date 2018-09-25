#ifndef MATERIALX_GEOMETRYHANDLER_H
#define MATERIALX_GEOMETRYHANDLER_H

#include <MaterialXCore/Types.h>
#include <string>
#include <memory>
#include <vector>

namespace MaterialX
{
/// Shared pointer to an GeometryHandler
using GeometryHandlerPtr = std::shared_ptr<class GeometryHandler>;

/// @class @GeometryHandler
/// Abstract class representing a geometry handler for providing data for shader binding.
///
class GeometryHandler
{
  public:
    /// Geometry index buffer
    using IndexBuffer = std::vector<unsigned int>;
    /// Float geometry buffer
    using FloatBuffer = std::vector<float>;
      
    /// Default constructor
    GeometryHandler();
    
    /// Default destructor
    virtual ~GeometryHandler();

    /// Set the identifier which will indicate what geometry to return
    /// e.g. UNIT_QUAD is one identifier
    virtual void setIdentifier(const std::string& identifier);
    
    /// Get geometry identifier
    const std::string& getIdentifier() const
    {
        return _identifier;
    }

    /// Create indexing data for geometry. The indexing is assumed to 
    /// be for a set of triangles. That is every 3 values index triangle data.
    virtual IndexBuffer& getIndexing() = 0;

    /// Create position data. 
    /// @param stride The stride between elements. Returned.
    /// @param index Attribute index for identifying different sets of data. Default value is 0.
    /// Passing in a 0 indicates to use the "default" or first set of data.
    virtual FloatBuffer& getPositions(unsigned int& stride, unsigned int index = 0) = 0;

    /// Create normal data.
    /// @param stride The stride between elements. Returned.
    /// @param index Attribute index for identifying different sets of data. Default value is 0.
    /// Passing in a 0 indicates to use the "default" or first set of data.
    virtual FloatBuffer& getNormals(unsigned int& stride, unsigned int index = 0) = 0;

    /// Create texture coordinate (uv) data.
    /// @param stride The stride between elements. Returned.
    /// @param index Attribute index for identifying different sets of data. Default value is 0.
    /// Passing in a 0 indicates to use the "default" or first set of data.
    virtual FloatBuffer& getTextureCoords(unsigned int& stride, unsigned int index = 0) = 0;

    /// Create tangent data.
    /// @param stride The stride between elements. Returned.
    /// @param index Attribute index for identifying different sets of data. Default value is 0.
    /// Passing in a 0 indicates to use the "default" or first set of data.
    virtual FloatBuffer& getTangents(unsigned int& stride, unsigned int index = 0) = 0;

    /// Create bitangent data.
    /// @param stride The stride between elements. Returned.
    /// @param index Attribute index for identifying different sets of data. Default value is 0.
    /// Passing in a 0 indicates to use the "default" or first set of data.
    virtual FloatBuffer& getBitangents(unsigned int& stride, unsigned int index = 0) = 0;

    /// Create color data.
    /// @param stride The stride between elements. Returned.
    /// @param index Attribute index for identifying different sets of data. Default value is 0.
    /// Passing in a 0 indicates to use the "default" or first set of data.
    virtual FloatBuffer& getColors(unsigned int& stride, unsigned int index = 0) = 0;

    /// Create attribute data. 
    /// @param attributeType String indicating type of attribute. Any string which starts
    /// with the attribute constants for this class will call the corresponding attribute
    /// specific method in this class. e.g. A string starting with POSITION_ATTRIBUTE
    /// will call the getPositions() method.
    /// @param stride The stride between elements. Returned.
    /// @param index Attribute index for identifying different sets of data. Default value is 0.
    /// Passing in a 0 indicates to use the "default" or first set of data.
    virtual FloatBuffer& getAttribute(const std::string& attributeType, unsigned int& stride, unsigned int index = 0) = 0;

    /// Return the minimum bounds for the geometry
    virtual const MaterialX::Vector3& getMinimumBounds()
    {
        return _minimumBounds;
    }

    /// Return the minimum bounds for the geometry
    virtual const MaterialX::Vector3& getMaximumBounds()
    {
        return _maximumBounds;
    }

    /// Geometry identifier indicating to create data for a unit quad.
    /// The quad is assumed to be positioned along the X/Y plane.
    static const std::string UNIT_QUAD;

    /// Geometry attribute types
    static const std::string POSITION_ATTRIBUTE;
    static const std::string NORMAL_ATTRIBUTE;
    static const std::string TEXCOORD_ATTRIBUTE;
    static const std::string TANGENT_ATTRIBUTE;
    static const std::string BITANGENT_ATTRIBUTE;
    static const std::string COLOR_ATTRIBUTE;

  protected:
     /// Geometry identifier
     std::string _identifier;

     /// Geometry bounds based on the identifier
     MaterialX::Vector3 _minimumBounds;
     MaterialX::Vector3 _maximumBounds;
};

} // namespace MaterialX
#endif
