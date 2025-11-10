//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderSlang/SlangShaderCache.h>
#include <fstream>

MATERIALX_NAMESPACE_BEGIN

namespace
{

std::string encodeBase64(const void* data, const size_t size)
{
    static std::string base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    //const uint8_t* input = reinterpret_cast<const uint8_t*>(data);
    std::vector<uint8_t> input(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + size);

    std::string result;

    for (size_t i = 0; i < size; i += 3)
    {
        /// Split next 3 characters to 4 groups of 6 bits.
        /// If there is no input character, leave the -1 placeholder.
        int32_t values[4] = {-1,-1,-1,-1};
        values[0] = (input[i] >> 2) & 0b00111111;
        values[1] = (input[i] << 4) & 0b00110000;
        if (i + 1 < size)
        {
            values[1] += (input[i + 1] >> 4) & 0b00001111;
            values[2] += (input[i + 1] << 2) & 0b00111100;
        }
        if (i + 2 < size)
        {
            values[2] += (input[i + 2] >> 6) & 0b00000011;
            values[3] += (input[i + 2]) & 0b00111111;
        }

        /// Encode valid characters, replace placeholders with =
        for (int j = 0; j < 4; ++j)
            result.push_back(values[j] >= 0 ? base64[values[j]] : '=');
    }

    return result;
}

} // namespace

SlangShaderCache::SlangShaderCache(const FilePath& cachePath) :
    _cachePath(cachePath)
{
    _cachePath.createDirectory();
}

rhi::Result SlangShaderCache::writeCache(ISlangBlob* key, ISlangBlob* data)
{
    std::string filename = encodeBase64(key->getBufferPointer(), key->getBufferSize());

    filename += ".slangcache";
    FilePath path = _cachePath / filename;

    std::ofstream ofs(path, std::ios::binary);
    if (!ofs)
        return SLANG_E_NOT_FOUND;

    ofs.write((const char*) data->getBufferPointer(),
              data->getBufferSize());
    return SLANG_OK;
}

rhi::Result SlangShaderCache::queryCache(ISlangBlob* key, ISlangBlob** outData)
{
    std::string filename = encodeBase64(key->getBufferPointer(), key->getBufferSize());

    filename += ".slangcache";
    FilePath path = _cachePath / filename;

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
    {
        *outData = nullptr;
        return SLANG_E_NOT_FOUND;
    }

    ifs.seekg(0, std::ios::end);
    size_t length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    rhi::ComPtr<ISlangBlob> blob = rhi::getRHI()->createBlob(nullptr, length);
    ifs.read((char*)blob->getBufferPointer(), blob->getBufferSize());
    ifs.close();

    *outData = blob.detach();
    return SLANG_OK;
}

MATERIALX_NAMESPACE_END
