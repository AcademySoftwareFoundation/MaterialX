//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/TextureBaker.h>

#include <MaterialXCore/MaterialNode.h>
#include <MaterialXRenderGlsl/GlslProgram.h>
#include <MaterialXRender/Util.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>

#include <iostream>

namespace MaterialX
{

namespace
{

// Helper function to initialize file search path
FileSearchPath initFileSearchPath()
{
    FilePath installSearchPath = FilePath::getModulePath().getParentPath();
    FilePath devSearchPath = FilePath(__FILE__).getParentPath().getParentPath().getParentPath();
    FileSearchPath searchPath = FileSearchPath(installSearchPath);
    searchPath.append(installSearchPath);

    if (!devSearchPath.isEmpty() && devSearchPath.exists())
    {
        searchPath.append(devSearchPath);
        devSearchPath = devSearchPath / "libraries";
        if (devSearchPath.exists())
        {
            searchPath.append(devSearchPath);
        }
    }
    return searchPath;
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
            std::unordered_set<NodePtr> shaderNodes = getShaderNodes(node, SURFACE_SHADER_TYPE_STRING);
            if (!shaderNodes.empty())
            {
                renderableElem = *shaderNodes.begin();
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

// Helper function to generate mtlx filenames
FilePath generateOutMtlxFilename(string file, string srName)
{
    FilePath origFile = FilePath(file);
    string extension = origFile.getExtension();
    origFile.removeExtension();
    string outStr = origFile.getBaseName() + "_" + srName + "_baked." + extension;
    FilePath out = FilePath(outStr);
    return out;
}

} // anonymous namespace

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

void TextureBaker::bakeShaderInputs(ConstShaderRefPtr shaderRef, GenContext& context, const FilePath& outputFolder, const string& udim)
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
            FilePath filename = FilePath(outputFolder / generateTextureFilename(output, shaderRef->getName(), udim));
            bakeGraphOutput(output, context, filename);
        }
    }
}

void TextureBaker::bakeShaderInputs(NodePtr shader, GenContext& context, const FilePath& outputFolder, const string& udim)
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
            FilePath filename = FilePath(outputFolder / generateTextureFilename(output, shader->getName(), udim));
            bakeGraphOutput(output, context, filename);
        }
    }
}

void TextureBaker::bakeGraphOutput(OutputPtr output, GenContext& context, const FilePath& filename)
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

    save(filename);
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
            param->setValueString(generateTextureFilename(output, shaderRef->getName(), (udimSetValue) ? UDIM_TOKEN : EMPTY_STRING));

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
            param->setValueString(generateTextureFilename(output, shader->getName(), (udimSetValue) ? UDIM_TOKEN : EMPTY_STRING));

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

FilePath TextureBaker::generateTextureFilename(OutputPtr output, const string& srName, const string& udim)
{
    string outputName = createValidName(output->getNamePath());
    string srSegment = srName.empty() ? EMPTY_STRING : "_" + srName;
    string udimSuffix = udim.empty() ? EMPTY_STRING : "_" + udim;

    return FilePath(outputName + srSegment + "_baked" + udimSuffix + "." + _extension);
}

void TextureBaker::bakeAllShaders(DocumentPtr& doc, string file, bool hdr, int texresx, int texresy)
{
    TextureBakerPtr baker = TextureBaker::create(texresx, texresy, hdr ? Image::BaseType::FLOAT : Image::BaseType::UINT8);
    FileSearchPath filename = initFileSearchPath();
    GenContext genContext = initGenContext();
    genContext.registerSourceCodeSearchPath(filename);
    StringResolverPtr resolver = StringResolver::create();
    StbImageLoaderPtr stbimg = StbImageLoader::create();
    ImageHandlerPtr imageHandler = GLTextureHandler::create(stbimg);
    StringVec renderablePaths = getRenderablePaths(doc);

    for (const auto& renderablePath : renderablePaths)
    {
        ElementPtr elem = doc->getDescendant(renderablePath);
        TypedElementPtr typedElem = elem ? elem->asA<TypedElement>() : nullptr;
        ShaderRefPtr sr = typedElem ? typedElem->asA<ShaderRef>() : nullptr;
        if (!sr)
        {
            continue;
        }
        FilePath out = generateOutMtlxFilename(file, sr->getName());

        // Check for any udim set.
        ValuePtr udimSetValue = doc->getGeomPropValue("udimset");
        StringVec udims;
        if (udimSetValue && udimSetValue->isA<StringVec>())
        {
            udims = udimSetValue->asA<StringVec>();
        }
        else
        {
            udims.push_back(EMPTY_STRING);
        }   
        for (const string& udim : udims)
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
            std::cout << "Baked out shader inputs for " << sr->getName() << " " << udim << std::endl;
        }
        baker->writeBakedDocument(sr, out, udimSetValue);
        std::cout << "Wrote out " << out.asString() << std::endl;
    }
}

} // namespace MaterialX
