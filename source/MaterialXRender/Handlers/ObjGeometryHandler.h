#ifndef MATERIALX_OBJGEOMETRYHANDLER_H
#define MATERIALX_OBJGEOMETRYHANDLER_H

#include <string>
#include <memory>
#include <MaterialXRender/Handlers/GeometryHandler.h>

namespace MaterialX
{
/// Shared pointer to an GeometryHandler
using ObjGeometryHandlerPtr = std::shared_ptr<class ObjGeometryHandler>;


/// @class @ObjGeometryHandler
/// Utility geometry handler to read in OBJ files to providing data for shader binding.
///
class ObjGeometryHandler : public GeometryHandler
{
  public:
      /// Static instance create function
      static ObjGeometryHandlerPtr create() { return std::make_shared<ObjGeometryHandler>(); }

    /// Default constructor
    ObjGeometryHandler();
    
    /// Default destructor
    virtual ~ObjGeometryHandler();

    /// Set the OBJ file name to read from
    void setIdentifier(const std::string& identifier) override;

    IndexBuffer& getIndexing() override;
    FloatBuffer& getPositions(unsigned int& stride, unsigned int index = 0) override;
    FloatBuffer& getNormals(unsigned int& stride, unsigned int index = 0) override;
    FloatBuffer& getTextureCoords(unsigned int& stride, unsigned int index = 0) override;
    FloatBuffer& getTangents(unsigned int& stride, unsigned int index = 0) override;
    FloatBuffer& getBitangents(unsigned int& stride, unsigned int index = 0) override;
    FloatBuffer& getColors(unsigned int& stride, unsigned int index = 0) override;
    FloatBuffer& getAttribute(const std::string& attributeType, unsigned int& stride, unsigned int index = 0) override;
    const Vector3& getMinimumBounds() override;
    Vector3& getMaximumBounds() override;

  private:
    /// Read data from file based on the identifier set
    void readData();

    /// Set up data for a default quadrilaterial
    void setQuadData();

    /// Clear any existing data
    void clearData();

    IndexBuffer _indexing;
    FloatBuffer _positionData;
    FloatBuffer _normalData;
    FloatBuffer _texcoordData[2];
    FloatBuffer _tangentData[2];
    FloatBuffer _bitangentData[2];
    FloatBuffer _colorData[2];
};

} // namespace MaterialX
#endif
