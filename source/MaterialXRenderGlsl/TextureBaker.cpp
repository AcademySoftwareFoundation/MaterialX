//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/TextureBaker.h>

#include <MaterialXFormat/XmlIo.h>

namespace MaterialX
{

TextureBaker::TextureBaker(unsigned int width, unsigned int height, Image::BaseType baseType) :
    GlslRenderer(width, height, baseType),
    _generator(GlslShaderGenerator::create())
{
    // Assign a default extension for texture baking.
    _extension = (baseType == Image::BaseType::UINT8) ?
                 ImageLoader::PNG_EXTENSION :
                 ImageLoader::HDR_EXTENSION;

    // Initialize the underlying renderer.
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

void TextureBaker::bakeShaderInputs(NodePtr shader, GenContext& context, const FilePath& outputFolder)
{
    if (!shader)
    {
        return;
    }

    for (InputPtr input : shader->getInputs())
    {
        OutputPtr output = input->getConnectedOutput();
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
    getFrameBuffer()->setEncodeSrgb(encodeSrgb);

    renderTextureSpace();

    // TODO: Add support for graphs containing geometric nodes such as position and normal.
    //       Currently, the only supported geometric node is texcoord.

    FilePath filename = outputFolder / generateTextureFilename(output);
    save(filename);
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
    if (_baseType == Image::BaseType::UINT8)
    {
        bakedNodeGraph->setColorSpace("srgb_texture");
    }

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

void TextureBaker::writeBakedDocument(NodePtr shader, const FilePath& filename)
{
    if (!shader)
    {
        return;
    }

    // Create document.
    DocumentPtr bakedTextureDoc = createDocument();

    // Create top-level elements.
    NodeGraphPtr bakedNodeGraph = bakedTextureDoc->addNodeGraph("NG_baked");
    bakedNodeGraph->setColorSpace("srgb_texture");
    NodePtr bakedMaterial = bakedTextureDoc->addNode(SURFACE_MATERIAL_NODE_STRING, "M_baked", MATERIAL_TYPE_STRING);
    NodePtr bakedShader = bakedTextureDoc->addNode(shader->getCategory(), shader->getName() + "_baked", shader->getType());
    InputPtr shaderInput = bakedMaterial->addInput(SURFACE_SHADER_TYPE_STRING, SURFACE_SHADER_TYPE_STRING);
    shaderInput->setNodeName(bakedShader->getName());

    // Create input elements on the baked shader reference.
    for (ValueElementPtr valueElem : shader->getChildrenOfType<ValueElement>())
    {
        InputPtr input = valueElem->asA<Input>();
        if (input && input->getConnectedOutput())
        {
            OutputPtr output = input->getConnectedOutput();

            // Create the baked bind input.
            InputPtr bakedInput = bakedShader->addInput(input->getName(), input->getType());

            // Add the image node.
            NodePtr bakedImage = bakedNodeGraph->addNode("image", input->getName() + "_baked", input->getType());
            ParameterPtr param = bakedImage->addParameter("file", "filename");
            param->setValueString(generateTextureFilename(output));

            // Add the graph output and connect it to the image node upstream
            // and the shader input downstream.
            OutputPtr bakedOutput = bakedNodeGraph->addOutput(input->getName() + "_output", input->getType());
            bakedOutput->setConnectedNode(bakedImage);
            bakedInput->setAttribute(PortElement::NODE_GRAPH_ATTRIBUTE, bakedNodeGraph->getName());
            bakedInput->setAttribute(PortElement::OUTPUT_ATTRIBUTE, bakedOutput->getName());
        }
        else
        {
            ElementPtr bakedElem = bakedShader->addChildOfCategory(valueElem->getCategory(), valueElem->getName());
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
