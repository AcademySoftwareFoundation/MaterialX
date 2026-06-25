//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_HLSLPROGRAM_H
#define MATERIALX_HLSLPROGRAM_H

/// @file
/// HLSL program object built from a generated MaterialX shader

#include <MaterialXRenderHlsl/Export.h>

#include <MaterialXGenShader/Shader.h>

#include <cstdint>
#include <string>
#include <vector>

struct ID3D11Device;
struct ID3D11VertexShader;
struct ID3D11PixelShader;

MATERIALX_NAMESPACE_BEGIN

class HlslProgram;
using HlslProgramPtr = shared_ptr<class HlslProgram>;

/// Class of a HLSL resource as reported by D3D shader reflection. Only the
/// classes we care about for graphics shaders are exposed.
enum class HlslResourceType
{
    CBuffer,        ///< Constant buffer (b#)
    Texture,        ///< Shader resource view (t#)
    Sampler,        ///< Sampler (s#)
    Other           ///< Any other binding D3D reports (uavs, structured buffers, ...)
};

/// One reflected resource binding from a compiled HLSL stage.
struct HlslResourceBinding
{
    std::string name;        ///< Binding name as seen in the source.
    HlslResourceType type = HlslResourceType::Other;
    unsigned int slot = 0;   ///< Register slot index (b#, t#, s# depending on type).
    unsigned int space = 0;  ///< Register space (D3D12 root signatures).
    unsigned int count = 1;  ///< Array count, or 1 for scalar bindings.
};

/// Compiler backend used to translate HLSL source into bytecode.
enum class HlslCompilerBackend
{
    /// FXC via d3dcompiler_47 / D3DCompile. Caps at Shader Model 5.1, but
    /// ships with every Windows SDK and produces DXBC. Default.
    Fxc,
    /// DXC via dxcompiler / IDxcCompiler3. Required for Shader Model 6.x
    /// and HLSL 2021 features. Produces DXIL. Loaded dynamically at first
    /// use; falls back with a clear error if dxcompiler.dll is unavailable.
    Dxc
};

/// @class HlslProgram
/// Wraps invocation of FXC (d3dcompiler_47.dll's D3DCompile) or DXC
/// (dxcompiler.dll's IDxcCompiler3) to turn a MaterialX-generated HLSL
/// shader pair into vertex + pixel shader bytecode. The class is
/// intentionally renderer-free - it does not own a D3D device or render
/// any pixels - and is suitable for use in headless CI to validate that
/// generated HLSL compiles.
///
/// Usage with FXC (default, SM 5.x):
/// @code
///     HlslProgramPtr prog = HlslProgram::create();
///     prog->build(shader);
///     if (!prog->isValid()) { std::cerr << prog->getCompileLog(); }
/// @endcode
///
/// Usage with DXC (SM 6.x):
/// @code
///     HlslProgramPtr prog = HlslProgram::create();
///     prog->setCompilerBackend(HlslCompilerBackend::Dxc);
///     prog->setShaderModel("6_0");
///     prog->build(shader);
/// @endcode
class MX_RENDERHLSL_API HlslProgram
{
  public:
    HlslProgram();
    ~HlslProgram();

    static HlslProgramPtr create()
    {
        return std::make_shared<HlslProgram>();
    }

    /// Select FXC or DXC. Default is FXC. Switching to DXC dynamically
    /// loads dxcompiler.dll on the next build() call; if loading fails the
    /// build returns false with the error in getCompileLog().
    void setCompilerBackend(HlslCompilerBackend backend) { _backend = backend; }
    HlslCompilerBackend getCompilerBackend() const { return _backend; }

    /// Set the HLSL shader-model profile suffix used for both stages
    /// (e.g. "5_0", "5_1", "6_0"). The actual profile passed to the
    /// compiler becomes "vs_<sm>" and "ps_<sm>".
    void setShaderModel(const std::string& sm) { _shaderModel = sm; }
    const std::string& getShaderModel() const { return _shaderModel; }

    /// Set the entry-point function names. Defaults match what
    /// HlslShaderGenerator emits ("VSMain" / "PSMain").
    void setEntryPoints(const std::string& vs, const std::string& ps)
    {
        _vsEntry = vs;
        _psEntry = ps;
    }

    /// Compile the vertex and pixel stages of the supplied generated
    /// MaterialX shader. After this call, isValid() reflects whether both
    /// stages compiled, getCompileLog() carries the D3DCompile messages,
    /// and getVertexBytecode() / getPixelBytecode() expose the bytecode.
    /// Returns true on success.
    bool build(ShaderPtr shader);

    /// Compile both stages from raw source strings. Useful for tests and
    /// for callers that build HLSL outside the MaterialX shader graph.
    /// Same semantics as build(ShaderPtr).
    bool build(const std::string& vsSource, const std::string& psSource);

    /// Compile a raw source string for the given stage profile. Useful for
    /// targeted unit tests of the compile path. Returns true on success.
    bool buildStage(const std::string& source, const std::string& entry,
                    const std::string& profile, std::vector<uint8_t>& bytecode,
                    std::string& log);

    bool isValid() const { return _valid; }

    const std::vector<uint8_t>& getVertexBytecode() const { return _vsBytecode; }
    const std::vector<uint8_t>& getPixelBytecode()  const { return _psBytecode; }

    /// Combined log from the most recent build, including both stages.
    const std::string& getCompileLog() const { return _log; }

    /// Run D3D shader reflection on the stage's bytecode and return every
    /// bound resource (cbuffer, texture, sampler). Returns an empty vector
    /// if the stage has not been built or reflection failed. Useful for
    /// validating that the slot assignments match what an attached
    /// HlslResourceBindingContext intended.
    std::vector<HlslResourceBinding> getVertexBindings() const;
    std::vector<HlslResourceBinding> getPixelBindings()  const;

    /// Reflect a raw bytecode buffer. Public for test code that wants to
    /// inspect bytecode without going through build().
    static std::vector<HlslResourceBinding> reflectBindings(const std::vector<uint8_t>& bytecode);

    /// Instantiate a GPU vertex shader from the most-recently-built vertex
    /// bytecode. Returns nullptr if the program is not built or the device
    /// rejects the bytecode (typically a profile / feature-level mismatch).
    /// The caller takes ownership and must Release() when done.
    ID3D11VertexShader* createVertexShader(ID3D11Device* device) const;

    /// As above for the pixel stage.
    ID3D11PixelShader* createPixelShader(ID3D11Device* device) const;

  private:
    HlslCompilerBackend _backend = HlslCompilerBackend::Fxc;
    std::string _shaderModel = "5_0";
    std::string _vsEntry = "VSMain";
    std::string _psEntry = "PSMain";

    std::vector<uint8_t> _vsBytecode;
    std::vector<uint8_t> _psBytecode;
    std::string _log;
    bool _valid = false;
};

MATERIALX_NAMESPACE_END

#endif
