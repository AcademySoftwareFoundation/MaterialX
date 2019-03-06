//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_TINYOBJLOADER_H
#define MATERIALX_TINYOBJLOADER_H

#include <string>
#include <memory>
#include <MaterialXRender/Handlers/GeometryHandler.h>

namespace MaterialX
{
/// Shared pointer to an TinyObjLoader
using TinyObjLoaderPtr = std::shared_ptr<class TinyObjLoader>;

/// @class @TinyObjLoader
/// Wrapper for geometry loader to read in OBJ files using the TinyObjLoader library.
///
class TinyObjLoader : public GeometryLoader
{
  public:
    /// Static instance create function
    static TinyObjLoaderPtr create() { return std::make_shared<TinyObjLoader>(); }

    /// Default constructor
    TinyObjLoader()
    {
        _extensions = { "obj", "OBJ" };
    }
    
    /// Default destructor
    virtual ~TinyObjLoader() {}

    /// Load geometry from disk
    bool load(const std::string& fileName, MeshList& meshList) override;
};

} // namespace MaterialX
#endif
