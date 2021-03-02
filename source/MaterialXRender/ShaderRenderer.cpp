//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/ShaderRenderer.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

namespace MaterialX
{

ImageVec ShaderRenderer::getReferencedImages(const ShaderPtr& shader)
{
    if (!shader && !_imageHandler)
    {
        return {};
    }

    ImageVec imageList;

    // Prefetch all required images for Public Uniforms and query their dimensions. 
    // Since Images are cached by ImageHandler, they will be reused during bindTextures
    const ShaderStage& stage = shader->getStage(Stage::PIXEL);
    const VariableBlock& block = stage.getUniformBlock(HW::PUBLIC_UNIFORMS);
    for (const auto& uniform : block.getVariableOrder())
    {
        if (uniform->getType() == Type::FILENAME)
        {
            const string fileName(uniform->getValue()->getValueString());
            ImagePtr image = _imageHandler->acquireImage(fileName);
            if (image)
            {
                imageList.push_back(image);
            }
        }
    }

    return imageList;
}

} // namespace MaterialX
