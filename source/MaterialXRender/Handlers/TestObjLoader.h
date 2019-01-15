#ifndef MATERIALX_TESTOBJLOADER_H
#define MATERIALX_TESTOBJLOADER_H

#include <string>
#include <memory>
#include <MaterialXRender/Handlers/GeometryHandler.h>

namespace MaterialX
{
/// Shared pointer to an TestObjLoader
using TestObjLoaderPtr = std::shared_ptr<class TestObjLoader>;

/// @class @TestObjHandler
/// Utility geometry loader to read in OBJ files for unit testing.
///
class TestObjLoader : public GeometryLoader
{
  public:
    /// Static instance create function
    static TestObjLoaderPtr create() { return std::make_shared<TestObjLoader>(); }

    /// Default constructor
    TestObjLoader() :
        _readGroups(true)
    {
        _extensions = { "obj", "OBJ" };
    }
    
    /// Default destructor
    virtual ~TestObjLoader() {}

    /// Load geometry from disk
    bool load(const std::string& fileName, MeshList& meshList) override;

    /// Set to read groups as partitions. 
    void setReadGroups(bool val)
    {
        _readGroups = val;
    }

    /// Read groups as partitions. Default is false.
    bool readGroups() const
    {
        return _readGroups;
    }

  protected:
    bool _readGroups;
};

} // namespace MaterialX
#endif
