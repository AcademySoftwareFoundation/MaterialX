//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/TextureBaker.h>
#include <MaterialXCore/MaterialNode.h>

#include <MaterialXRender/OiioImageLoader.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/Util.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>

#include <iostream>

namespace MaterialX
{

namespace {

const string SRGB_TEXTURE = "srgb_texture";
const string LIN_REC709 = "lin_rec709";
const string BAKED_POSTFIX = "_baked";

StringVec getRenderablePaths(ConstDocumentPtr doc)
{
    StringVec renderablePaths;
    std::vector<TypedElementPtr> elems;
    findRenderableElements(doc, elems);
    for (TypedElementPtr elem : elems)
    {
        renderablePaths.push_back(elem->getNamePath());
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
    _targetUnitSpace("meter"),
    _bakedGraphName("NG_baked"),
    _bakedGeomInfoName("GI_baked"),
    _averageImages(false),
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

FilePath TextureBaker::generateTextureFilename(OutputPtr output, const string& shaderName, const string& udim)
{
    string outputName = createValidName(output->getNamePath());
    string shaderSuffix = shaderName.empty() ? EMPTY_STRING : "_" + shaderName;
    string udimSuffix = udim.empty() ? EMPTY_STRING : "_" + udim;

    return FilePath(outputName + shaderSuffix + BAKED_POSTFIX + udimSuffix + "." + _extension);
}

void TextureBaker::bakeShaderInputs(NodePtr material, NodePtr shader, GenContext& context, const string& udim)
{
    _material = material;
    
    if (!shader)
    {
        return;
    }

    std::set<OutputPtr> bakedOutputs;
    StringSet categories;
    categories.insert("normalmap");

    for (InputPtr input : shader->getInputs())
    {
        OutputPtr output = input->getConnectedOutput();
        if (output && !bakedOutputs.count(output))
        {
            ElementPtr outputNode = output->getParent();
            if (outputNode && outputNode->isA<NodeGraph>())
            {
                NodeGraphPtr outputGraph = outputNode->asA<NodeGraph>();
                outputGraph->flattenSubgraphs();
            }

            bakedOutputs.insert(output);
            NodePtr normalMapNode = connectsToNodeOfCategory(output, categories);
            if (normalMapNode)
            {
                NodePtr sampleNode = output->getParent()->getChild(output->getNodeName())->asA<Node>();
                if (sampleNode == normalMapNode)
                {
                    output->setNodeName(sampleNode->getInput("in")->getNodeName());
                }
                _worldSpaceShaderInputs[input->getName()] = sampleNode;
            }
            FilePath texturefilepath = FilePath(_outputImagePath / generateTextureFilename(output, shader->getName(), udim));
            bakeGraphOutput(output, context, texturefilepath);
        }
    }

    // Unbind all images used to generate this set of shader inputs.
    _imageHandler->unbindImages();
}

void TextureBaker::bakeGraphOutput(OutputPtr output, GenContext& context, const FilePath& texturefilepath)
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
    baked.filename = texturefilepath;
    _bakedImageMap[output].push_back(baked);
}

void TextureBaker::optimizeBakedTextures(NodePtr shader)
{
    if (!shader)
    {
        return;
    }

    // Early exist if not optimizing
    if (!_optimizeConstants)
    {
        _bakedConstantMap.clear();
        return;
    }

    // If the graph used to create the texture has any of the following attributes
    // then it's value has changed from the original, and even if the image is a constant
    // it must not be optmized away.
    StringVec transformationAttributes;
    transformationAttributes.push_back(Element::COLOR_SPACE_ATTRIBUTE);
    transformationAttributes.push_back(ValueElement::UNIT_ATTRIBUTE);
    transformationAttributes.push_back(ValueElement::UNITTYPE_ATTRIBUTE);

    // Check for uniform images.
    for (auto& pair : _bakedImageMap)
    {
        bool outputIsUniform = true;
        OutputPtr outputPtr = pair.first;
        for (BakedImage& baked : pair.second)
        {
            if (hasElementAttributes(outputPtr, transformationAttributes))
            {
                outputIsUniform = false;
            }
            else if (_averageImages)
            {
                baked.uniformColor = baked.image->getAverageColor();
                baked.isUniform = true;
            }
            else if (baked.image->isUniformColor(&baked.uniformColor))
            {
                baked.image = createUniformImage(4, 4, baked.image->getChannelCount(), baked.image->getBaseType(), baked.uniformColor);
                baked.isUniform = true;
            }
            else
            {
                outputIsUniform = false;
            }
        }

        // Check for uniform outputs.
        if (outputIsUniform)
        {
            BakedConstant bakedConstant;
            bakedConstant.color = pair.second[0].uniformColor;
            _bakedConstantMap[pair.first] = bakedConstant;
        }
    }


    // Check for uniform outputs at their default values.
    NodeDefPtr shaderNodeDef = shader->getNodeDef();
    for (InputPtr shaderInput : shader->getInputs())
    {
        OutputPtr output = shaderInput->getConnectedOutput();
        if (output && _bakedConstantMap.count(output))
        {
            if (_bakedConstantMap.count(output) && shaderNodeDef)
            {
                InputPtr input = shaderNodeDef->getInput(shaderInput->getName());
                if (input)
                {
                    Color4 uniformColor = _bakedConstantMap[output].color;
                    string uniformColorString = getValueStringFromColor(uniformColor, input->getType());
                    if (uniformColorString == input->getValueString())
                    {
                        _bakedConstantMap[output].isDefault = true;
                        _bakedImageMap.erase(output);
                    }
                }
            }
        }
    }
}

DocumentPtr TextureBaker::getBakedMaterial(NodePtr shader, const StringVec& udimSet)
{
    if (!shader)
    {
        return nullptr;
    }
    NodeDefPtr shaderNodeDef = shader->getNodeDef();

    // Create document.
    DocumentPtr bakedTextureDoc = createDocument();
    bakedTextureDoc->setColorSpace(_colorSpace);

    // Create top-level elements. Note that the child names may not be what
    // was requested so member names must be updated here to reflect that.
    _bakedGraphName = bakedTextureDoc->createValidChildName(_bakedGraphName);
    NodeGraphPtr bakedNodeGraph = bakedTextureDoc->addNodeGraph(_bakedGraphName);
    _bakedGeomInfoName = bakedTextureDoc->createValidChildName(_bakedGeomInfoName);
    GeomInfoPtr bakedGeom = !udimSet.empty() ? bakedTextureDoc->addGeomInfo(_bakedGeomInfoName) : nullptr;
    if (bakedGeom)
    {
        bakedGeom->setGeomPropValue("udimset", udimSet, "stringarray");
    }
    NodePtr bakedShader = bakedTextureDoc->addNode(shader->getCategory(), shader->getName() + BAKED_POSTFIX, shader->getType());
    bakedNodeGraph->setColorSpace(_colorSpace);

    // Add a material node if any specified and connect it to the new shader node
    if (_material)
    {
        NodePtr bakedMaterial = bakedTextureDoc->addNode(_material->getCategory(), _material->getName() + BAKED_POSTFIX, _material->getType());
        for (auto sourceMaterialInput : _material->getInputs())
        {
            const string& sourceMaterialInputName = sourceMaterialInput->getName();
            NodePtr upstreamShader = sourceMaterialInput->getConnectedNode();
            if (upstreamShader && (upstreamShader->getNamePath() == shader->getNamePath()))
            {
                InputPtr bakedMaterialInput = bakedMaterial->getInput(sourceMaterialInputName);
                if (!bakedMaterialInput)
                {
                    bakedMaterialInput = bakedMaterial->addInput(sourceMaterialInputName, sourceMaterialInput->getType(), sourceMaterialInput->getIsUniform());
                }
                bakedMaterialInput->setNodeName(bakedShader->getName());
            }
        }
    }

    // Create inputs on baked shader and connected to baked images as required.
    for (ValueElementPtr valueElem : shader->getChildrenOfType<ValueElement>())
    {
        // Get source and destination inputs
        InputPtr sourceInput = valueElem->asA<Input>();
        if (!sourceInput)
        {
            continue;
        }
        const std::string& sourceName = sourceInput->getName();
        const std::string& sourceType = sourceInput->getType();
        InputPtr bakedInput = bakedShader->getInput(sourceName);
        if (!bakedInput)
        {
            bakedInput = bakedShader->addInput(sourceName, sourceType, sourceInput->getIsUniform());
        }

        OutputPtr output = sourceInput->getConnectedOutput();
        if (output)
        {
            // Skip uniform outputs at their default values.
            if (_bakedConstantMap.count(output) && _bakedConstantMap[output].isDefault)
            {
                continue;
            }

            // Store a constant value for uniform outputs.
            if (_optimizeConstants && _bakedConstantMap.count(output))
            {
                Color4 uniformColor = _bakedConstantMap[output].color;
                string uniformColorString = getValueStringFromColor(uniformColor, bakedInput->getType());
                bakedInput->setValueString(uniformColorString);
            }
            else
            {
                // Add the image node.
                NodePtr bakedImage = bakedNodeGraph->addNode("image", sourceName + BAKED_POSTFIX, sourceType);
                InputPtr input = bakedImage->addInput("file", "filename");
                input->setValueString(generateTextureFilename(output, shader->getName(), udimSet.empty() ? EMPTY_STRING : UDIM_TOKEN));

                // Check if is a normal node and transform normals into world space
                auto worldSpaceShaderInput = _worldSpaceShaderInputs.find(sourceInput->getName());
                if (worldSpaceShaderInput != _worldSpaceShaderInputs.end())
                {
                    NodePtr origNormalMapNode = worldSpaceShaderInput->second;
                    NodePtr normalMapNode = bakedNodeGraph->addNode("normalmap", sourceName + BAKED_POSTFIX + "_map", sourceType);
                    if (origNormalMapNode)
                    {
                        normalMapNode->copyContentFrom(origNormalMapNode);
                    }
                    InputPtr mapInput = normalMapNode->getInput("in");
                    if (!mapInput)
                    {
                        mapInput = normalMapNode->addInput("in", sourceType);
                    }
                    mapInput->setNodeName(bakedImage->getName());
                    bakedImage = normalMapNode;
                }

                // Add the graph output.
                OutputPtr bakedOutput = bakedNodeGraph->addOutput(sourceName + "_output", sourceType);
                bakedOutput->setConnectedNode(bakedImage);
                bakedInput->setConnectedOutput(bakedOutput);
            }
        }
        else
        {
            bakedInput->copyContentFrom(sourceInput);
        }
    }

    // Write referenced baked images.
    bool bakingSuccessful = true;
    for (const auto& pair : _bakedImageMap)
    {
        if (_optimizeConstants && _bakedConstantMap.count(pair.first))
        {
            continue;
        }
        for (const BakedImage& baked : pair.second)
        {
            if (!_imageHandler->saveImage(baked.filename, baked.image, true))
            {
                bakingSuccessful = false;
                std::cerr << "Failed to write baked image: " << baked.filename.asString() << std::endl;
            }
            else
            {
                std::cout << "Write baked image:" << baked.filename.asString() << std::endl;
            }
        }
    }

    // Clear cached information after each material bake
    _bakedImageMap.clear();
    _bakedConstantMap.clear();
    _worldSpaceShaderInputs.clear();
    _material = nullptr;

    if (bakingSuccessful)
        return bakedTextureDoc;
    else
        return nullptr;
}


ListofBakedDocuments TextureBaker::bakeAllMaterials(DocumentPtr doc, const FileSearchPath& imageSearchPath)
{
    GenContext genContext = GlslShaderGenerator::create();
    genContext.getOptions().hwSpecularEnvironmentMethod = SPECULAR_ENVIRONMENT_FIS;
    genContext.getOptions().hwDirectionalAlbedoMethod = DIRECTIONAL_ALBEDO_TABLE;
    genContext.getOptions().hwShadowMap = true;
    genContext.getOptions().targetColorSpaceOverride = LIN_REC709;
    genContext.getOptions().fileTextureVerticalFlip = true;
    genContext.getOptions().targetDistanceUnit = _targetUnitSpace;

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
    std::vector<NodePtr> renderableShaderNodes;

    ListofBakedDocuments bakedDocuments;
    for (const string& renderablePath : renderablePaths)
    {
        ElementPtr elem = doc->getDescendant(renderablePath);
        if (!elem)
        {
            continue;
        }
        NodePtr materialPtr = elem->asA<Node>();
        NodePtr shaderNode = nullptr;
        if (materialPtr)
        {
            std::unordered_set<NodePtr> shaderNodes = getShaderNodes(materialPtr);
            shaderNode = shaderNodes.empty() ? nullptr : *shaderNodes.begin();
        }
        if (!shaderNode)
        {
            continue;
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
            // Always clear any cached implementations before generation.
            genContext.clearNodeImplementations();

            ShaderPtr hwShader = createShader("Shader", genContext, shaderNode);
            if (!hwShader)
            {
                continue;
            }
            imageHandler->setSearchPath(imageSearchPath);
            resolver->setUdimString(tag);
            imageHandler->setFilenameResolver(resolver);
            setImageHandler(imageHandler);
            bakeShaderInputs(materialPtr, shaderNode, genContext, tag);

            // Optimize baked textures.
            optimizeBakedTextures(shaderNode);

            // Write the baked material and textures.
            DocumentPtr bakedMaterialDoc = getBakedMaterial(shaderNode, udimSet);
            bakedDocuments.push_back(std::make_pair(shaderNode->getName(), bakedMaterialDoc));
        }
    }

    return bakedDocuments;
}

void TextureBaker::setupUnitSystem(DocumentPtr unitDefinitions)
{
    UnitTypeDefPtr distanceTypeDef = unitDefinitions ? unitDefinitions->getUnitTypeDef("distance") : nullptr;
    UnitTypeDefPtr angleTypeDef = unitDefinitions ? unitDefinitions->getUnitTypeDef("angle") : nullptr;
    if (!distanceTypeDef && !angleTypeDef)
    {
        return;
    }

    UnitSystemPtr unitSystem = UnitSystem::create(_generator->getLanguage());
    if (!unitSystem)
    {
        return;
    }
    _generator->setUnitSystem(unitSystem);
    UnitConverterRegistryPtr registry = UnitConverterRegistry::create();
    registry->addUnitConverter(distanceTypeDef, LinearUnitConverter::create(distanceTypeDef));
    registry->addUnitConverter(angleTypeDef, LinearUnitConverter::create(angleTypeDef));
    _generator->getUnitSystem()->loadLibrary(unitDefinitions);
    _generator->getUnitSystem()->setUnitConverterRegistry(registry);
}

} // namespace MaterialX
