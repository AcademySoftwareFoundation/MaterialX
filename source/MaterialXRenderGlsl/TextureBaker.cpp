//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/TextureBaker.h>

#include <MaterialXRenderGlsl/GLTextureHandler.h>
#include <MaterialXRenderGlsl/GlslValidator.h>

#include <MaterialXRender/LightHandler.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/Util.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/HwShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXFormat/XmlIo.h>

namespace MaterialX
{

void TextureBaker::bakeShaderInputs(ShaderRefPtr shaderRef, const FileSearchPath& searchPath, GenContext context, const FilePath& outputFolder)
{
    if (!shaderRef)
    {
        return;
    }

    context.getOptions().textureSpaceRender = true;
    _searchPath = searchPath;
    for (BindInputPtr bindInput : shaderRef->getBindInputs())
    {
        OutputPtr output = bindInput->getConnectedOutput();
        if (output)
        {
            bakeGraphOutput(output, context, outputFolder);
        }
    }
}

void TextureBaker::bakeGraphOutput(OutputPtr output, GenContext& context, const FilePath& outputFolder)
{
    if (!output)
    {
        return;
    }

    _rasterizer = GlslValidator::create(_frameBufferRes);
    StbImageLoaderPtr stbLoader = StbImageLoader::create();
    GLTextureHandlerPtr imageHandler = GLTextureHandler::create(stbLoader);
    imageHandler->setSearchPath(_searchPath);
    _rasterizer->setImageHandler(imageHandler);
    _rasterizer->initialize();

    _generator = GlslShaderGenerator::create();

    const std::string outputName = createValidName(output->getNamePath());
    ShaderPtr shader = _generator->generate(outputName + "_baker", output, context);

    bool writeSrgb = output->getType() == "color3" || output->getType() == "color4";
    _rasterizer->validateCreation(shader);
    _rasterizer->renderScreenSpaceQuad(context, writeSrgb);

    FilePath filename = outputFolder / FilePath(outputName + "_baked." + _fileSuffix);
    _rasterizer->save(filename, false);
}

void TextureBaker::writeBakedDocument(ShaderRefPtr shaderRef, const FilePath& filename)
{
    if (!shaderRef)
    {
        return;
    }

    // Create document.
    DocumentPtr bakedTextureDoc = createDocument();

    // Create top-level elements.
    NodeGraphPtr bakedNodeGraph = bakedTextureDoc->addNodeGraph("NG_baked");
    MaterialPtr bakedMaterial = bakedTextureDoc->addMaterial("M_baked");
    ShaderRefPtr bakedShaderRef = bakedMaterial->addShaderRef(shaderRef->getName() + "_baked", shaderRef->getAttribute("node"));
    bakedNodeGraph->setColorSpace("srgb_texture");

    // Create bind elements on the baked shader reference.
    for (ValueElementPtr valueElem : shaderRef->getChildrenOfType<ValueElement>())
    {
        BindInputPtr bindInput = valueElem->asA<BindInput>();
        if (bindInput && bindInput->getConnectedOutput())
        {
            OutputPtr output = bindInput->getConnectedOutput();
            const std::string outputName = createValidName(output->getNamePath());

            // Create the baked bind input.
            BindInputPtr bakedBindInput = bakedShaderRef->addBindInput(bindInput->getName(), bindInput->getType());

            // Add the image node.
            NodePtr bakedImage = bakedNodeGraph->addNode("image", bindInput->getName() + "_baked", bindInput->getType());
            ParameterPtr param = bakedImage->addParameter("file", "filename");
            param->setValueString(outputName + "_baked." + _fileSuffix);

            // Add the graph output.
            OutputPtr bakedOutput = bakedNodeGraph->addOutput(bindInput->getName() + "_output", bindInput->getType());
            bakedOutput->setConnectedNode(bakedImage);
            bakedBindInput->setConnectedOutput(bakedOutput);
        }
        else
        {
            ElementPtr bakedElem = bakedShaderRef->addChildOfCategory(valueElem->getCategory(), valueElem->getName());
            bakedElem->copyContentFrom(valueElem);
        }
    }

    XmlWriteOptions writeOptions;
    writeOptions.writeXIncludeEnable = false;
    writeToXmlFile(bakedTextureDoc, filename, &writeOptions);
}

} // namespace MaterialX
