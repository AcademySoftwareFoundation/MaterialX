//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef GENGLSL_UTIL_H
#define GENGLSL_UTIL_H

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenGlsl/GlslSyntax.h>

#include <MaterialXTest/GenShaderUtil.h>

namespace mx = MaterialX;

class GlslShaderGeneratorTester : public GenShaderUtil::ShaderGeneratorTester
{
  public:
    using ParentClass = GenShaderUtil::ShaderGeneratorTester;

    GlslShaderGeneratorTester(const mx::FilePath& testRootPath, const mx::FilePath& libSearchPath,
                              const mx::FileSearchPath& srcSearchPath, const mx::FilePath& logFilePath) :
        GenShaderUtil::ShaderGeneratorTester(testRootPath, libSearchPath, srcSearchPath, logFilePath)
    {}

    void createGenerator() override
    {
        _shaderGenerator = mx::GlslShaderGenerator::create();
    }

    void addSkipNodeDefs() override
    {
        _skipNodeDefs.insert("ND_add_surfaceshader");
        _skipNodeDefs.insert("ND_multiply_surfaceshaderF");
        _skipNodeDefs.insert("ND_multiply_surfaceshaderC");
        _skipNodeDefs.insert("ND_mix_surfaceshader");
    }

    void setTestStages() override
    {
        _testStages.push_back(mx::Stage::VERTEX);
        _testStages.push_back(mx::Stage::PIXEL);
    }

    void setupDependentLibraries() override
    {
        ParentClass::setupDependentLibraries();

        mx::FilePath lightDir = mx::FilePath::getCurrentPath() / mx::FilePath("resources/Materials/TestSuite/Utilities/Lights");
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath("lightcompoundtest.mtlx"), _dependLib);
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath("lightcompoundtest_ng.mtlx"), _dependLib);
        GenShaderUtil::loadLibrary(lightDir / mx::FilePath("light_rig.mtlx"), _dependLib);
    }
};

#endif // GENGLSL_UTIL_H
