//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/TextureBaker.h>

#include <MaterialXRender/OiioImageLoader.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/Util.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>

#include <iostream>

namespace MaterialX
{

namespace {

string SRGB_TEXTURE = "srgb_texture";
string LIN_REC709 = "lin_rec709";

StringVec getRenderablePaths(ConstDocumentPtr doc)
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

string getValueStringFromColor(const Color4& color, const string& type)
{
    if (type == "color4" || type == "vector4")
    {
        return toValueString(color);
    }
    if (type == "color3" || type == "vector3")
    {
        return toValueString(Vector3(color[0], color[1], color[2]));
    }
    if (type == "color2" || type == "vector2")
    {
        return toValueString(Vector2(color[0], color[1]));
    }
    if (type == "float")
    {
        return toValueString(color[0]);
    }
    return EMPTY_STRING;
}

} // anonymous namespace

TextureBaker::TextureBaker(unsigned int width, unsigned int height, Image::BaseType baseType) :
    GlslRenderer(width, height, baseType),
    _optimizeConstants(true),
    _generator(GlslShaderGenerator::create())
{
    if (baseType == Image::BaseType::UINT8)
    {
#if MATERIALX_BUILD_OIIO
        _extension = ImageLoader::TIFF_EXTENSION;
#else
        _extension = ImageLoader::PNG_EXTENSION;
#endif
        _colorSpace = SRGB_TEXTURE;
    }
    else
    {
#if MATERIALX_BUILD_OIIO
        _extension = ImageLoader::EXR_EXTENSION;
#else
        _extension = ImageLoader::HDR_EXTENSION;
#endif
        _colorSpace = LIN_REC709;
    }
    initialize();
}

FilePath TextureBaker::generateTextureFilename(OutputPtr output, const string& shaderRefName, const string& udim)
{
    string outputName = createValidName(output->getNamePath());
    string shaderRefSuffix = shaderRefName.empty() ? EMPTY_STRING : "_" + shaderRefName;
    string udimSuffix = udim.empty() ? EMPTY_STRING : "_" + udim;

    return FilePath(outputName + shaderRefSuffix + "_baked" + udimSuffix + "." + _extension);
}

void TextureBaker::bakeShaderInputs(ConstShaderRefPtr shaderRef, GenContext& context, const FilePath& outputFolder, const string& udim)
{
    if (!shaderRef)
    {
        return;
    }

    _shaderRef = shaderRef;
    std::set<OutputPtr> bakedOutputs;

    for (BindInputPtr bindInput : _shaderRef->getBindInputs())
    {
        OutputPtr output = bindInput->getConnectedOutput();
        if (output && !bakedOutputs.count(output))
        {
            bakedOutputs.insert(output);
            if (connectsToNormalMapNode(output))
            {
                NodePtr normalMapNode = output->getParent()->getChild(output->getNodeName())->asA<Node>();
                InputPtr nodeInput = normalMapNode->getInput("in");
                if (nodeInput)
                {
                    output->setNodeName(nodeInput->getNodeName());
                }
                _worldSpaceShaderInputs.insert(bindInput->getName());
            }
            FilePath filename = FilePath(outputFolder / generateTextureFilename(output, _shaderRef->getName(), udim));
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

    bool encodeSrgb = _colorSpace == SRGB_TEXTURE &&
                      (output->getType() == "color3" || output->getType() == "color4");
    getFrameBuffer()->setEncodeSrgb(encodeSrgb);

    renderTextureSpace();

    BakedImage baked;
    baked.image = captureImage();
    baked.filename = filename;
    _bakedImageMap[output].push_back(baked);
}

void TextureBaker::optimizeBakedTextures()
{
    for (auto& pair : _bakedImageMap)
    {
        for (BakedImage& baked : pair.second)
        {
            baked.isUniform = baked.image->isUniformColor(&baked.uniformColor);
        }
        if (!pair.second.empty())
        {
            bool outputIsUniform = true;
            for (BakedImage& baked : pair.second)
            {
                if (!baked.isUniform || baked.uniformColor != pair.second[0].uniformColor)
                {
                    outputIsUniform = false;
                    break;
                }
            }
            if (outputIsUniform)
            {
                _bakedConstantMap[pair.first] = pair.second[0].uniformColor;
                if (!_optimizeConstants)
                {
                    for (BakedImage& baked : pair.second)
                    {
                        baked.image = createUniformImage(1, 1, baked.image->getChannelCount(), baked.image->getBaseType(), baked.uniformColor);
                    }
                }
            }
        }
    }
}

void TextureBaker::writeBakedMaterial(const FilePath& filename, const StringVec& udimSet)
{
    if (!_shaderRef)
    {
        return;
    }
    NodeDefPtr shaderNodeDef = _shaderRef->getNodeDef();

    // Create document.
    DocumentPtr bakedTextureDoc = createDocument();
    bakedTextureDoc->setColorSpace(_colorSpace);

    // Create top-level elements.
    NodeGraphPtr bakedNodeGraph = bakedTextureDoc->addNodeGraph("NG_baked");
    GeomInfoPtr bakedGeom = !udimSet.empty() ? bakedTextureDoc->addGeomInfo("GI_baked") : nullptr;
    if (bakedGeom)
    {
        bakedGeom->setGeomPropValue("udimset", udimSet, "stringarray");
    }
    MaterialPtr bakedMaterial = bakedTextureDoc->addMaterial("M_baked");
    ShaderRefPtr bakedShaderRef = bakedMaterial->addShaderRef(_shaderRef->getName() + "_baked", _shaderRef->getAttribute("node"));

    // Create bind elements on the baked shader reference.
    for (ValueElementPtr valueElem : _shaderRef->getChildrenOfType<ValueElement>())
    {
        BindInputPtr bindInput = valueElem->asA<BindInput>();
        OutputPtr output = bindInput ? bindInput->getConnectedOutput() : nullptr;
        if (output)
        {
            // Check for a constant default value.
            if (_bakedConstantMap.count(output) && shaderNodeDef)
            {
                InputPtr input = shaderNodeDef->getActiveInput(bindInput->getName());
                if (input)
                {
                    Color4 uniformColor = _bakedConstantMap[output];
                    string uniformColorString = getValueStringFromColor(uniformColor, input->getType());
                    if (uniformColorString == input->getValueString())
                    {
                        _bakedImageMap.erase(output);
                        continue;
                    }
                }
            }

            // Create the baked bindinput.
            BindInputPtr bakedBindInput = bakedShaderRef->addBindInput(bindInput->getName(), bindInput->getType());

            // Store a constant value for uniform outputs.
            if (_optimizeConstants && _bakedConstantMap.count(output))
            {
                Color4 uniformColor = _bakedConstantMap[output];
                string uniformColorString = getValueStringFromColor(uniformColor, bakedBindInput->getType());
                bakedBindInput->setValueString(uniformColorString);
                continue;
            }

            // Add the image node.
            NodePtr bakedImage = bakedNodeGraph->addNode("image", bindInput->getName() + "_baked", bindInput->getType());
            ParameterPtr param = bakedImage->addParameter("file", "filename");
            param->setValueString(generateTextureFilename(output, _shaderRef->getName(), udimSet.empty() ? EMPTY_STRING : UDIM_TOKEN));

            // Check if is a normal node and transform normals into world space
            if (_worldSpaceShaderInputs.count(bindInput->getName()))
            {
                NodePtr bakedImageOrig = bakedImage;
                bakedImage = bakedNodeGraph->addNode("normalmap", bindInput->getName() + "_baked_map", bindInput->getType());
                InputPtr mapInput = bakedImage->addInput("in", bindInput->getType());
                mapInput->setNodeName(bakedImageOrig->getName());
            }

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

    // Write referenced baked images.
    for (const auto& pair : _bakedImageMap)
    {
        if (_optimizeConstants && _bakedConstantMap.count(pair.first))
        {
            continue;
        }
        for (const BakedImage& baked : pair.second)
        {
            if (_imageHandler->saveImage(baked.filename, baked.image, true))
            {
                std::cout << "Wrote baked image: " << baked.filename.asString() << std::endl;
            }
            else
            {
                std::cout << "Failed to write baked image: " << baked.filename.asString() << std::endl;
            }
        }
    }

    // Write baked document.
    writeToXmlFile(bakedTextureDoc, filename);
    std::cout << "Wrote baked document: " << filename.asString() << std::endl;
}

void TextureBaker::bakeAllMaterials(DocumentPtr doc, const FileSearchPath& imageSearchPath, const FilePath& outputFilename)
{
    GenContext genContext = GlslShaderGenerator::create();
    genContext.getOptions().hwSpecularEnvironmentMethod = SPECULAR_ENVIRONMENT_FIS;
    genContext.getOptions().hwDirectionalAlbedoMethod = DIRECTIONAL_ALBEDO_TABLE;
    genContext.getOptions().hwShadowMap = true;
    genContext.getOptions().targetColorSpaceOverride = LIN_REC709;
    genContext.getOptions().fileTextureVerticalFlip = true;

    DefaultColorManagementSystemPtr cms = DefaultColorManagementSystem::create(genContext.getShaderGenerator().getLanguage());
    cms->loadLibrary(doc);
    genContext.registerSourceCodeSearchPath(getDefaultSearchPath());
    genContext.getShaderGenerator().setColorManagementSystem(cms);
    StringResolverPtr resolver = StringResolver::create();
    ImageHandlerPtr imageHandler = GLTextureHandler::create(StbImageLoader::create());
#if MATERIALX_BUILD_OIIO
    imageHandler->addLoader(OiioImageLoader::create());
#endif
    StringVec renderablePaths = getRenderablePaths(doc);

    for (const string& renderablePath : renderablePaths)
    {
        ElementPtr elem = doc->getDescendant(renderablePath);
        TypedElementPtr typedElem = elem ? elem->asA<TypedElement>() : nullptr;
        ShaderRefPtr shaderRef = typedElem ? typedElem->asA<ShaderRef>() : nullptr;
        if (!shaderRef)
        {
            continue;
        }

        FilePath writeFilename = outputFilename;
        if (renderablePaths.size() > 1)
        {
            string extension = writeFilename.getExtension();
            writeFilename.removeExtension();
            writeFilename = FilePath(writeFilename.asString() + "_" + shaderRef->getName() + "." + extension);
        }

        // Compute the UDIM set.
        ValuePtr udimSetValue = doc->getGeomPropValue("udimset");
        StringVec udimSet;
        if (udimSetValue && udimSetValue->isA<StringVec>())
        {
            udimSet = udimSetValue->asA<StringVec>();
        }

        // Compute the material tag set.
        StringVec materialTags = udimSet;
        if (materialTags.empty())
        {
            materialTags.push_back(EMPTY_STRING);
        }

        // Iterate over material tags.
        for (const string& tag : materialTags)
        {
            ShaderPtr hwShader = createShader("Shader", genContext, elem);
            if (!hwShader)
            {
                continue;
            }
            imageHandler->setSearchPath(imageSearchPath);
            resolver->setUdimString(tag);
            imageHandler->setFilenameResolver(resolver);
            setImageHandler(imageHandler);
            bakeShaderInputs(shaderRef, genContext, writeFilename.getParentPath(), tag);
        }

        // Optimize baked textures.
        optimizeBakedTextures();

        // Write the baked material and textures.
        writeBakedMaterial(writeFilename, udimSet);
    }
}

} // namespace MaterialX
