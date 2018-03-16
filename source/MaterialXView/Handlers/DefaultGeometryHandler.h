#ifndef MATERIALX_DEFAULTGEOMETRYHANDLER_H
#define MATERIALX_DEFAULTGEOMETRYHANDLER_H

#include <string>
#include <memory>
#include <MaterialXView/Handlers/GeometryHandler.h>

namespace MaterialX
{
/// Shared pointer to an GeometryHandler
using DefaultGeometryHandlerPtr = std::shared_ptr<class DefaultGeometryHandler>;


/// @class @GeometryHandler
/// Utility geometry handler for providing data for shader binding.
///
class DefaultGeometryHandler : public GeometryHandler
{
  public:
    /// Static instance creator
    static DefaultGeometryHandlerPtr creator() { return std::make_shared<DefaultGeometryHandler>(); }

    /// Default constructor
    DefaultGeometryHandler();
    
    /// Default destructor
    virtual ~DefaultGeometryHandler();

    void setIdentifier(const std::string identifier) override;

    IndexBuffer& getIndexing(size_t &bufferSize) override;
    FloatBuffer& getPositions(size_t &bufferSize) override;
    FloatBuffer& getNormals(size_t &bufferSize) override;
    FloatBuffer& getTextureCoords(size_t &bufferSize, unsigned int index = 0) override;
    FloatBuffer& getTangents(size_t &bufferSize, unsigned int index = 0) override;
    FloatBuffer& getBitangents(size_t &bufferSize, unsigned int index = 0) override;
    FloatBuffer& getColors(size_t &bufferSize, unsigned int index = 0) override;

  private:
    void clearData();

    IndexBuffer _indexing;
    FloatBuffer _positionData;
    FloatBuffer _normalData;
    FloatBuffer _texcoordData;
    FloatBuffer _tangentData;
    FloatBuffer _bitangentData;
    FloatBuffer _colorData;
};

} // namespace MaterialX
#endif
