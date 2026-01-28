//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>
#include <MaterialXTest/MaterialXGenMdl/GenMdl.h>

#include <MaterialXRender/ShaderRenderer.h>
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
    void createRenderer(std::ostream& /*log*/) override { };

    bool runRenderer(const std::string& shaderName,
                     mx::TypedElementPtr element,
                     mx::GenContext& context,
                     mx::DocumentPtr doc,
                     std::ostream& log,
                     const GenShaderUtil::TestSuiteOptions& testOptions,
                     RenderUtil::RenderProfileTimes& profileTimes,
                     const mx::FileSearchPath& imageSearchPath,
                     const std::string& outputPath = ".",
                     mx::ImageVec* imageVec = nullptr) override;

    void addSkipFiles() override
    {
        // The command-line assembled in runRenderer() assumes that the simple name of the MDL
        // material is the same as "shaderName", but for this file the MDL material name gets a "1"
        // suffix. Communicating the generated name from the stage to the shader could help, but in
        // general it is probably better to give the main object a predictable name and use suffixes
        // for internal objects.
        _skipFiles.insert("translucent_bsdf.mtlx");
    }

    mx::DocumentPtr _last_doc;
};

// Renderer execution
bool MdlShaderRenderTester::runRenderer(const std::string& shaderName,
                                        mx::TypedElementPtr element,
                                        mx::GenContext& context,
                                        mx::DocumentPtr doc,
                                        std::ostream& log,
                                        const GenShaderUtil::TestSuiteOptions& testOptions,
                                        RenderUtil::RenderProfileTimes& profileTimes,
                                        const mx::FileSearchPath&,
                                        const std::string& outputPath,
                                        mx::ImageVec* /*imageVec*/)
{
    std::cout << "Validating MDL rendering for: " << doc->getSourceUri() << ", element: " << element->getNamePath() << std::endl;

    mx::ScopedTimer totalMDLTime(&profileTimes.languageTimes.totalTime);

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
            profileTimes.elementsTested++;

            mx::ShaderPtr shader;
            try
            {
                mx::ScopedTimer genTimer(&profileTimes.languageTimes.generationTime);
                mx::GenOptions& contextOptions = context.getOptions();
                contextOptions = options;
                contextOptions.targetColorSpaceOverride = "lin_rec709";
                contextOptions.fileTextureVerticalFlip = true;

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
                return false;
            }
            CHECK(shader->getSourceCode().length() > 0);

            std::string mdlCmdStr = shader->getSourceCode();

            std::string shaderPath;
            mx::FilePath outputFilePath = outputPath;
            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
            }

            // Note: mkdir will fail if the directory already exists which is ok.
            {
                mx::ScopedTimer ioDir(&profileTimes.languageTimes.ioTime);
                outputFilePath.createDirectory();
            }

            shaderPath = outputFilePath / mx::FilePath(shaderName);

            // Write out mdl file
            {
                mx::ScopedTimer ioTimer(&profileTimes.languageTimes.ioTime);
                std::ofstream file;
                file.open(shaderPath + ".rendermdl.mdl");
                file << shader->getSourceCode();
                file.close();
            }

            // Render (and validate)
            bool validated = false;
            try
            {
                // Executable
                std::string command(MATERIALX_MDL_BINARY_TESTRENDER);
                CHECK(!command.empty());

                // Avoid picking up by accident MDL support files for MaterialX shipped with the
                // example (the ones from this MaterialX installation should be used instead).
                command += " --nostdpath";

                // Set MDL search paths (same as for the resolver).
                for (const auto& sp : resolver->getMdlSearchPaths())
                {
                    command += " --mdl_path \"" + sp.asString() + "\"";
                }

                // Set MDL search path for the module itself.
                command += " --mdl_path \"" + outputPath + "\"";

                // Set environment
                mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
                mx::FilePath rootPath = searchPath.isEmpty() ? mx::FilePath() : searchPath[0];
                std::string iblFile = (rootPath / testOptions.radianceIBLPath).asString();
                command += " --hdr \"" + iblFile + "\"";
                command += " --hdr_rotate 90";
                command += " --background 0.073239 0.073239 0.083535";

                // Set scene
                command += " --camera 0 0 3 0 0 0";
                command += " --fov 45";
                command += " --materialxtest_mode"; // align texcoord space with OSL

                // Application setup
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
                command += " -o " + shaderPath + "_mdl" + ext;

                // Set the material
                std::string mdlMaterialName = "::" + shaderName + ".rendermdl::" + shaderName;
                command += " " + mdlMaterialName;

                // Redirect output
                std::string logFile(shaderPath + ".genmdl_render_log.txt");
                command += " >" + logFile + " 2>&1";

                // Run the renderer executable
                int returnValue = std::system(command.c_str());
                CHECK(returnValue == 0);

                // Log command and return code
                log << command << std::endl;
                log << "\tReturn code: " << std::to_string(returnValue) << std::endl;

                // Append output
                std::ifstream logStream(logFile);
                std::string line;
                while (std::getline(logStream, line))
                {
                    log << "\tLog: " << line << std::endl;
                }
                validated = true;

                // Remove output images for auxiliary buffers (not needed here, no error checking)
                std::string aux_buffers[]
                    = { "_albedo", "_normal", "_albedo_diffuse", "_albedo_glossy", "_roughness" };
                for (const auto& aux_buffer : aux_buffers)
                {
                    std::remove((shaderPath + "_mdl" + aux_buffer + ext).c_str());
                }
            }
            catch (mx::ExceptionRenderError& e)
            {
                for (const auto& error : e.errorLog())
                {
                    log << e.what() << " " << error << std::endl;
                }
                log << ">> Refer to shader code in dump file: " << shaderPath << ".rendermdl.mdl file" << std::endl;
            }
            catch (mx::Exception& e)
            {
                log << e.what() << "\n";
            }
            CHECK(validated);
        }
    }

    return true;
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
