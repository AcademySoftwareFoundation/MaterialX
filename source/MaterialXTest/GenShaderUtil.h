//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef GENSHADER_UTIL_H
#define GENSHADER_UTIL_H

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/File.h>
#include <MaterialXFormat/XmlIo.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/TypeDesc.h>
#include <MaterialXGenShader/UnitSystem.h>
#include <MaterialXGenShader/Util.h>

#include <cstdlib>
#include <fstream>
#include <iostream>

namespace mx = MaterialX;

namespace GenShaderUtil
{
    
/// An unordered map from light names to light indices.
using LightIdMap = std::unordered_map<std::string, unsigned int>;

//
// Get source content, source path and resolved paths for
// an implementation
//
bool getShaderSource(mx::GenContext& context, 
                        const mx::ImplementationPtr implementation,
                        mx::FilePath& sourcePath,
                        mx::FilePath& resolvedPath,
                        std::string& sourceContents);

// Test code generation for a given element
bool generateCode(mx::GenContext& context, const std::string& shaderName, mx::TypedElementPtr element,
                    std::ostream& log, mx::StringVec testStages, mx::StringVec& sourceCode);

// Check that implementations exist for all nodedefs supported per generator
void checkImplementations(mx::GenContext& context,
                            const mx::StringSet& generatorSkipNodeTypes,
                            const mx::StringSet& generatorSkipNodeDefs,
                            unsigned int expectedSkipCount);

// Utility test to  check unique name generation on a shader generator
void testUniqueNames(mx::GenContext& context, const std::string& stage);

//
// Render validation options. Reflects the _options.mtlx
// file in the test suite area.
//
class TestSuiteOptions
{
  public:
    // Print out options
    void print(std::ostream& output) const;

    // Option options from an options file
    bool readOptions(const std::string& optionFile);

    // Filter list of files to only run validation on.
    mx::StringVec overrideFiles;

    // List of language,target pair identifier storage as
    // strings in the form <language>_<target>.
    mx::StringSet languageAndTargets;

    // Comma separated list of light setup files
    mx::StringVec lightFiles;

    // Set to true to always dump generated code to disk
    bool dumpGeneratedCode = false;

    // Check the count of number of implementations used
    bool checkImplCount = true;

    // Run using a set of interfaces:
    // - 3 = run complete + reduced.
    // - 2 = run complete only (default)
    // - 1 = run reduced only.
    int shaderInterfaces = 2;

    // Validate element before attempting to generate code. Default is false.
    bool validateElementToRender = false;

    // Perform source code compilation validation test
    bool compileCode = true;

    // Perform rendering validation test
    bool renderImages = true;

    // Perform saving of image.
    bool saveImages = true;

    // Set this to be true if it is desired to dump out uniform and attribut information to the logging file.
    bool dumpUniformsAndAttributes = true;

    // Non-shaded geometry file
    MaterialX::FilePath unShadedGeometry;

    // Shaded geometry file
    MaterialX::FilePath shadedGeometry;

    // Amount to scale geometry. 
    float geometryScale;

    // Enable direct lighting. Default is true. 
    bool enableDirectLighting;

    // Enable indirect lighting. Default is true. 
    bool enableIndirectLighting;

    // Method for specular environment sampling (only used for HW rendering):
    //   0 : Prefiltered - Use a radiance IBL texture that has been prefiltered with the BRDF.
    //   1 : Filtered Importance Sampling - Use FIS to sample the IBL texture according to the BRDF in runtime.
    // Default value is 1.
    int specularEnvironmentMethod;

    // Radiance IBL file.
    mx::FilePath radianceIBLPath;

    // Irradiance IBL file.
    mx::FilePath irradianceIBLPath;

    // Transforms UVs of loaded geometry
    MaterialX::Matrix44 transformUVs;

    // Additional library paths
    mx::FileSearchPath externalLibraryPaths;

    // Additional testPaths paths
    mx::FileSearchPath externalTestPaths;

    // Desired major version to test
    int desiredMajorVersion;

    // Desired minor version to test
    int desiredMinorVersion;
};

// Utility class to handle testing of shader generators.
// Currently only tests source code generation.
class ShaderGeneratorTester
{
  public:
    ShaderGeneratorTester(mx::ShaderGeneratorPtr shaderGenerator, const mx::FilePathVec& testRootPaths, 
                            const mx::FilePath& libSearchPath, const mx::FileSearchPath& srcSearchPath, 
                            const mx::FilePath& logFilePath) :
        _shaderGenerator(shaderGenerator),
        _languageTargetString(shaderGenerator ? (shaderGenerator->getLanguage() + "_" + shaderGenerator->getTarget()) : "NULL"),
        _testRootPaths(testRootPaths),
        _libSearchPath(libSearchPath),
        _srcSearchPath(srcSearchPath),
        _logFilePath(logFilePath)
    {
    }

    ~ShaderGeneratorTester()
    {
    }

    // Check if testing should be performed based in input options
    virtual bool runTest(const TestSuiteOptions& testOptions)
    {
        return (testOptions.languageAndTargets.count(_languageTargetString) > 0);
    }

    // Stages to test is required from derived class
    virtual void setTestStages() = 0;

    // Add files in to not examine
    virtual void addSkipFiles();

    // Add nodedefs to not examine
    virtual void addSkipNodeDefs();

    // Add files to be skipped while loading libraries
    virtual void addSkipLibraryFiles();

    // Add color management
    virtual void addColorManagement();

    // Add unit system
    virtual void addUnitSystem();

    // Load in dependent libraries
    virtual void setupDependentLibraries();

    // TODO: Merge the methods below with equivalent methods in LightHandler.

    // From a set of nodes, create a mapping of nodedef identifiers to numbers
    virtual LightIdMap computeLightIdMap(const std::vector<mx::NodePtr>& nodes);

    // Find lights to use based on an input document
    virtual void findLights(mx::DocumentPtr doc, std::vector<mx::NodePtr>& lights);

    // Register light node definitions and light count with a given generation context
    virtual void registerLights(mx::DocumentPtr doc, const std::vector<mx::NodePtr>& lights, mx::GenContext& context);

    // Generate source code for a given element and check that code was produced.
    virtual bool generateCode(mx::GenContext& context, const std::string& shaderName, mx::TypedElementPtr element,
                              std::ostream& log, mx::StringVec testStages, mx::StringVec& sourceCode);

    // Run test for source code generation
    void validate(const mx::GenOptions& generateOptions, const std::string& optionsFilePath);

  protected:
    // Check to see that all implementations have been tested for a given
    // language.
    void checkImplementationUsage(const mx::StringSet& usedImpls,
                                    const mx::GenContext& context,
                                    std::ostream& stream);

    // Get implementation "whitelist" for those implementations that have
    // been skipped for checking
    virtual void getImplementationWhiteList(mx::StringSet& whiteList) = 0;

    mx::ShaderGeneratorPtr _shaderGenerator;
    const std::string _languageTargetString;
    mx::DefaultColorManagementSystemPtr _colorManagementSystem;

    // Unit system 
    mx::UnitSystemPtr _unitSystem;
    std::string _defaultDistanceUnit;

    mx::DocumentPtr _dependLib;

    const mx::FilePathVec _testRootPaths;
    const mx::FilePath _libSearchPath;
    const mx::FileSearchPath _srcSearchPath;
    const mx::FilePath _logFilePath;

    mx::StringSet _skipFiles;
    mx::StringSet _skipLibraryFiles;
    std::vector<mx::DocumentPtr> _documents;
    mx::StringVec _documentPaths;
    std::ofstream _logFile;
    mx::StringSet _skipNodeDefs;
    mx::StringVec _testStages;

    std::vector<mx::NodePtr> _lights;
    std::unordered_map<std::string, unsigned int> _lightIdentifierMap;

    mx::StringSet _usedImplementations;
};



} // namespace GenShaderUtil

#endif
