//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef GENMDL_H
#define GENMDL_H

#include <memory>

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

namespace mx = MaterialX;

class MdlStringResolver;
using MdlStringResolverPtr = std::shared_ptr<MdlStringResolver>;

/// Resolve MaterialX file paths to MDL resource paths,
/// which is done in preprocessDocument before generator and renderer tests.
class MdlStringResolver : public mx::StringResolver
{
  public:
    /// Create a new string resolver.
    static MdlStringResolverPtr create();
    ~MdlStringResolver() = default;

    /// Setup the resolver for a given document
    void initialize(
        mx::DocumentPtr document,
        std::ostream* logFile,
        std::initializer_list<mx::FilePath> additionalSearchpaths);

    /// Given an input string and type, apply all appropriate modifiers and
    /// return the resulting string.
    std::string resolve(const std::string& str, const std::string& type) const override;

    /// Get the list of MDL search paths from which we can locate resources.
    const mx::FileSearchPath& getMdlSearchPaths() const { return _mdl_searchPaths; }

  private:
    /// search paths computed during `initialize`
    mx::FileSearchPath _mdl_searchPaths;

    /// log stream of the tester
    std::ostream* _logFile;
};


class MdlShaderGeneratorTester : public GenShaderUtil::ShaderGeneratorTester
{
  public:
    using ParentClass = GenShaderUtil::ShaderGeneratorTester;

    MdlShaderGeneratorTester(mx::ShaderGeneratorPtr shaderGenerator, const std::vector<mx::FilePath>& testRootPaths,
                             const mx::FileSearchPath& searchPath, const mx::FilePath& logFilePath, bool writeShadersToDisk) :
        GenShaderUtil::ShaderGeneratorTester(shaderGenerator, testRootPaths, searchPath, logFilePath, writeShadersToDisk)
    {}

    void setTestStages() override
    {
        _testStages.push_back(mx::Stage::PIXEL);
    }

    // Ignore trying to create shader code for the following nodedefs
    void addSkipNodeDefs() override
    {
        _skipNodeDefs.insert("ND_point_light");
        _skipNodeDefs.insert("ND_spot_light");
        _skipNodeDefs.insert("ND_directional_light");
        _skipNodeDefs.insert("ND_dot_");
        ParentClass::addSkipNodeDefs();
    }

    // Ignore certain .mtlx files
    void addSkipFiles() override
    {
        // no additional files are skipped
        ShaderGeneratorTester::addSkipFiles();
    }

    // Ignore light shaders in the document for MDL
    void findLights(mx::DocumentPtr /*doc*/, std::vector<mx::NodePtr>& lights) override
    {
        lights.clear();
    }

    // No direct lighting to register for MDL
    void registerLights(mx::DocumentPtr /*doc*/, const std::vector<mx::NodePtr>& /*lights*/, mx::GenContext& /*context*/) override
    {
    }

    // Allows the tester to alter the document, e.g., by flattering file names
    void preprocessDocument(mx::DocumentPtr doc) override;

    // Compile MDL with mdlc if specified
    void compileSource(const std::vector<mx::FilePath>& sourceCodePaths) override;

  protected:
    void getImplementationWhiteList(mx::StringSet& whiteList) override
    {
        whiteList =
        {
            "displacementshader", "volumeshader", "surfacematerial", "volumematerial", "geompropvalue",
            "IM_constant_", "IM_dot_", "IM_angle", "IM_geomattrvalue",
            "IM_absorption_vdf_", "IM_mix_vdf_", "IM_add_vdf_", "IM_multiply_vdf",
            "IM_measured_edf_", "IM_blackbody_", "IM_conical_edf_",
            "IM_displacement_", "IM_volume_", "IM_light_"
        };
        ShaderGeneratorTester::getImplementationWhiteList(whiteList);
    }

    MdlStringResolverPtr _mdlCustomResolver;
};

#endif // GENMDL_H
