//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
// Smoke tests for the DXC code path on HlslProgram. The DXC SDK ships with
// modern Windows SDKs (10.0.20348+); when dxcompiler.dll is unavailable on
// the test machine the build fails gracefully and the test is skipped via
// a WARN, rather than failing the suite. CI machines on supported SDK
// versions exercise the real SM 6.0 compile path.
//

#include <MaterialXTest/External/Catch/catch.hpp>

#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXRenderHlsl/HlslProgram.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

namespace mx = MaterialX;

namespace
{

const char* kTrivialVs = R"(
struct VsIn  { float3 pos : POSITION; };
struct VsOut { float4 pos : SV_Position; };

VsOut VSMain(VsIn vin)
{
    VsOut vout;
    vout.pos = float4(vin.pos, 1.0);
    return vout;
}
)";

const char* kTrivialPs = R"(
struct VsOut { float4 pos : SV_Position; };

float4 PSMain(VsOut pin) : SV_Target
{
    return float4(pin.pos.xy * 0.5 + 0.5, 0.0, 1.0);
}
)";

bool dxcAvailable()
{
    // Probe: try a trivial DXC compile and see whether the dynamic load
    // succeeded. If dxcompiler.dll is missing the log carries the
    // "[dxc] dxcompiler.dll not found" marker.
    mx::HlslProgramPtr probe = mx::HlslProgram::create();
    probe->setCompilerBackend(mx::HlslCompilerBackend::Dxc);
    probe->setShaderModel("6_0");
    std::vector<uint8_t> bytecode;
    std::string log;
    probe->buildStage(kTrivialPs, "PSMain", "ps_6_0", bytecode, log);
    return log.find("dxcompiler.dll not found") == std::string::npos;
}

} // namespace

TEST_CASE("Render: Hlsl Dxc SmokeStage", "[renderhlsl][dxc]")
{
    if (!dxcAvailable())
    {
        WARN("dxcompiler.dll not available; skipping DXC tests.");
        return;
    }

    mx::HlslProgramPtr prog = mx::HlslProgram::create();
    prog->setCompilerBackend(mx::HlslCompilerBackend::Dxc);
    prog->setShaderModel("6_0");

    std::vector<uint8_t> vsBc, psBc;
    std::string vsLog, psLog;
    bool vsOk = prog->buildStage(kTrivialVs, "VSMain", "vs_6_0", vsBc, vsLog);
    INFO("VS log:\n" << vsLog);
    REQUIRE(vsOk);
    REQUIRE_FALSE(vsBc.empty());

    bool psOk = prog->buildStage(kTrivialPs, "PSMain", "ps_6_0", psBc, psLog);
    INFO("PS log:\n" << psLog);
    REQUIRE(psOk);
    REQUIRE_FALSE(psBc.empty());

    // DXIL containers begin with the four-byte ASCII tag "DXBC" (yes, the
    // DXIL container reuses the historical DXBC header magic).
    REQUIRE(vsBc.size() >= 4);
    REQUIRE(vsBc[0] == 'D'); REQUIRE(vsBc[1] == 'X');
    REQUIRE(vsBc[2] == 'B'); REQUIRE(vsBc[3] == 'C');
}

TEST_CASE("Render: Hlsl Dxc StandardSurface", "[renderhlsl][dxc]")
{
    if (!dxcAvailable())
    {
        WARN("dxcompiler.dll not available; skipping DXC tests.");
        return;
    }

    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    mx::FilePath testFile = searchPath.find("resources/Materials/Examples/StandardSurface/standard_surface_carpaint.mtlx");
    mx::DocumentPtr doc = mx::createDocument();
    mx::readFromXmlFile(doc, testFile);
    doc->setDataLibrary(libraries);

    mx::ElementPtr element = doc->getChild("Car_Paint");
    REQUIRE(element);

    mx::GenContext context(mx::HlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);
    context.getShaderGenerator().registerTypeDefs(doc);

    mx::ShaderPtr shader = context.getShaderGenerator().generate(element->getName(), element, context);
    REQUIRE(shader);

    mx::HlslProgramPtr prog = mx::HlslProgram::create();
    prog->setCompilerBackend(mx::HlslCompilerBackend::Dxc);
    prog->setShaderModel("6_0");
    bool built = prog->build(shader);
    INFO("DXC compile log:\n" << prog->getCompileLog());
    REQUIRE(built);

    // Reflection should still report at least the cbuffers we expect.
    auto bindings = prog->getPixelBindings();
    REQUIRE_FALSE(bindings.empty());
}
