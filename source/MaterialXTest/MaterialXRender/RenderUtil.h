//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef RENDER_UTIL_H
#define RENDER_UTIL_H

#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

#include <MaterialXCore/Document.h>

#include <MaterialXFormat/XmlIo.h>

#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/UnitSystem.h>

#include <MaterialXRender/Util.h>
#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/ImageHandler.h>

#include <chrono>
#include <ctime>

#define LOG_TO_FILE

namespace mx = MaterialX;

// Utilities for running render tests.
//
// Execution uses existing code generator instances to produce the code and corresponding renderer
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
    ShaderRenderTester(mx::ShaderGeneratorPtr shaderGenerator);
    virtual ~ShaderRenderTester();

    bool validate(const mx::FilePathVec& testRootPaths, const mx::FilePath optionsFilePath);

  protected:
    // Check if testing should be performed based in input options
#if defined(MATERIALX_TEST_RENDER)
    virtual bool runTest(const GenShaderUtil::TestSuiteOptions& testOptions)
    {
        return (testOptions.targets.count(_shaderGenerator->getTarget()) > 0);
    }
#else
    virtual bool runTest(const GenShaderUtil::TestSuiteOptions& /*testOptions*/)
    {
        return false;
    }
#endif

    // Add files to skip
    void addSkipFiles()
    {
        _skipFiles.insert("_options.mtlx");
        _skipFiles.insert("light_rig_test_1.mtlx");
        _skipFiles.insert("light_rig_test_2.mtlx");
        _skipFiles.insert("light_compound_test.mtlx");
    }

    // Load dependencies
    void loadDependentLibraries(GenShaderUtil::TestSuiteOptions options, mx::FileSearchPath searchPath,
                             mx::DocumentPtr& dependLib);

    // Load any additional libraries requird by the generator
    virtual void loadAdditionalLibraries(mx::DocumentPtr /*dependLib*/,
                                         GenShaderUtil::TestSuiteOptions& /*options*/) {};

    //
    // Code generation methods
    //

    // Register any additional source code paths used by the generator
    virtual void registerSourceCodeSearchPaths(mx::GenContext& /*context*/) {};

    // Register any lights used by the generation context
    virtual void registerLights(mx::DocumentPtr /*dependLib*/,
                                const GenShaderUtil::TestSuiteOptions &/*options*/,
                                mx::GenContext& /*context*/) {};

    //
    // Code validation methods (compile and render)
    //

    // Create a renderer for the generated code
    virtual void createRenderer(std::ostream& log) = 0;

    // Run the renderer
    virtual bool runRenderer(const std::string& shaderName,
        mx::TypedElementPtr element,
        mx::GenContext& context,
        mx::DocumentPtr doc,
        std::ostream& log,
        const GenShaderUtil::TestSuiteOptions& testOptions,
        RenderUtil::RenderProfileTimes& profileTimes,
        const mx::FileSearchPath& imageSearchPath,
        const std::string& outputPath = ".",
        mx::ImageVec* imageVec = nullptr) = 0;

    // Save an image
    virtual bool saveImage(const mx::FilePath&, mx::ConstImagePtr, bool) const { return false;  };

    // Create a list of generation options based on unit test options
    // These options will override the original generation context options.
    void getGenerationOptions(const GenShaderUtil::TestSuiteOptions& testOptions,
                              const mx::GenOptions& originalOptions,
                              std::vector<mx::GenOptions>& optionsList);

    // Print execution summary
    void printRunLog(const RenderProfileTimes &profileTimes,
                     const GenShaderUtil::TestSuiteOptions& options,
                     std::ostream& stream,
                     mx::DocumentPtr dependLib);

    virtual bool canBake() const { return false; }
    virtual void runBake(mx::DocumentPtr /*doc*/, const mx::FileSearchPath& /*imageSearchPath*/, const mx::FilePath& /*outputFilename*/,
                         unsigned int /*bakeWidth*/, unsigned int /*bakeHeight*/, bool /*bakeHdr*/, std::ostream& /*log*/) {};

    // Generator to use
    mx::ShaderGeneratorPtr _shaderGenerator;

    // Files to skip
    mx::StringSet _skipFiles;
};

} // namespace RenderUtil

#endif
