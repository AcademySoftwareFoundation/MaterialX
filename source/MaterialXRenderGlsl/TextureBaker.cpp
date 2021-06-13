//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRenderGlsl/TextureBaker.h>

#include <MaterialXRender/OiioImageLoader.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/Util.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>

#include <MaterialXFormat/XmlIo.h>

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
    if (type == "vector2")
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
    _distanceUnit("meter"),
    _averageImages(false),
    _optimizeConstants(true),
    _bakedGraphName("NG_baked"),
    _bakedGeomInfoName("GI_baked"),
    _outputStream(&std::cout),
    _hashImageNames(false),
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

    // Initialize our base renderer.
    initialize();

    // Initialize our image handler.
    _imageHandler = GLTextureHandler::create(StbImageLoader::create());
#if MATERIALX_BUILD_OIIO
    _imageHandler->addLoader(OiioImageLoader::create());
#endif

    // Create our dedicated frame capture image.
    _frameCaptureImage = Image::create(width, height, 4, baseType);
    _frameCaptureImage->createResourceBuffer();
}

FilePath TextureBaker::generateTextureFilename(OutputPtr output, const string& shaderName, const string& udim)
{
    string outputName = createValidName(output->getNamePath());
    string shaderSuffix = shaderName.empty() ? EMPTY_STRING : "_" + shaderName;
    string udimSuffix = udim.empty() ? EMPTY_STRING : "_" + udim;
    std::string bakedImageName; 
    bakedImageName = outputName + shaderSuffix + BAKED_POSTFIX + udimSuffix;
    if (_hashImageNames)
    {
        std::stringstream hashStream;
        hashStream << std::hash<std::string>{}(bakedImageName);
        bakedImageName = hashStream.str();
    }
    return FilePath(bakedImageName + "." + _extension);
}

bool TextureBaker::writeBakedImage(const BakedImage& baked, ImagePtr image)
{
    if (!_imageHandler->saveImage(baked.filename, image, true))
    {
        if (_outputStream)
        {
            *_outputStream << "Failed to write baked image: " << baked.filename.asString() << std::endl;
        }
        return false;
    }

    if (_outputStream)
    {
        *_outputStream << "Wrote baked image: " << baked.filename.asString() << std::endl;
    }

    return true;
}

void TextureBaker::bakeShaderInputs(NodePtr material, NodePtr shader, GenContext& context, const string& udim)
{
    _material = material;
    
    if (!shader)
    {
        return;
    }

    std::set<OutputPtr> bakedOutputs;
    for (InputPtr input : shader->getInputs())
    {
        OutputPtr output = input->getConnectedOutput();
        if (output && !bakedOutputs.count(output))
        {
            bakedOutputs.insert(output);

            // When possible, nodes with world-space outputs are applied outside of the baking process.
            NodePtr worldSpaceNode = connectsToWorldSpaceNode(output);
            if (worldSpaceNode)
            {
                output->setConnectedNode(worldSpaceNode->getConnectedNode("in"));
                _worldSpaceNodes[input->getName()] = worldSpaceNode;
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

    // Render and capture the requested image.
    renderTextureSpace();
    captureImage(_frameCaptureImage);

    // Construct a baked image record.
    BakedImage baked;
    baked.filename = texturefilepath;
    if (_averageImages)
    {
        baked.uniformColor = _frameCaptureImage->getAverageColor();
        baked.isUniform = true;
    }
    else if (_frameCaptureImage->isUniformColor(&baked.uniformColor))
    {
        baked.isUniform = true;
    }
    _bakedImageMap[output].push_back(baked);

    // Write non-uniform images to disk.
    if (!baked.isUniform)
    {
        writeBakedImage(baked, _frameCaptureImage);
    }
}

void TextureBaker::optimizeBakedTextures(NodePtr shader)
{
    if (!shader)
    {
        return;
    }

    // Check for fully uniform outputs.
    for (auto& pair : _bakedImageMap)
    {
        bool outputIsUniform = true;
        for (BakedImage& baked : pair.second)
        {
            if (!baked.isUniform || baked.uniformColor != pair.second[0].uniformColor)
            {
                outputIsUniform = false;
                continue;
            }
        }
        if (outputIsUniform)
        {
            BakedConstant bakedConstant;
            bakedConstant.color = pair.second[0].uniformColor;
            _bakedConstantMap[pair.first] = bakedConstant;
        }
    }

    // Check for uniform outputs at their default values.
    NodeDefPtr shaderNodeDef = shader->getNodeDef();
    if (shaderNodeDef)
    {
        for (InputPtr shaderInput : shader->getInputs())
        {
            OutputPtr output = shaderInput->getConnectedOutput();
            if (output && _bakedConstantMap.count(output))
            {
                InputPtr input = shaderNodeDef->getInput(shaderInput->getName());
                if (input)
                {
                    Color4 uniformColor = _bakedConstantMap[output].color;
                    string uniformColorString = getValueStringFromColor(uniformColor, input->getType());
                    string defaultValueString = input->hasValue() ? input->getValue()->getValueString() : EMPTY_STRING;
                    if (uniformColorString == defaultValueString)
                    {
                        _bakedConstantMap[output].isDefault = true;
                    }
                }
            }
        }
    }

    // Remove baked images that have been replaced by constant values.
    for (auto& pair : _bakedConstantMap)
    {
        if (pair.second.isDefault || _optimizeConstants || _averageImages)
        {
            _bakedImageMap.erase(pair.first);
        }
    }
}

DocumentPtr TextureBaker::bakeMaterial(NodePtr shader, const StringVec& udimSet)
{
    if (!shader)
    {
        return nullptr;
    }

    // Create document.
    DocumentPtr bakedTextureDoc = createDocument();
    if (shader->getDocument()->hasColorSpace())
    {
        bakedTextureDoc->setColorSpace(shader->getDocument()->getColorSpace());
    }

    // Create node graph and geometry info.
    NodeGraphPtr bakedNodeGraph;
    if (!_bakedImageMap.empty())
    {
        _bakedGraphName = bakedTextureDoc->createValidChildName(_bakedGraphName);
        bakedNodeGraph = bakedTextureDoc->addNodeGraph(_bakedGraphName);
        bakedNodeGraph->setColorSpace(_colorSpace);
    }
    _bakedGeomInfoName = bakedTextureDoc->createValidChildName(_bakedGeomInfoName);
    GeomInfoPtr bakedGeom = !udimSet.empty() ? bakedTextureDoc->addGeomInfo(_bakedGeomInfoName) : nullptr;
    if (bakedGeom)
    {
        bakedGeom->setGeomPropValue("udimset", udimSet, "stringarray");
    }

    // Create a shader node.
    NodePtr bakedShader = bakedTextureDoc->addNode(shader->getCategory(), shader->getName() + BAKED_POSTFIX, shader->getType());

    // Optionally create a material node, connecting it to the new shader node.
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
                    bakedMaterialInput = bakedMaterial->addInput(sourceMaterialInputName, sourceMaterialInput->getType());
                }
                bakedMaterialInput->setNodeName(bakedShader->getName());
            }
        }
    }

    // Create and connect inputs on the new shader node.
    for (ValueElementPtr valueElem : shader->getChildrenOfType<ValueElement>())
    {
        // Get the source input and its connected output.
        InputPtr sourceInput = valueElem->asA<Input>();
        if (!sourceInput)
        {
            continue;
        }
        OutputPtr output = sourceInput->getConnectedOutput();

        // Skip uniform outputs at their default values.
        if (output && _bakedConstantMap.count(output) && _bakedConstantMap[output].isDefault)
        {
            continue;
        }

        // Find or create the baked input.
        const std::string& sourceName = sourceInput->getName();
        const std::string& sourceType = sourceInput->getType();
        InputPtr bakedInput = bakedShader->getInput(sourceName);
        if (!bakedInput)
        {
            bakedInput = bakedShader->addInput(sourceName, sourceType);
        }

        // Assign image or constant data to the baked input.
        if (output)
        {
            // Store a constant value for uniform outputs.
            if (_optimizeConstants && _bakedConstantMap.count(output))
            {
                Color4 uniformColor = _bakedConstantMap[output].color;
                string uniformColorString = getValueStringFromColor(uniformColor, bakedInput->getType());
                bakedInput->setValueString(uniformColorString);
                if (bakedInput->getType() == "color3" || bakedInput->getType() == "color4")
                {
                    bakedInput->setColorSpace(_colorSpace);
                }
                continue;
            }

            if (!_bakedImageMap.empty())
            {
                // Add the image node.
                NodePtr bakedImage = bakedNodeGraph->addNode("image", sourceName + BAKED_POSTFIX, sourceType);
                InputPtr input = bakedImage->addInput("file", "filename");
                input->setValueString(generateTextureFilename(output, shader->getName(), udimSet.empty() ? EMPTY_STRING : UDIM_TOKEN));

                // Reconstruct any world-space nodes that were excluded from the baking process.
                auto worldSpacePair = _worldSpaceNodes.find(sourceInput->getName());
                if (worldSpacePair != _worldSpaceNodes.end())
                {
                    NodePtr origWorldSpaceNode = worldSpacePair->second;
                    if (origWorldSpaceNode)
                    {
                        NodePtr newWorldSpaceNode = bakedNodeGraph->addNode(origWorldSpaceNode->getCategory(), sourceName + BAKED_POSTFIX + "_map", sourceType);
                        newWorldSpaceNode->copyContentFrom(origWorldSpaceNode);
                        InputPtr mapInput = newWorldSpaceNode->getInput("in");
                        if (mapInput)
                        {
                            mapInput->setNodeName(bakedImage->getName());
                        }
                        bakedImage = newWorldSpaceNode;
                    }
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

    // Generate uniform images and write to disk.
    ImagePtr uniformImage = createUniformImage(4, 4, 4, _baseType, Color4());
    for (const auto& pair : _bakedImageMap)
    {
        for (const BakedImage& baked : pair.second)
        {
            if (baked.isUniform)
            {
                uniformImage->setUniformColor(baked.uniformColor);
                writeBakedImage(baked, uniformImage);
            }
        }
    }

    // Clear cached information after each material bake
    _bakedImageMap.clear();
    _bakedConstantMap.clear();
    _worldSpaceNodes.clear();
    _material = nullptr;

    // Return the baked document on success.
    return bakedTextureDoc;
}

BakedDocumentVec TextureBaker::createBakeDocuments(DocumentPtr doc, const FileSearchPath& searchPath)
{
    GenContext genContext(_generator);
    genContext.getOptions().targetColorSpaceOverride = LIN_REC709;
    genContext.getOptions().fileTextureVerticalFlip = true;
    genContext.getOptions().targetDistanceUnit = _distanceUnit;

    DefaultColorManagementSystemPtr cms = DefaultColorManagementSystem::create(genContext.getShaderGenerator().getTarget());
    cms->loadLibrary(doc);
    for (const FilePath& path : searchPath)
    {
        genContext.registerSourceCodeSearchPath(path / "libraries");
    }
    genContext.getShaderGenerator().setColorManagementSystem(cms);
    StringResolverPtr resolver = StringResolver::create();
    StringVec renderablePaths = getRenderablePaths(doc);

    BakedDocumentVec bakedDocuments;
    for (const string& renderablePath : renderablePaths)
    {
        ElementPtr elem = doc->getDescendant(renderablePath);
        if (!elem || !elem->isA<Node>())
        {
            continue;
        }
        NodePtr materialNode = elem->asA<Node>();

        vector<NodePtr> shaderNodes = getShaderNodes(materialNode);
        NodePtr shaderNode = shaderNodes.empty() ? nullptr : shaderNodes[0];
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
            _imageHandler->setSearchPath(searchPath);
            resolver->setUdimString(tag);
            _imageHandler->setFilenameResolver(resolver);
            bakeShaderInputs(materialNode, shaderNode, genContext, tag);
        }

        // Optimize baked textures.
        optimizeBakedTextures(shaderNode);

        // Write the baked material and textures.
        DocumentPtr bakedMaterialDoc = bakeMaterial(shaderNode, udimSet);
        bakedDocuments.push_back(std::make_pair(shaderNode->getName(), bakedMaterialDoc));
    }

    return bakedDocuments;
}

void TextureBaker::bakeAllMaterials(DocumentPtr doc, const FileSearchPath& searchPath, const FilePath& outputFilename)
{
    if (_outputImagePath.isEmpty())
    {
        _outputImagePath = outputFilename.getParentPath();
        if (!_outputImagePath.exists())
        {
            _outputImagePath.createDirectory();
        }
    }

    BakedDocumentVec bakedDocuments = createBakeDocuments(doc, searchPath);
    size_t bakeCount = bakedDocuments.size();
    if (bakeCount == 1)
    {
        if (bakedDocuments[0].second)
        {
            writeToXmlFile(bakedDocuments[0].second, outputFilename);
            if (_outputStream)
            {
                *_outputStream << "Wrote baked document: " << outputFilename.asString() << std::endl;
            }
        }
    }
    else
    {
        // Add additional filename decorations if there are multiple documents.
        for (size_t i = 0; i < bakeCount; i++)
        {
            if (bakedDocuments[i].second)
            {
                FilePath writeFilename = outputFilename;
                const std::string extension = writeFilename.getExtension();
                writeFilename.removeExtension();
                writeFilename = FilePath(writeFilename.asString() + "_" + bakedDocuments[i].first + "." + extension);
                writeToXmlFile(bakedDocuments[i].second, writeFilename);
                if (_outputStream)
                {
                    *_outputStream << "Wrote baked document: " << writeFilename.asString() << std::endl;
                }
            }
        }
    }
}

void TextureBaker::setupUnitSystem(DocumentPtr unitDefinitions)
{
    UnitTypeDefPtr distanceTypeDef = unitDefinitions ? unitDefinitions->getUnitTypeDef("distance") : nullptr;
    UnitTypeDefPtr angleTypeDef = unitDefinitions ? unitDefinitions->getUnitTypeDef("angle") : nullptr;
    if (!distanceTypeDef && !angleTypeDef)
    {
        return;
    }

    UnitSystemPtr unitSystem = UnitSystem::create(_generator->getTarget());
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
