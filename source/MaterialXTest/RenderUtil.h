//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef RENDER_UTIL_H
#define RENDER_UTIL_H

#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>

#include <MaterialXRender/Util.h>
#include <MaterialXRender/LightHandler.h>

#include <MaterialXTest/GenShaderUtil.h>

#include <chrono>
#include <ctime>

#define LOG_TO_FILE

namespace mx = MaterialX;

// Utilities for running render tests.
//
// Execution uses existing code generator instances to produce the code and corresponding validator
// instance to check the validity of the generated code by compiling and / or rendering
// the code to produce images on disk.
// 
// Input uniform and stream checking as well as node implementation coverage and profiling
// can also be performed depending on the options enabled.
//
// See the test suite file "_options.mtlx" which is parsed during validaiton to
// restrive validation options.
//
namespace RenderUtil
{
void createLightRig(mx::DocumentPtr doc, mx::LightHandler& lightHandler, mx::GenContext& context,
    const mx::FilePath& envIrradiancePath, const mx::FilePath& envRadiancePath);

//
// Render validation options. Reflects the _options.mtlx
// file in the test suite area.
//
class RenderTestOptions
{
  public:
    // Print out options
    void print(std::ostream& output) const;

    // Option options from an options file
    bool readOptions(const std::string& optionFile);

    // Filter list of files to only run validation on.
    MaterialX::StringVec overrideFiles;

    // List of language,target pair identifier storage as 
    // strings in the form <language>_<target>.
    MaterialX::StringSet languageAndTargets;

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

    // Radiance IBL file
    MaterialX::FilePath radianceIBLPath;

    // IradianceIBL file
    MaterialX::FilePath irradianceIBLPath;
};

// Scoped timer which adds a duration to a given externally reference timing duration
//
class AdditiveScopedTimer
{
  public:
    AdditiveScopedTimer(double& durationRefence, const std::string& label)
        : _duration(durationRefence)
        , _debugUpdate(false)
        , _label(label)
    {
        startTimer();
    }

    ~AdditiveScopedTimer()
    {
        endTimer();
    }

    void startTimer()
    {
        _startTime = std::chrono::system_clock::now();

        if (_debugUpdate)
        {
            std::cout << "Start time for timer (" << _label << ") is: " << _duration << std::endl;
        }
    }

    void endTimer()
    {
        std::chrono::time_point<std::chrono::system_clock> endTime = std::chrono::system_clock::now();
        std::chrono::duration<double> timeDuration = endTime - _startTime;
        double currentDuration = timeDuration.count();
        _duration += currentDuration;
        _startTime = endTime;

        if (_debugUpdate)
        {
            std::cout << "Current duration for timer (" << _label << ") is: " << currentDuration << ". Total duration: " << _duration << std::endl;
        }
    }

  protected:
    double &_duration;
    bool _debugUpdate;
    std::string _label;
    std::chrono::time_point<std::chrono::system_clock> _startTime;
};


// Per language profile times
//
class LanguageProfileTimes
{
  public:
    void print(const std::string& label, std::ostream& output) const
    {
        output << label << std::endl;
        output << "\tTotal: " << totalTime << " seconds" << std::endl;;
        output << "\tSetup: " << setupTime << " seconds" << std::endl;;
        output << "\tTransparency: " << transparencyTime << " seconds" << std::endl;;
        output << "\tGeneration: " << generationTime << " seconds" << std::endl;;
        output << "\tCompile: " << compileTime << " seconds" << std::endl;
        output << "\tRender: " << renderTime << " seconds" << std::endl;
        output << "\tI/O: " << ioTime << " seconds" << std::endl;
        output << "\tImage save: " << imageSaveTime << " seconds" << std::endl;
    }
    double totalTime = 0.0;
    double setupTime = 0.0;
    double transparencyTime = 0.0;
    double generationTime = 0.0;
    double compileTime = 0.0;
    double renderTime = 0.0;
    double ioTime = 0.0;
    double imageSaveTime = 0.0;
};

// Render validation profiling structure
//
class RenderProfileTimes
{
  public:
    void print(std::ostream& output) const
    {
        output << "Overall time: " << languageTimes.totalTime << " seconds" << std::endl;
        output << "\tI/O time: " << ioTime << " seconds" << std::endl;
        output << "\tValidation time: " << validateTime << " seconds" << std::endl;
        output << "\tRenderable search time: " << renderableSearchTime << " seconds" << std::endl;

        languageTimes.print("Profile Times:", output);

        output << "Elements tested: " << elementsTested << std::endl;
    }

    LanguageProfileTimes languageTimes;
    double totalTime = 0;
    double ioTime = 0.0;
    double validateTime = 0.0;
    double renderableSearchTime = 0.0;
    unsigned int elementsTested = 0;
};

// Base class used for performing compilation and render tests for a given
// shading language and target.
//
class ShaderRenderTester
{
  public:
    ShaderRenderTester() {};
    virtual ~ShaderRenderTester() {};
    bool validate();

  protected:
    // The shading language / target being tested
    virtual const std::string& languageTargetString() = 0;

    // Check if testing should be performed based in input options
    virtual bool runTest(const RenderUtil::RenderTestOptions& testOptions) const = 0;

    // Load any additional libraries requird by the generator
    virtual void loadLibraries(mx::DocumentPtr /*dependLib*/,
                               RenderUtil::RenderTestOptions& /*options*/) {};

    //
    // Code generation methods
    //
    // Create the appropirate code generator for the language/target
    virtual void createShaderGenerator() = 0;

    // Register any additional source code paths used by the generator
    virtual void registerSourceCodeSearchPaths(mx::GenContext& /*context*/) {};
    
    // Register any lights used by the generation context
    virtual void registerLights(mx::DocumentPtr /*dependLib*/, 
                                const RenderUtil::RenderTestOptions &/*options*/,
                                mx::GenContext& /*context*/) {};

    //
    // Code validation methods (compile and render)
    //
    // Create a validator for the generated code
    virtual void createValidator(std::ostream& log) = 0;
    
    // Run the validator
    virtual bool runValidator(const std::string& shaderName,
        mx::TypedElementPtr element,
        mx::GenContext& context,
        mx::DocumentPtr doc,
        std::ostream& log,
        const RenderUtil::RenderTestOptions& testOptions,
        RenderUtil::RenderProfileTimes& profileTimes,
        const mx::FileSearchPath& imageSearchPath,
        const std::string& outputPath = ".") = 0;

    // Create a list of generation options based on unit test options
    // These options will override the original generation context options.
    void getGenerationOptions(const RenderTestOptions& testOptions,
                              const mx::GenOptions& originalOptions,
                              std::vector<mx::GenOptions>& optionsList);

    // Get implemenation "whitelist" for those implementations that have
    // been skipped for checking
    virtual void getImplementationWhiteList(mx::StringSet& whiteList) = 0;

    // Check to see that all implemenations have been tested for a given
    // lanuage.
    void checkImplementationUsage(const std::string& language,
                                  mx::StringSet& usedImpls,
                                  mx::DocumentPtr dependLib,
                                  mx::GenContext& context,
                                  std::ostream& stream);

    // Print execution summary 
    void printRunLog(const RenderProfileTimes &profileTimes,
                     const RenderTestOptions& options,
                     mx::StringSet& usedImpls,
                     std::ostream& stream,
                     mx::DocumentPtr dependLib,
                     mx::GenContext& context,
                     const std::string& language);

    // Generator to use
    mx::ShaderGeneratorPtr _shaderGenerator;
};

} // namespace RenderUtil

#endif

