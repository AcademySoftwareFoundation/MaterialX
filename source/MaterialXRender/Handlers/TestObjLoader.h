#ifndef MATERIALX_OBJGEOMETRYHANDLER_H
#define MATERIALX_OBJGEOMETRYHANDLER_H

#include <string>
#include <memory>
#include <MaterialXRender/Handlers/GeometryHandler.h>

namespace MaterialX
{
/// Shared pointer to an TestObjLoader
using TestObjLoaderPtr = std::shared_ptr<class TestObjLoader>;

/// @class @TestObjHandler
/// Utility geometry handler to read in OBJ files for unit testing.
///
class TestObjLoader : public GeometryLoader
{
  public:
    /// Static instance create function
    static TestObjLoaderPtr create() { return std::make_shared<TestObjLoader>(); }

    /// Default constructor
    TestObjLoader() 
    {
        _extensions = { "obj", "OBJ" };
    }
    
    /// Default destructor
    virtual ~TestObjLoader() {}

    /// Load geometry from disk
    bool load(const std::string& fileName, MeshList& meshList) override;
};

} // namespace MaterialX
#endif
