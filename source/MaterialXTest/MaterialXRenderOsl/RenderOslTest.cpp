//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXRenderOsl/OslRenderer.h>
#include <MaterialXFormat/Util.h>

#include <chrono>
#include <filesystem>
#include <fstream>

namespace mx = MaterialX;

TEST_CASE("Render: OSL thread settings", "[renderworkers]")
{
    auto renderer = mx::OslRenderer::create();
    CHECK(renderer->getThreadCount() == 0);
    renderer->setThreadCount(3);
    CHECK(renderer->getThreadCount() == 3);
}

#if !defined(_WIN32)
TEST_CASE("Render: OSL failed commands reject stale output", "[renderworkers]")
{
    auto dir = std::filesystem::temp_directory_path() /
        ("materialx osl failure " + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(dir);
    struct Cleanup
    {
        std::filesystem::path path;
        ~Cleanup() { std::error_code error; std::filesystem::remove_all(path, error); }
    } cleanup { dir };

    auto renderer = mx::OslRenderer::create();
    renderer->setOslOutputFilePath(dir.string());
    renderer->setOslShaderName("test");
    renderer->setOslCompilerExecutable("/bin/false");
    std::ofstream(dir / "test.oso") << "stale compiled shader";
    mx::ShaderRenderer::StageMap stages { { "pixel", "shader test() {}" } };
    CHECK_THROWS_AS(renderer->createProgram(stages), mx::ExceptionRenderError);
    CHECK_FALSE(std::filesystem::exists(dir / "test.oso"));

    // A silent successful command that writes no output is not a compiler.
    renderer->setOslCompilerExecutable("/bin/true");
    CHECK_THROWS_AS(renderer->createProgram(stages), mx::ExceptionRenderError);
    renderer->setOslCompilerExecutable((dir / "missing compiler").string());
    CHECK_THROWS_AS(renderer->createProgram(stages), mx::ExceptionRenderError);

    renderer->setOslShaderOutput("out", "color");
    auto utilityPath = mx::getDefaultDataSearchPath().find("resources/Utilities");
    renderer->setOslUtilityOSOPath(utilityPath);
    renderer->setOslTestRenderSceneTemplateFile(utilityPath / "scene_template.xml");
    renderer->setOslTestRenderExecutable("/bin/false");
    std::ofstream(dir / "test_osl.png") << "stale image";
    auto original = std::filesystem::current_path();
    CHECK_THROWS_AS(renderer->render(), mx::ExceptionRenderError);
    CHECK_FALSE(std::filesystem::exists(dir / "test_osl.png"));
    CHECK(std::filesystem::current_path() == original);
}
#endif
