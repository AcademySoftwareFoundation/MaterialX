//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef GENOSL_H
#define GENOSL_H

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXGenOsl/OslShaderGenerator.h>

#include <MaterialXTest/GenShaderUtil.h>

namespace mx = MaterialX;

class OslShaderGeneratorTester : public GenShaderUtil::ShaderGeneratorTester
{
  public:
    using ParentClass = GenShaderUtil::ShaderGeneratorTester;

    OslShaderGeneratorTester(mx::ShaderGeneratorPtr shaderGenerator, const std::vector<mx::FilePath>& testRootPaths,
                             const mx::FilePath& libSearchPath, const mx::FileSearchPath& srcSearchPath,
                             const mx::FilePath& logFilePath) :
        GenShaderUtil::ShaderGeneratorTester(shaderGenerator, testRootPaths, libSearchPath, srcSearchPath, logFilePath)
    {}

    void setTestStages() override
    {
        _testStages.push_back(mx::Stage::PIXEL);
    }

    // Ignore trying to create shader code for lightshaders
    void addSkipNodeDefs() override
    {
        _skipNodeDefs.insert("ND_point_light");
        _skipNodeDefs.insert("ND_spot_light");
        _skipNodeDefs.insert("ND_directional_light");
        ParentClass::addSkipNodeDefs();
    }

    // Ignore light shaders in the document for OSL
    void findLights(mx::DocumentPtr /*doc*/, std::vector<mx::NodePtr>& lights) override
    {
        lights.clear();
    }

    // No direct lighting to register for OSL
    void registerLights(mx::DocumentPtr /*doc*/, const std::vector<mx::NodePtr>& /*lights*/, mx::GenContext& /*context*/) override
    {
    }

  protected:
    void getImplementationWhiteList(mx::StringSet& whiteList) override
    {
        whiteList =
        {
            "ambientocclusion", "arrayappend", "backfacing", "screen", "curveadjust", "displacementshader",
            "volumeshader", "IM_constant_", "IM_dot_", "IM_geomattrvalue", "IM_length_"
        };
    }
};

#endif // GENOSL_H
