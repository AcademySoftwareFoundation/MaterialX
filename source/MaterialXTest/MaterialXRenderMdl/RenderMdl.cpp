//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>
#include <MaterialXTest/MaterialXGenMdl/GenMdl.h>

#include <MaterialXRender/ShaderRenderer.h>
#include <MaterialXRender/Util.h>
#include <MaterialXRender/StbImageLoader.h>
#if defined(MATERIALX_BUILD_OIIO)
    #include <MaterialXRender/OiioImageLoader.h>
#endif

#include <MaterialXGenMdl/MdlShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>

#include <MaterialXFormat/Util.h>

namespace mx = MaterialX;

class MdlShaderRenderTester : public RenderUtil::ShaderRenderTester
{
  public:
    explicit MdlShaderRenderTester(mx::ShaderGeneratorPtr shaderGenerator) :
        RenderUtil::ShaderRenderTester(shaderGenerator)
    {
    }

  protected:
    void setTargetGenerationOptions(mx::GenOptions& /*options*/) override
    {
        // No texture coordinate flips are required for MDL, since MDL and
        // MaterialX define texture space equally, with the origin at the
        // lower left.
    }

    void createRenderer(std::ostream& /*log*/) override { };

    unsigned int getRenderWorkerCount(const GenShaderUtil::TestSuiteOptions& options) const override
    {
        return static_cast<unsigned int>(options.mdlRenderWorkers);
    }

    RenderUtil::RenderProfileResult runRenderer(
        const RenderUtil::RenderSession& session,
        const RenderUtil::RenderItem& item,
        mx::GenContext& context) override;

    mx::DocumentPtr _last_doc;
};

// Renderer execution
RenderUtil::RenderProfileResult MdlShaderRenderTester::runRenderer(
    const RenderUtil::RenderSession& session,
    const RenderUtil::RenderItem& item,
    mx::GenContext& context)
{
    RenderUtil::RenderProfileResult result;
    const std::string& shaderName = item.shaderName;
    mx::DocumentPtr doc = item.document;
    mx::TypedElementPtr element = item.element;
    const GenShaderUtil::TestSuiteOptions& testOptions = session.testOptions;
    std::ostream& log = session.log;

    std::cout << "Validating MDL rendering for: " << doc->getSourceUri() << ", element: " << element->getNamePath() << std::endl;


    mx::ShaderGenerator& shadergen = context.getShaderGenerator();

    std::vector<mx::GenOptions> optionsList;
    getGenerationOptions(testOptions, context.getOptions(), optionsList);

    if (element && doc)
    {
        log << "------------ Run MDL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        // Flatten resource paths. Invoking the custom resolver must happen only once per document.
        MdlStringResolverPtr resolver(new MdlStringResolver());
        resolver->initialize(doc, &log, {});
        if (doc != _last_doc)
        {
            mx::flattenFilenames(doc, resolver->getMdlSearchPaths(), resolver);
            _last_doc = doc;
        }

        for (const auto& options : optionsList)
        {
            result.elementsTested++;

            mx::ShaderPtr shader;
            try
            {
                mx::ScopedTimer genTimer(&result.languageTimes.generationTime);
                context.getOptions() = options;

                // Specify the MDL target version to be the latest which is also the default.
                mx::GenMdlOptionsPtr genMdlOptions = std::make_shared<mx::GenMdlOptions>();
                genMdlOptions->targetVersion = mx::GenMdlOptions::MdlVersion::MDL_LATEST;
                context.pushUserData(mx::GenMdlOptions::GEN_CONTEXT_USER_DATA_KEY, genMdlOptions);

                shader = shadergen.generate(shaderName, element, context);
            }
            catch (mx::Exception& e)
            {
                log << ">> " << e.what() << "\n";
                shader = nullptr;
            }
            CHECK(shader != nullptr);
            if (shader == nullptr)
            {
                log << ">> Failed to generate shader\n";
                result.success = false;
                return result;
            }
            std::string source = shader->getSourceCode();
            CHECK(!source.empty());
            if (source.empty())
            {
                result.success = false;
                continue;
            }

            std::string shaderPath;
            mx::FilePath outputFilePath = item.outputPath;
            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
            }

            shaderPath = (outputFilePath / shaderName).asString();
            try
            {
                std::string executable(MATERIALX_MDL_BINARY_TESTRENDER);
                if (executable.empty())
                    throw mx::ExceptionRenderError("MDL renderer executable is not configured");
                std::string command = mx::quoteCommandArgument(executable);

                // Avoid picking up by accident MDL support files for MaterialX shipped with the
                // example (the ones from this MaterialX installation should be used instead).
                command += " --nostdpath";

                // Set MDL search paths (same as for the resolver).
                for (const auto& sp : resolver->getMdlSearchPaths())
                {
                    command += " --mdl_path " + mx::quoteCommandArgument(sp.asString());
                }

                // Set MDL search path for the module itself (directory containing the .mdl file).
                command += " --mdl_path " + mx::quoteCommandArgument(outputFilePath.asString());

                // Set environment
                mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
                mx::FilePath rootPath = searchPath.isEmpty() ? mx::FilePath() : searchPath[0];
                std::string iblFile = (rootPath / testOptions.radianceIBLPath).asString();
                command += " --hdr " + mx::quoteCommandArgument(iblFile);
                command += " --hdr_rotate 90";
                command += " --background 0.073239 0.073239 0.083535";

                // Set scene
                command += " --camera 0 0 3 0 0 0";
                command += " --fov 45";

                // Since no scene geometry is specified, df_vulkan renders its
                // built-in sphere, whose texture coordinates span (0, 0) to
                // (2, 1).  Halve its U range to align it with the sphere
                // geometry used by other renderers in the test suite.  The
                // default V orientation of df_vulkan matches the MaterialX
                // convention, with the origin at the lower left.
                command += " --uv_scale 0.5 1";

                // Application setup
                command += " --noaux";
                command += " --nocc";
                command += " --no_shader_opt";      // does not pay off for the testsuite
                command += " --no_window";
                command += " --warning";            // filter info messages from the log
                command += " --res 512 512";
                command += " --max_path_length 6";
                command += " --max_sss_steps 256";
                command += " --spp " + std::to_string(testOptions.enableReferenceQuality ? 1024 : 256);

                // Set the output image file
#if defined(MATERIALX_BUILD_OIIO)
                std::string ext = ".exr";
#else
                std::string ext = ".png";
#endif
                mx::FilePath imagePath = shaderPath + "_mdl" + ext;
                command += " -o " + mx::quoteCommandArgument(imagePath.asString());

                // Set the material
                std::string mdlMaterialName = "::" + shaderName + ".rendermdl::" + shaderName;
                command += " " + mx::quoteCommandArgument(mdlMaterialName);

                // Redirect output
                std::string logFile(shaderPath + ".genmdl_render_log.txt");
                command += " >" + mx::quoteCommandArgument(logFile) + " 2>&1";

                std::string label = doc->getSourceUri() + ": " + element->getNamePath() +
                    (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED ? " [reduced]" : " [complete]");
                session.jobs.submit(label, shaderPath,
                    [command, source, shaderPath, outputFilePath, imagePath, logFile](RenderUtil::RenderJobResult& job) {
                        job.imagePath = imagePath;
                        outputFilePath.createDirectory(true);
                        std::ofstream file(shaderPath + ".rendermdl.mdl");
                        file << source;
                        file.close();
                        if (!file)
                            throw mx::ExceptionRenderError("Cannot write MDL source: " + shaderPath);
                        mx::removeStaleFile(imagePath);
                        int returnValue;
                        {
                            mx::ScopedTimer renderTimer(&job.renderTime);
                            returnValue = std::system(command.c_str());
                        }
                        job.log = command + "\nReturn code: " + std::to_string(returnValue) + "\n";
                        std::ifstream logStream(logFile);
                        job.log.append(std::istreambuf_iterator<char>(logStream), std::istreambuf_iterator<char>());
                        job.success = returnValue == 0;
                    });
            }
            catch (std::exception& e)
            {
                log << e.what() << std::endl;
                FAIL_CHECK(e.what());
                result.success = false;
            }
        }
    }

    result.languageTimes.totalTime = result.languageTimes.generationTime + result.languageTimes.ioTime;
    return result;
}

TEST_CASE("Render: MDL TestSuite", "[rendermdl]")
{
    mx::FilePath renderExec(MATERIALX_MDL_BINARY_TESTRENDER);
    if (!renderExec.exists())
    {
        INFO("Skipping the MDL test suite as its executable locations haven't been set.");
        return;
    }

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath optionsFilePath = searchPath.find("resources/Materials/TestSuite/_options.mtlx");

    MdlShaderRenderTester renderTester(mx::MdlShaderGenerator::create());
    renderTester.validate(optionsFilePath);
}
