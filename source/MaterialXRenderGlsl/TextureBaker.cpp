//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/TextureBaker.h>

#include <MaterialXFormat/XmlIo.h>

namespace MaterialX
{

TextureBaker::TextureBaker(unsigned int res) :
    GlslRenderer(res),
    _generator(GlslShaderGenerator::create()),
    _extension(ImageLoader::PNG_EXTENSION)
{
    initialize();
}

void TextureBaker::bakeShaderInputs(ShaderRefPtr shaderRef, GenContext& context, const FilePath& outputFolder)
{
    if (!shaderRef)
    {
        return;
    }

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

    ShaderPtr shader = _generator->generate("BakingShader", output, context);
    createProgram(shader);

    bool encodeSrgb = output->getType() == "color3" || output->getType() == "color4";
    renderTextureSpace(encodeSrgb);

    // TODO: Add support for graphs containing geometric nodes such as position and normal.
    //       Currently, the only supported geometric node is texcoord.

    FilePath filename = outputFolder / generateTextureFilename(output);
    save(filename, false);
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

            // Create the baked bind input.
            BindInputPtr bakedBindInput = bakedShaderRef->addBindInput(bindInput->getName(), bindInput->getType());

            // Add the image node.
            NodePtr bakedImage = bakedNodeGraph->addNode("image", bindInput->getName() + "_baked", bindInput->getType());
            ParameterPtr param = bakedImage->addParameter("file", "filename");
            param->setValueString(generateTextureFilename(output));

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

    writeToXmlFile(bakedTextureDoc, filename);
}

FilePath TextureBaker::generateTextureFilename(OutputPtr output)
{
    string outputName = createValidName(output->getNamePath());
    string udimSuffix = _udim.empty() ? EMPTY_STRING : "_" + _udim;
    return FilePath(outputName + "_baked" + udimSuffix + "." + _extension);
}

} // namespace MaterialX
