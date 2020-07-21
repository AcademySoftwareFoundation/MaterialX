//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/TextureBaker.h>

#include <MaterialXRenderGlsl/GlslProgram.h>
#include <MaterialXRender/Util.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

#include <iostream>

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

void TextureBaker::bakeShaderInputs(ConstShaderRefPtr shaderRef, GenContext& context, const FilePath& outputFolder, const string udim)
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
            bakeGraphOutput(output, context, outputFolder, udim);
        }
    }
}

void TextureBaker::bakeShaderInputs(NodePtr shader, GenContext& context, const FilePath& outputFolder, const string udim)
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
            bakeGraphOutput(output, context, outputFolder, udim);
        }
    }
}

void TextureBaker::bakeGraphOutput(OutputPtr output, GenContext& context, const FilePath& outputFolder, const string udim)
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

    FilePath filename = outputFolder / generateTextureFilename(output, udim);
    save(filename);
    std::cout << "Saved " + filename.getBaseName() << std::endl;
}

void TextureBaker::writeBakedDocument(ConstShaderRefPtr shaderRef, const FilePath& filename, ValuePtr udimSetValue)
{
    if (!shaderRef)
    {
        return;
    }

    // Create document.
    DocumentPtr bakedTextureDoc = createDocument();

    // Create top-level elements.
    NodeGraphPtr bakedNodeGraph = bakedTextureDoc->addNodeGraph("NG_baked");
    GeomInfoPtr bakedGeom = (udimSetValue) ? bakedTextureDoc->addGeomInfo("GI_baked") : nullptr;
    if (bakedGeom)
    {
        bakedGeom->setGeomPropValue("udimset", udimSetValue->getValueString(), "stringarray");
    }
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
            param->setValueString(generateTextureFilename(output, (udimSetValue) ? "<UDIM>" : ""));

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
    std::cout << "Wrote out " + filename.getBaseName() << std::endl;
}

void TextureBaker::writeBakedDocument(NodePtr shader, const FilePath& filename, ValuePtr udimSetValue)
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
    GeomInfoPtr bakedGeom = (udimSetValue) ? bakedTextureDoc->addGeomInfo("GI_baked") : nullptr;
    if (bakedGeom)
    {
        bakedGeom->setGeomPropValue("udimset", udimSetValue->getValueString(), "stringarray");
    }
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
            param->setValueString(generateTextureFilename(output, (udimSetValue) ? "<UDIM>" : ""));

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

// Helper function to generate mtlx filename
FilePath TextureBaker::generateTextureFilename(OutputPtr output, const string udim)
{
    string outputName = createValidName(output->getNamePath());
    string udimSuffix = udim.empty() ? EMPTY_STRING : "_" + udim;

    return FilePath(outputName + "_baked" + udimSuffix + "." + _extension);
}

// Helper function to initialize file search pat
FileSearchPath initFileSearchPath()
{
    FilePath installSearchPath = FilePath::getModulePath().getParentPath();
    FilePath devSearchPath = FilePath(__FILE__).getParentPath().getParentPath().getParentPath();
    FileSearchPath filename = FileSearchPath(installSearchPath);
    filename.append(installSearchPath);

    if (!devSearchPath.isEmpty() && devSearchPath.exists())
    {
        filename.append(devSearchPath);
        devSearchPath = devSearchPath / "libraries";
        if (devSearchPath.exists())
        {
            filename.append(devSearchPath);
        }
    }
    return filename;
}

// Helper function to initialize shader generation context
GenContext initGenContext()
{
    GenContext genContext = GlslShaderGenerator::create();
    genContext.getOptions().hwSpecularEnvironmentMethod = SPECULAR_ENVIRONMENT_FIS;
    genContext.getOptions().hwDirectionalAlbedoMethod = DIRECTIONAL_ALBEDO_TABLE;
    genContext.getOptions().hwShadowMap = true;
    genContext.getOptions().targetColorSpaceOverride = "lin_rec709";
    genContext.getOptions().fileTextureVerticalFlip = true;
    return genContext;
}

// Helper function to determine which materials to bake from renderable paths
StringVec getRenderablePaths(DocumentPtr& doc)
{
    StringVec renderablePaths;
    std::vector<TypedElementPtr> elems;
    std::vector<TypedElementPtr> materials;
    findRenderableElements(doc, elems);

    if (elems.empty())
    {
        return StringVec();
    }
    for (TypedElementPtr elem : elems)
    {
        TypedElementPtr renderableElem = elem;
        NodePtr node = elem->asA<Node>();
        if (node && node->getType() == MATERIAL_TYPE_STRING)
        {
            std::vector<NodePtr> shaderNodes = getShaderNodes(node, SURFACE_SHADER_TYPE_STRING);
            if (!shaderNodes.empty())
            {
                renderableElem = shaderNodes[0];
            }
            materials.push_back(node);
        }
        else
        {
            ShaderRefPtr shaderRef = elem->asA<ShaderRef>();
            TypedElementPtr materialRef = (shaderRef ? shaderRef->getParent()->asA<TypedElement>() : nullptr);
            materials.push_back(materialRef);
        }
        renderablePaths.push_back(renderableElem->getNamePath());
    }
    return renderablePaths;
}

// Helper function to generate texture filenames
FilePath generateOutTextureName(string file)
{
    FilePath origFile = FilePath(file);
    string extension = origFile.getExtension();
    origFile.removeExtension();
    string outStr = origFile.getBaseName() + "_baked." + extension;
    FilePath out = FilePath(outStr);
    return out;
}

void TextureBaker::bakeAndSave(DocumentPtr& doc, string file, bool hdr, int texres)
{
    TextureBakerPtr baker = TextureBaker::create(texres, texres, hdr ? Image::BaseType::FLOAT : Image::BaseType::UINT8);
    FileSearchPath filename = initFileSearchPath();
    GenContext genContext = initGenContext();
    genContext.registerSourceCodeSearchPath(filename);
    StringVec renderablePaths = getRenderablePaths(doc);
    StringResolverPtr resolver = StringResolver::create();
    for (const auto& renderablePath : renderablePaths)
    {
        ElementPtr elem = doc->getDescendant(renderablePath);
        TypedElementPtr typedElem = elem ? elem->asA<TypedElement>() : nullptr;
        if (!typedElem)
        {
            continue;
        }

        ShaderRefPtr sr = elem->asA<ShaderRef>();
        FilePath out = generateOutTextureName(file);
        StbImageLoaderPtr stbimg = StbImageLoader::create();
        ImageHandlerPtr imageHandler = GLTextureHandler::create(stbimg);

        // Check for any udim set.
        ValuePtr udimSetValue = doc->getGeomPropValue("udimset");

        if (udimSetValue && udimSetValue->isA<StringVec>())
        {
            for (const string& udim : udimSetValue->asA<StringVec>())
            {
                ShaderPtr hwShader = createShader("Shader", genContext, elem);
                if (!hwShader)
                {
                    return;
                }
                imageHandler->setSearchPath(filename);
                resolver->setUdimString(udim);
                imageHandler->setFilenameResolver(resolver);
                baker->setImageHandler(imageHandler);
                baker->bakeShaderInputs(sr, genContext, out.getParentPath(), udim);
            }
        }
        else
        {
            ShaderPtr hwShader = createShader("Shader", genContext, elem);
            if (!hwShader) 
            {
                return;
            }
            imageHandler->setSearchPath(filename);
            imageHandler->setFilenameResolver(resolver);
            baker->setImageHandler(imageHandler);
            baker->bakeShaderInputs(sr, genContext, out.getParentPath());
        }
        baker->writeBakedDocument(sr, out, udimSetValue);
    }
}

} // namespace MaterialX
