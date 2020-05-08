//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef GENGLSL_H
#define GENGLSL_H

#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

namespace mx = MaterialX;

class GlslShaderGeneratorTester : public GenShaderUtil::ShaderGeneratorTester
{
  public:
    using ParentClass = GenShaderUtil::ShaderGeneratorTester;

    GlslShaderGeneratorTester(mx::ShaderGeneratorPtr shaderGenerator, const mx::FilePathVec& testRootPaths, 
                              const mx::FilePath& libSearchPath, const mx::FileSearchPath& srcSearchPath, 
                              const mx::FilePath& logFilePath, bool writeShadersToDisk) :
        GenShaderUtil::ShaderGeneratorTester(shaderGenerator, testRootPaths, libSearchPath, srcSearchPath, logFilePath, writeShadersToDisk)
    {}

    void setTestStages() override
    {
        _testStages.push_back(mx::Stage::VERTEX);
        _testStages.push_back(mx::Stage::PIXEL);
    }

    void setupDependentLibraries() override
    {
        ParentClass::setupDependentLibraries();

        mx::FilePath lightDir = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/lights");
        loadLibrary(lightDir / mx::FilePath("light_compound_test.mtlx"), _dependLib);
        loadLibrary(lightDir / mx::FilePath("light_rig_test_1.mtlx"), _dependLib);
    }

  protected:
    void getImplementationWhiteList(mx::StringSet& whiteList) override
    {
        whiteList =
        {
            "ambientocclusion", "arrayappend", "backfacing", "screen", "curveadjust", "displacementshader",
            "volumeshader", "IM_constant_", "IM_dot_", "IM_geompropvalue", "IM_light_genglsl",
            "IM_point_light_genglsl", "IM_spot_light_genglsl", "IM_directional_light_genglsl", "IM_angle", 
            "surfacematerial", "volumematerial", "ND_surfacematerial", "ND_volumematerial" 
        };
    }
};

#endif // GENGLSL_H
