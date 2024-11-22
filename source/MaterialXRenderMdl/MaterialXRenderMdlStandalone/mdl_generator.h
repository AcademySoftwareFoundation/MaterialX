/******************************************************************************
 * Copyright 2025 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/

 // examples/mdl_sdk/dxr/mdl_d3d12/materialx/mdl_generator.h

#ifndef MATERIALX_MDL_GENERATOR_H
#define MATERIALX_MDL_GENERATOR_H

#include <MaterialXGenMdl/MdlShaderGenerator.h>
#include <string>

namespace mi { namespace neuraylib {
    class IMdl_configuration;
}}


// --------------------------------------------------------------------------------------------

/// Basic MDL code-gen from MaterialX.
/// A more sophisticated supports will be needed for full function support.
class MdlGenerator
{
public:
    struct Result
    {
        std::string materialxFilename;
        std::string generatedMdlCode;
        std::string materialxMaterialName;
        std::string generatedMdlName;
    };

    /// Constructor.
    explicit MdlGenerator();

    /// Destructor.
    ~MdlGenerator() = default;

    /// Specify an additional absolute search path location (e.g. '/projects/MaterialX').
    /// This path will be queried when locating standard data libraries,
    /// XInclude references, and referenced images.
    void AddMaterialxSearchPath(const std::string& mtlxPath);

    /// Specify an additional relative path to a custom data library folder
    /// (e.g. 'libraries/custom'). MaterialX files at the root of this folder will be included
    /// in all content documents.
    void AddMaterialxLibrary(const std::string& mtlxLibrary);

    /// Specify the MDL language version the code generator should produce.
    void SetMdlVersion(MaterialX::GenMdlOptions::MdlVersion targetVersion);

    /// set the main mtlx file of the material to generate code from.
    bool SetSource(const std::string& mtlxMaterial, const std::string& materialName);

    /// generate MDL code
    bool Generate(mi::neuraylib::IMdl_configuration* mdlConfiguration, Result& inoutResult) const;

    /// flip v-coordinate for all texture lookups.
    /// this should only be needed in the MaterialXTest context where image space and texture coordinate
    /// do not align property. In general MDL and MaterialX use follow the same convention and flipping
    /// is not needed.
    void SetFileTextureVerticalFlip(bool flip);

private:
    std::vector<std::string> _mtlxSearchPaths;
    std::vector<std::string> _mtlxRelativeLibraryPaths;
    std::string _mtlxSource;
    std::string _mtlxMaterialName;
    MaterialX::GenMdlOptions::MdlVersion _mdlVersion;
    bool _fileTextureVerticalFlip;
};

#endif
