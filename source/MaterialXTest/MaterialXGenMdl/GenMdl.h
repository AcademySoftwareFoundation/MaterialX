//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef GENMDL_H
#define GENMDL_H

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

namespace mx = MaterialX;

class MdlShaderGeneratorTester : public GenShaderUtil::ShaderGeneratorTester
{
  public:
    using ParentClass = GenShaderUtil::ShaderGeneratorTester;

    MdlShaderGeneratorTester(mx::ShaderGeneratorPtr shaderGenerator, const std::vector<mx::FilePath>& testRootPaths,
                             const mx::FilePath& libSearchPath, const mx::FileSearchPath& srcSearchPath,
                             const mx::FilePath& logFilePath, bool writeShadersToDisk) :
        GenShaderUtil::ShaderGeneratorTester(shaderGenerator, testRootPaths, libSearchPath, srcSearchPath, logFilePath, writeShadersToDisk)
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

    // Ignore files using derivatives
    void addSkipFiles() override
    {
        std::string renderExec(MATERIALX_MDL_RENDER_EXECUTABLE);
        if (std::string::npos != renderExec.find("df_cuda"))
        {
            // df_cuda will currently hang on rendering one of the shaders in this file
            _skipFiles.insert("heighttonormal_in_nodegraph.mtlx");
        }
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

    // Compile MDL with mdlc if specified
    void compileSource(const std::vector<mx::FilePath>& sourceCodePaths) override;

  protected:
    void getImplementationWhiteList(mx::StringSet& whiteList) override
    {
        whiteList =
        {
            "ambientocclusion", "arrayappend", "backfacing", "screen", "curveadjust", "displacementshader",
            "volumeshader", "IM_constant_", "IM_dot_", "IM_geomattrvalue", "IM_angle",
            "geompropvalue", "surfacematerial", "volumematerial", "IM_absorption_vdf_", "IM_mix_vdf_",
            "IM_measured_edf_", "IM_blackbody_", "IM_conical_edf_", 
            "IM_displacement_", "IM_thin_surface_", "IM_volume_", "IM_light_"
        };
        ShaderGeneratorTester::getImplementationWhiteList(whiteList);
    }
};

#endif // GENOSL_H
