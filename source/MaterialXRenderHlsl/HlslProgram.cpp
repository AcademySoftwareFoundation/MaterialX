//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderHlsl/HlslProgram.h>

#include <MaterialXGenShader/ShaderStage.h>

#include <Windows.h>
#include <d3d11.h>
#include <d3d11shader.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>
#include <dxcapi.h>
#include <wrl/client.h>

#include <cwchar>
#include <mutex>

MATERIALX_NAMESPACE_BEGIN

namespace
{

using Microsoft::WRL::ComPtr;

// Translate a D3DBlob into a uint8_t vector and, in a single helper, drain a
// possibly-null error blob into the output log.
void appendBlobToLog(ID3DBlob* blob, std::string& log)
{
    if (!blob)
        return;
    const char* msg = static_cast<const char*>(blob->GetBufferPointer());
    log.append(msg, blob->GetBufferSize());
}

void copyBlobToBytecode(ID3DBlob* blob, std::vector<uint8_t>& out)
{
    if (!blob)
        return;
    const auto* src = static_cast<const uint8_t*>(blob->GetBufferPointer());
    const std::size_t sz = blob->GetBufferSize();
    out.assign(src, src + sz);
}

// DXC entry point loaded dynamically so the renderer module does not need
// to link dxcompiler.lib at build time. The first DXC build call resolves
// the symbol from dxcompiler.dll; if the load fails the build returns
// false with a clear error in the log.
typedef HRESULT (__stdcall *DxcCreateInstanceProc)(REFCLSID, REFIID, LPVOID*);

DxcCreateInstanceProc loadDxcCreateInstance(std::string& log)
{
    static std::mutex s_mutex;
    static HMODULE s_module = nullptr;
    static DxcCreateInstanceProc s_proc = nullptr;
    static bool s_attempted = false;

    std::lock_guard<std::mutex> guard(s_mutex);
    if (!s_attempted)
    {
        s_attempted = true;
        s_module = ::LoadLibraryW(L"dxcompiler.dll");
        if (s_module)
        {
            s_proc = reinterpret_cast<DxcCreateInstanceProc>(
                ::GetProcAddress(s_module, "DxcCreateInstance"));
        }
    }

    if (!s_proc)
    {
        log += "[dxc] dxcompiler.dll not found or DxcCreateInstance unresolved; "
               "install the DXC redistributable or update the Windows SDK.\n";
    }
    return s_proc;
}

std::wstring widen(const std::string& s)
{
    std::wstring out;
    out.reserve(s.size());
    for (char c : s)
    {
        out.push_back(static_cast<wchar_t>(static_cast<unsigned char>(c)));
    }
    return out;
}

bool buildStageDxc(const std::string& source, const std::string& entry,
                   const std::string& profile, std::vector<uint8_t>& bytecode,
                   std::string& log)
{
    bytecode.clear();

    DxcCreateInstanceProc createInstance = loadDxcCreateInstance(log);
    if (!createInstance)
        return false;

    ComPtr<IDxcCompiler3> compiler;
    if (FAILED(createInstance(CLSID_DxcCompiler, IID_PPV_ARGS(compiler.GetAddressOf()))))
    {
        log += "[dxc] failed to create IDxcCompiler3 instance.\n";
        return false;
    }

    ComPtr<IDxcUtils> utils;
    if (FAILED(createInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.GetAddressOf()))))
    {
        log += "[dxc] failed to create IDxcUtils instance.\n";
        return false;
    }

    DxcBuffer src{};
    src.Ptr = source.data();
    src.Size = source.size();
    src.Encoding = DXC_CP_ACP;

    const std::wstring wEntry = widen(entry);
    const std::wstring wProfile = widen(profile);

    std::vector<LPCWSTR> args;
    args.push_back(L"-E"); args.push_back(wEntry.c_str());
    args.push_back(L"-T"); args.push_back(wProfile.c_str());
    args.push_back(L"-HV"); args.push_back(L"2021");
#ifdef _DEBUG
    args.push_back(L"-Zi");
    args.push_back(L"-Od");
#else
    args.push_back(L"-O3");
#endif

    ComPtr<IDxcResult> result;
    HRESULT hr = compiler->Compile(&src, args.data(), static_cast<UINT32>(args.size()),
                                   nullptr, IID_PPV_ARGS(result.GetAddressOf()));
    if (FAILED(hr) || !result)
    {
        log += "[dxc] IDxcCompiler3::Compile failed.\n";
        return false;
    }

    ComPtr<IDxcBlobUtf8> errors;
    if (SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.GetAddressOf()), nullptr))
        && errors && errors->GetStringLength() > 0)
    {
        log += "[" + profile + " " + entry + "] ";
        log.append(errors->GetStringPointer(), errors->GetStringLength());
        if (!log.empty() && log.back() != '\n')
            log += '\n';
    }

    HRESULT status = S_OK;
    result->GetStatus(&status);
    if (FAILED(status))
        return false;

    ComPtr<IDxcBlob> object;
    if (FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(object.GetAddressOf()), nullptr))
        || !object)
    {
        log += "[dxc] no compiled object returned.\n";
        return false;
    }

    const auto* p = static_cast<const uint8_t*>(object->GetBufferPointer());
    const std::size_t sz = object->GetBufferSize();
    bytecode.assign(p, p + sz);
    return true;
}

bool buildStageFxc(const std::string& source, const std::string& entry,
                   const std::string& profile, std::vector<uint8_t>& bytecode,
                   std::string& log)
{
    bytecode.clear();

    ComPtr<ID3DBlob> codeBlob;
    ComPtr<ID3DBlob> errBlob;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    HRESULT hr = ::D3DCompile(
        source.c_str(),
        source.size(),
        nullptr,             // sourceName
        nullptr,             // defines
        nullptr,             // include
        entry.c_str(),
        profile.c_str(),
        flags,
        0,
        codeBlob.GetAddressOf(),
        errBlob.GetAddressOf());

    if (errBlob)
    {
        log += "[" + profile + " " + entry + "] ";
        appendBlobToLog(errBlob.Get(), log);
        if (!log.empty() && log.back() != '\n')
            log += '\n';
    }

    if (FAILED(hr))
    {
        return false;
    }

    copyBlobToBytecode(codeBlob.Get(), bytecode);
    return true;
}

} // namespace

HlslProgram::HlslProgram() = default;
HlslProgram::~HlslProgram() = default;

bool HlslProgram::buildStage(const std::string& source, const std::string& entry,
                             const std::string& profile, std::vector<uint8_t>& bytecode,
                             std::string& log)
{
    return _backend == HlslCompilerBackend::Dxc
        ? buildStageDxc(source, entry, profile, bytecode, log)
        : buildStageFxc(source, entry, profile, bytecode, log);
}

bool HlslProgram::build(ShaderPtr shader)
{
    _valid = false;
    _vsBytecode.clear();
    _psBytecode.clear();
    _log.clear();

    if (!shader)
    {
        _log = "HlslProgram::build: null shader\n";
        return false;
    }

    const std::string& vs = shader->getSourceCode(Stage::VERTEX);
    const std::string& ps = shader->getSourceCode(Stage::PIXEL);

    bool vsOk = !vs.empty() && buildStage(vs, _vsEntry, "vs_" + _shaderModel, _vsBytecode, _log);
    bool psOk = !ps.empty() && buildStage(ps, _psEntry, "ps_" + _shaderModel, _psBytecode, _log);

    _valid = vsOk && psOk;
    return _valid;
}

bool HlslProgram::build(const std::string& vsSource, const std::string& psSource)
{
    _valid = false;
    _vsBytecode.clear();
    _psBytecode.clear();
    _log.clear();

    bool vsOk = !vsSource.empty() && buildStage(vsSource, _vsEntry, "vs_" + _shaderModel, _vsBytecode, _log);
    bool psOk = !psSource.empty() && buildStage(psSource, _psEntry, "ps_" + _shaderModel, _psBytecode, _log);

    _valid = vsOk && psOk;
    return _valid;
}

std::vector<HlslResourceBinding> HlslProgram::getVertexBindings() const
{
    return reflectBindings(_vsBytecode);
}

std::vector<HlslResourceBinding> HlslProgram::getPixelBindings() const
{
    return reflectBindings(_psBytecode);
}

ID3D11VertexShader* HlslProgram::createVertexShader(ID3D11Device* device) const
{
    if (!device || _vsBytecode.empty())
        return nullptr;
    ID3D11VertexShader* vs = nullptr;
    HRESULT hr = device->CreateVertexShader(_vsBytecode.data(), _vsBytecode.size(),
                                            nullptr, &vs);
    return SUCCEEDED(hr) ? vs : nullptr;
}

ID3D11PixelShader* HlslProgram::createPixelShader(ID3D11Device* device) const
{
    if (!device || _psBytecode.empty())
        return nullptr;
    ID3D11PixelShader* ps = nullptr;
    HRESULT hr = device->CreatePixelShader(_psBytecode.data(), _psBytecode.size(),
                                           nullptr, &ps);
    return SUCCEEDED(hr) ? ps : nullptr;
}

namespace
{

HlslResourceType classifyBindingType(D3D_SHADER_INPUT_TYPE t)
{
    switch (t)
    {
        case D3D_SIT_CBUFFER: return HlslResourceType::CBuffer;
        case D3D_SIT_TEXTURE: return HlslResourceType::Texture;
        case D3D_SIT_SAMPLER: return HlslResourceType::Sampler;
        default:              return HlslResourceType::Other;
    }
}

// Try to reflect DXIL via DXC's IDxcUtils::CreateReflection. Returns true
// if reflection succeeded; false if dxcompiler isn't available or the
// bytecode isn't recognised as DXIL. Falls back caller-side to D3DReflect
// for DXBC.
bool reflectDxil(const std::vector<uint8_t>& bytecode, std::vector<HlslResourceBinding>& out)
{
    std::string ignoredLog;
    DxcCreateInstanceProc createInstance = loadDxcCreateInstance(ignoredLog);
    if (!createInstance)
        return false;

    ComPtr<IDxcUtils> utils;
    if (FAILED(createInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.GetAddressOf()))))
        return false;

    DxcBuffer buf{};
    buf.Ptr = bytecode.data();
    buf.Size = bytecode.size();
    buf.Encoding = DXC_CP_ACP;

    ComPtr<ID3D12ShaderReflection> refl;
    if (FAILED(utils->CreateReflection(&buf, IID_PPV_ARGS(refl.GetAddressOf()))) || !refl)
        return false;

    D3D12_SHADER_DESC desc = {};
    if (FAILED(refl->GetDesc(&desc)))
        return false;

    out.reserve(out.size() + desc.BoundResources);
    for (UINT i = 0; i < desc.BoundResources; ++i)
    {
        D3D12_SHADER_INPUT_BIND_DESC ibd = {};
        if (FAILED(refl->GetResourceBindingDesc(i, &ibd)))
            continue;
        HlslResourceBinding b;
        b.name  = ibd.Name ? ibd.Name : std::string();
        b.slot  = ibd.BindPoint;
        b.space = ibd.Space;     // DXC carries register spaces through.
        b.count = ibd.BindCount ? ibd.BindCount : 1;
        b.type  = classifyBindingType(ibd.Type);
        out.push_back(std::move(b));
    }
    return true;
}

bool reflectDxbc(const std::vector<uint8_t>& bytecode, std::vector<HlslResourceBinding>& out)
{
    ComPtr<ID3D11ShaderReflection> refl;
    if (FAILED(::D3DReflect(bytecode.data(), bytecode.size(),
                            IID_PPV_ARGS(refl.GetAddressOf()))) || !refl)
        return false;

    D3D11_SHADER_DESC desc = {};
    if (FAILED(refl->GetDesc(&desc)))
        return false;

    out.reserve(out.size() + desc.BoundResources);
    for (UINT i = 0; i < desc.BoundResources; ++i)
    {
        D3D11_SHADER_INPUT_BIND_DESC ibd = {};
        if (FAILED(refl->GetResourceBindingDesc(i, &ibd)))
            continue;
        HlslResourceBinding b;
        b.name  = ibd.Name ? ibd.Name : std::string();
        b.slot  = ibd.BindPoint;
        // D3D11_SHADER_INPUT_BIND_DESC has no Space field; spaces are a
        // D3D12 concept and only the DXIL path can populate them.
        b.space = 0;
        b.count = ibd.BindCount ? ibd.BindCount : 1;
        b.type  = classifyBindingType(ibd.Type);
        out.push_back(std::move(b));
    }
    return true;
}

} // namespace

std::vector<HlslResourceBinding> HlslProgram::reflectBindings(const std::vector<uint8_t>& bytecode)
{
    std::vector<HlslResourceBinding> out;
    if (bytecode.empty())
        return out;

    // Both DXBC (FXC) and DXIL (DXC) start with the four-byte tag "DXBC".
    // The container holds different chunks: a DXIL container has a "DXIL"
    // part. We try DXC's reflection path first when DXC is available;
    // ID3D12ShaderReflection reflects DXIL successfully and silently fails
    // on DXBC, in which case we fall back to D3DReflect.
    if (reflectDxil(bytecode, out))
        return out;
    out.clear();
    reflectDxbc(bytecode, out);
    return out;
}

MATERIALX_NAMESPACE_END
