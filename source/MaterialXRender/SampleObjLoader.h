//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SAMPLEOBJLOADER_H
#define MATERIALX_SAMPLEOBJLOADER_H

/// @file
/// Sample OBJ geometry loader

#include <string>
#include <memory>
#include <MaterialXRender/GeometryHandler.h>

namespace MaterialX
{
/// Shared pointer to an SampleObjLoader
using SampleObjLoaderPtr = std::shared_ptr<class SampleObjLoader>;

/// @class SampleObjLoader
/// Utility geometry loader to read in OBJ files for unit testing.
///
class SampleObjLoader : public GeometryLoader
{
  public:
    /// Static instance create function
    static SampleObjLoaderPtr create() { return std::make_shared<SampleObjLoader>(); }

    /// Default constructor
    SampleObjLoader() :
        _readGroups(true),
        _debugDump(false)
    {
        _extensions = { "obj", "OBJ" };
    }
    
    /// Default destructor
    virtual ~SampleObjLoader() {}

    /// Load geometry from disk
    bool load(const FilePath& filePath, MeshList& meshList) override;

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
    bool _debugDump;
};

} // namespace MaterialX
#endif
