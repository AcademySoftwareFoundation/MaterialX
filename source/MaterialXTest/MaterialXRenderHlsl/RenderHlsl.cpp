//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//
// HLSL render-side validation. Drives every shader in the standard test
// suite through HlslShaderGenerator, then through D3DCompile (FXC) via
// HlslProgram, and asserts the bytecode comes out clean. This is the only
// way the codegen path actually proves "compiles with DXC/FXC" in CI; the
// pure [genhlsl] tests only check structural properties of the source.
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXGenShader/GenShaderUtil.h>

#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXRenderHlsl/HlslProgram.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

#include <fstream>
#include <set>
#include <sstream>

namespace mx = MaterialX;

namespace
{

// Walk a directory tree of .mtlx test materials, generate HLSL for the
// surface-shader element of each one, and run it through HlslProgram. Any
// failures are accumulated in `failures` for a single end-of-test summary.
struct CompileFailure
{
    mx::FilePath file;
    std::string element;
    std::string log;
};

bool compileMaterial(const mx::FilePath& mtlxPath, mx::DocumentPtr libraries,
                     const mx::FileSearchPath& searchPath,
                     std::vector<CompileFailure>& failures)
{
    mx::DocumentPtr doc = mx::createDocument();
    try
    {
        mx::readFromXmlFile(doc, mtlxPath, searchPath);
    }
    catch (const std::exception&)
    {
        return true; // Parsing problems are not the renderer's concern.
    }
    doc->setDataLibrary(libraries);

    std::vector<mx::TypedElementPtr> renderable = mx::findRenderableElements(doc);
    if (renderable.empty())
        return true;

    mx::GenContext context(mx::HlslShaderGenerator::create());
    context.registerSourceCodeSearchPath(searchPath);
    context.getShaderGenerator().registerTypeDefs(doc);

    bool allOk = true;
    for (auto& elem : renderable)
    {
        if (!elem)
            continue;
        mx::ShaderPtr shader;
        try
        {
            shader = context.getShaderGenerator().generate(elem->getName(), elem, context);
        }
        catch (const std::exception& e)
        {
            CompileFailure f{ mtlxPath, elem->getName(), std::string("[gen] ") + e.what() };
            failures.push_back(std::move(f));
            allOk = false;
            continue;
        }
        if (!shader)
            continue;

        mx::HlslProgramPtr prog = mx::HlslProgram::create();
        if (!prog->build(shader))
        {
            CompileFailure f{ mtlxPath, elem->getName(), prog->getCompileLog() };
            failures.push_back(std::move(f));
            allOk = false;
        }
    }
    return allOk;
}

void compileTree(const mx::FilePath& root, mx::DocumentPtr libraries,
                 const mx::FileSearchPath& searchPath,
                 std::vector<CompileFailure>& failures)
{
    if (!root.exists())
        return;
    for (const auto& filename : root.getFilesInDirectory(mx::MTLX_EXTENSION))
    {
        compileMaterial(root / filename, libraries, searchPath, failures);
    }
    for (const auto& sub : root.getSubDirectories())
    {
        if (sub == root)
            continue;
        compileTree(sub, libraries, searchPath, failures);
    }
}

} // namespace

TEST_CASE("Render: Hlsl Compile TestSuite", "[renderhlsl]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::DocumentPtr libraries = mx::createDocument();
    mx::loadLibraries({ "libraries" }, searchPath, libraries);

    std::vector<CompileFailure> failures;

    compileTree(searchPath.find("resources/Materials/TestSuite"), libraries, searchPath, failures);
    compileTree(searchPath.find("resources/Materials/Examples"),  libraries, searchPath, failures);

    if (!failures.empty())
    {
        std::ostringstream out;
        out << "HLSL D3DCompile failures (" << failures.size() << "):\n";
        const std::size_t reportLimit = 10;
        for (std::size_t i = 0; i < failures.size() && i < reportLimit; ++i)
        {
            out << "  " << failures[i].file.asString() << " :: " << failures[i].element << "\n"
                << failures[i].log << "\n";
        }
        if (failures.size() > reportLimit)
        {
            out << "  ... " << (failures.size() - reportLimit) << " more omitted\n";
        }
        // Persist the full log alongside the codegen test outputs so CI
        // artifacts capture every failure for triage.
        std::ofstream logFile("renderhlsl_compile_log.txt");
        for (const auto& f : failures)
        {
            logFile << f.file.asString() << " :: " << f.element << "\n"
                    << f.log << "\n----\n";
        }
        FAIL(out.str());
    }
}
