//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>
#include <MaterialXTest/MaterialXGenMdl/GenMdl.h>

#include <MaterialXRender/StbImageLoader.h>
#if defined(MATERIALX_BUILD_OIIO)
    #include <MaterialXRender/OiioImageLoader.h>
#endif

#include <MaterialXGenMdl/MdlShaderGenerator.h>

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
    void createRenderer(std::ostream& log) override;

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
};

// Renderer setup
void MdlShaderRenderTester::createRenderer(std::ostream& log)
{
    mx::FilePath renderExec(MATERIALX_MDL_RENDER_EXECUTABLE);

    if (!renderExec.exists())
    {
        log << "MDL renderer executable not set: 'MATERIALX_MDL_RENDER_EXECUTABLE'" << std::endl;
        log << "For running the tests, either set 'MATERIALX_MDL_RENDER_EXECUTABLE' or 'MATERIALX_BUILD_MDL_RENDER" << std::endl;
        REQUIRE((false && "MDL renderer executable not set'"));
        return;
    }
}

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
    std::cout << "Validating MDL rendering for: " << doc->getSourceUri() << " element: " << element->getNamePath() << std::endl;
    mx::ScopedTimer totalMDLTime(&profileTimes.languageTimes.totalTime);

    std::vector<mx::GenOptions> optionsList;
    getGenerationOptions(testOptions, context.getOptions(), optionsList);

    if (element && doc)
    {
        log << "------------ Run MDL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (const auto& options : optionsList)
        {
            profileTimes.elementsTested++;

            // prepare the output folder
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
            std::string shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);

            try
            {
                // executable
                std::string renderCommand(MATERIALX_MDL_RENDER_EXECUTABLE);
                CHECK(!renderCommand.empty());

                // Set MaterialX search paths
                mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
                mx::FilePath rootPath = searchPath.isEmpty() ? mx::FilePath() : searchPath[0];
                renderCommand += " --mtlx_path \"" + rootPath.asString() + "\"";

                // Set MDL search paths
                // Use the same resolver as in the generator tests.
                // Here, we don't actually resolve but instead only use the MDL search paths.
                MdlStringResolver resolver;
                resolver.initialize(doc, &log, {});
                for (const auto& sp : resolver.getMdlSearchPaths())
                {
                    renderCommand += " --mdl_path \"" + sp.asString() + "\"";
                }

                // Set environment
                std::string iblFile = (rootPath / "resources/lights/san_giuseppe_bridge.hdr").asString();
                renderCommand += " --hdr \"" + iblFile + "\"";
                renderCommand += " --hdr_rotate 90";
                renderCommand += " --background 0.073239 0.073239 0.083535";

                // Set scene
                renderCommand += " --camera 0 0 3 0 0 0 --fov 45";
                renderCommand += " --materialxtest_mode"; // align image and texcoord space with OSL

                // Set the material
                // the renderer supports MaterialX natively
                renderCommand += " --mat " + doc->getSourceUri() + "?name=" + element->getNamePath();

                // Application setup
                renderCommand += " --nogui"; // headless mode
                renderCommand += " --res 512 512 --spp 1024 --max_path_length 3";
                renderCommand += " --warn"; // reduce the log messages

                // addition optional render arguments
                std::string renderArgs(MATERIALX_MDL_RENDER_ARGUMENTS);
                if (!renderArgs.empty())
                {
                    renderCommand += " " + renderArgs;
                }

                // Write out an .mdl file and the corresponding glsl code
                if (testOptions.dumpGeneratedCode)
                {
                    renderCommand += " --generated " + shaderPath + ".mdl";
                    renderCommand += " --generated_glsl " + shaderPath + ".mdl.glsl";
                }

                // set the output image file
#if defined(MATERIALX_BUILD_OIIO)
                renderCommand += " -o " + shaderPath + "_mdl.exr";
#else
                renderCommand += " -o " + shaderPath + "_mdl.png";
#endif
                // also create a full log
                //renderCommand += " --log_file " + shaderPath + +".mdl_render_log.txt";

                // run the renderer executable
                int returnValue = std::system(renderCommand.c_str());
                CHECK(returnValue == 0);
                continue;

                mx::FilePath errorLogFile = shaderPath + ".mdl_render_log.txt";
                std::ifstream logStream(errorLogFile);
                mx::StringVec result;
                std::string line;
                bool writeLogCode = true;
                bool writeLogLine = false;
                while (std::getline(logStream, line))
                {
                    // add the return code to the log
                    if (writeLogCode)
                    {
                        log << renderCommand << std::endl;
                        log << "\tReturn code: " << std::to_string(returnValue) << std::endl;
                        writeLogCode = false;
                    }
                    // add errors and warnings to the log
                    if (line.find("[ERROR]") != std::string::npos || line.find("[WARNING]") != std::string::npos)
                    {
                        writeLogLine = true;
                    }
                    if (line.find("[INFO]") != std::string::npos || line.find("[VERBOSE]") != std::string::npos)
                    {
                        writeLogLine = false;
                    }
                    if (writeLogLine)
                    {
                        log << "\tLog: " << line << std::endl;
                    }
                }
                CHECK(returnValue == 0);
            }
            catch (mx::Exception& e)
            {
                log << ">> " << e.what() << "\n";
            }
        }
    }

    return true;
}

TEST_CASE("Render: MDL TestSuite", "[rendermdl]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath optionsFilePath = searchPath.find("resources/Materials/TestSuite/_options.mtlx");

    MdlShaderRenderTester renderTester(mx::MdlShaderGenerator::create());
    renderTester.validate(optionsFilePath);
}
