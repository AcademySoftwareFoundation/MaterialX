#include <MaterialXView/Viewer.h>

#include <MaterialXRenderGlsl/GLTextureHandler.h>
#include <MaterialXRenderGlsl/TextureBaker.h>

#include <MaterialXRender/Harmonics.h>
#include <MaterialXRender/OiioImageLoader.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/TinyObjLoader.h>
#include <MaterialXRender/Util.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/Shader.h>

#include <nanogui/button.h>
#include <nanogui/combobox.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/messagedialog.h>

#include <fstream>
#include <iostream>

const float PI = std::acos(-1.0f);

const int MIN_ENV_SAMPLES = 4;
const int MAX_ENV_SAMPLES = 1024;
const int DEFAULT_ENV_SAMPLES = 16;

const int SHADOW_MAP_SIZE = 2048;
const int IRRADIANCE_MAP_WIDTH = 256;
const int IRRADIANCE_MAP_HEIGHT = 128;

const std::string DIR_LIGHT_NODE_CATEGORY = "directional_light";
const std::string IRRADIANCE_MAP_FOLDER = "irradiance";

const float ENV_MAP_SPLIT_RADIANCE = 16.0f;
const float MAX_ENV_TEXEL_RADIANCE = 36000.0f;
const float IDEAL_ENV_MAP_RADIANCE = 5.2374f;

const float MODEL_SPHERE_RADIUS = 2.0f;

namespace {

bool stringEndsWith(const std::string& str, std::string const& end)
{
    if (str.length() >= end.length())
    {
        return !str.compare(str.length() - end.length(), end.length(), end);
    }
    return false;
}

void writeTextFile(const std::string& text, const std::string& filePath)
{
    std::ofstream file;
    file.open(filePath);
    file << text;
    file.close();
}

mx::DocumentPtr loadLibraries(const mx::FilePathVec& libraryFolders, const mx::FileSearchPath& searchPath)
{
    mx::DocumentPtr doc = mx::createDocument();
    for (const std::string& libraryFolder : libraryFolders)
    {
        mx::CopyOptions copyOptions;
        copyOptions.skipConflictingElements = true;

        mx::XmlReadOptions readOptions;
        readOptions.skipConflictingElements = true;

        mx::FilePath libraryPath = searchPath.find(libraryFolder);
        for (const mx::FilePath& filename : libraryPath.getFilesInDirectory(mx::MTLX_EXTENSION))
        {
            mx::DocumentPtr libDoc = mx::createDocument();
            mx::readFromXmlFile(libDoc, libraryPath / filename, mx::FileSearchPath(), &readOptions);
            doc->importLibrary(libDoc, &copyOptions);
        }
    }
    return doc;
}

void applyModifiers(mx::DocumentPtr doc, const DocumentModifiers& modifiers)
{
    for (mx::ElementPtr elem : doc->traverseTree())
    {
        if (modifiers.remapElements.count(elem->getCategory()))
        {
            elem->setCategory(modifiers.remapElements.at(elem->getCategory()));
        }
        if (modifiers.remapElements.count(elem->getName()))
        {
            elem->setName(modifiers.remapElements.at(elem->getName()));
        }
        mx::StringVec attrNames = elem->getAttributeNames();
        for (const std::string& attrName : attrNames)
        {
            if (modifiers.remapElements.count(elem->getAttribute(attrName)))
            {
                elem->setAttribute(attrName, modifiers.remapElements.at(elem->getAttribute(attrName)));
            }
        }
        if (elem->hasFilePrefix() && !modifiers.filePrefixTerminator.empty())
        {
            std::string filePrefix = elem->getFilePrefix();
            if (!stringEndsWith(filePrefix, modifiers.filePrefixTerminator))
            {
                elem->setFilePrefix(filePrefix + modifiers.filePrefixTerminator);
            }
        }
        std::vector<mx::ElementPtr> children = elem->getChildren();
        for (mx::ElementPtr child : children)
        {
            if (modifiers.skipElements.count(child->getCategory()) ||
                modifiers.skipElements.count(child->getName()))
            {
                elem->removeChild(child->getName());
            }
        }
    }

    // Remap references to unimplemented shader nodedefs.
    for (mx::MaterialPtr material : doc->getMaterials())
    {
        for (mx::ShaderRefPtr shaderRef : material->getShaderRefs())
        {
            mx::NodeDefPtr nodeDef = shaderRef->getNodeDef();
            if (nodeDef && !nodeDef->getImplementation())
            {
                std::vector<mx::NodeDefPtr> altNodeDefs = doc->getMatchingNodeDefs(nodeDef->getNodeString());
                for (mx::NodeDefPtr altNodeDef : altNodeDefs)
                {
                    if (altNodeDef->getImplementation())
                    {
                        shaderRef->setNodeDefString(altNodeDef->getName());
                    }
                }
            }
        }
    }
    for (mx::NodePtr materialNode : doc->getMaterialNodes())
    {
        for (mx::NodePtr shader : getShaderNodes(materialNode))
        {
            mx::NodeDefPtr nodeDef = shader->getNodeDef();
            if (nodeDef && !nodeDef->getImplementation())
            {
                std::vector<mx::NodeDefPtr> altNodeDefs = doc->getMatchingNodeDefs(nodeDef->getNodeString());
                for (mx::NodeDefPtr altNodeDef : altNodeDefs)
                {
                    if (altNodeDef->getImplementation())
                    {
                        shader->setNodeDefString(altNodeDef->getName());
                    }
                }
            }
        }
    }
}

} // anonymous namespace

//
// Viewer methods
//

Viewer::Viewer(const std::string& materialFilename,
               const std::string& meshFilename,
               const mx::FilePathVec& libraryFolders,
               const mx::FileSearchPath& searchPath,
               const DocumentModifiers& modifiers,
               mx::HwSpecularEnvironmentMethod specularEnvironmentMethod,
               const std::string& envRadiancePath,
               int multiSampleCount) :
    ng::Screen(ng::Vector2i(1280, 960), "MaterialXView",
        true, false,
        8, 8, 24, 8,
        multiSampleCount),
    _eye(0.0f, 0.0f, 5.0f),
    _up(0.0f, 1.0f, 0.0f),
    _viewAngle(45.0f),
    _nearDist(0.05f),
    _farDist(5000.0f),
    _modelZoom(1.0f),
    _userZoom(1.0f),
    _userTranslationActive(false),
    _userTranslationPixel(0, 0),
    _libraryFolders(libraryFolders),
    _searchPath(searchPath),
    _materialFilename(materialFilename),
    _modifiers(modifiers),
    _envRadiancePath(envRadiancePath),
    _directLighting(true),
    _indirectLighting(true),
    _normalizeEnvironment(false),
    _splitDirectLight(false),
    _generateReferenceIrradiance(false),
    _saveGeneratedLights(false),
    _shadowSoftness(1),
    _ambientOcclusionGain(0.6f),
    _meshFilename(meshFilename),
    _selectedGeom(0),
    _selectedMaterial(0),
    _lightHandler(mx::LightHandler::create()),
    _cameraViewHandler(mx::ViewHandler::create()),
    _shadowViewHandler(mx::ViewHandler::create()),
    _genContext(mx::GlslShaderGenerator::create()),
    _unitRegistry(mx::UnitConverterRegistry::create()),
    _splitByUdims(false),
    _mergeMaterials(false),
    _bakeTextures(false),
    _outlineSelection(false),
    _specularEnvironmentMethod(specularEnvironmentMethod),
    _envSamples(DEFAULT_ENV_SAMPLES),
    _drawEnvironment(false),
    _showAdvancedProperties(false),
    _captureFrame(false),
    _bakeRequested(false)
{
    _window = new ng::Window(this, "Viewer Options");
    _window->setPosition(ng::Vector2i(15, 15));
    _window->setLayout(new ng::GroupLayout());

    // Initialize the standard libraries and color/unit management.
    loadStandardLibraries();

    // Set default generator options.
    _genContext.getOptions().hwSpecularEnvironmentMethod = _specularEnvironmentMethod;
    _genContext.getOptions().hwShadowMap = true;
    _genContext.getOptions().targetColorSpaceOverride = "lin_rec709";
    _genContext.getOptions().fileTextureVerticalFlip = true;

    // Initialize image handler.
#if MATERIALX_BUILD_OIIO
    mx::ImageLoaderPtr imageLoader = mx::OiioImageLoader::create();
#else
    mx::ImageLoaderPtr imageLoader = mx::StbImageLoader::create();
#endif
    _imageHandler = mx::GLTextureHandler::create(imageLoader);

    // Initialize user interfaces.
    createLoadMeshInterface(_window, "Load Mesh");
    createLoadMaterialsInterface(_window, "Load Material");
    createLoadEnvironmentInterface(_window, "Load Environment");
    createPropertyEditorInterface(_window, "Property Editor");
    createAdvancedSettings(_window);

    // Create geometry selection box.
    _geomLabel = new ng::Label(_window, "Select Geometry");
    _geometrySelectionBox = new ng::ComboBox(_window, { "None" });
    _geometrySelectionBox->setChevronIcon(-1);
    _geometrySelectionBox->setCallback([this](int choice)
    {
        size_t index = (size_t) choice;
        if (index < _geometryList.size())
        {
            _selectedGeom = index;
            if (_materialAssignments.count(getSelectedGeometry()))
            {
                setSelectedMaterial(_materialAssignments[getSelectedGeometry()]);
            }
            updateDisplayedProperties();
            updateMaterialSelectionUI();
        }
    });

    // Create material selection box.
    _materialLabel = new ng::Label(_window, "Assigned Material");
    _materialSelectionBox = new ng::ComboBox(_window, { "None" });
    _materialSelectionBox->setChevronIcon(-1);
    _materialSelectionBox->setCallback([this](int choice)
    {
        size_t index = (size_t) choice;
        if (index < _materials.size())
        {
            // Update selected material index
            _selectedMaterial = index;

            // Assign selected material to geometry
            assignMaterial(getSelectedGeometry(), _materials[index]);
        }
    });

    // Create geometry handler.
    mx::TinyObjLoaderPtr loader = mx::TinyObjLoader::create();
    _geometryHandler = mx::GeometryHandler::create();
    _geometryHandler->addLoader(loader);
    _geometryHandler->loadGeometry(_searchPath.find(_meshFilename));

    // Create environment geometry handler.
    _envGeometryHandler = mx::GeometryHandler::create();
    _envGeometryHandler->addLoader(loader);
    mx::FilePath envSphere("resources/Geometry/sphere.obj");
    _envGeometryHandler->loadGeometry(_searchPath.find(envSphere));

    // Initialize environment light.
    loadEnvironmentLight();

    // Generate wireframe material.
    try
    {
        _wireMaterial = Material::create();
        _wireMaterial->generateConstantShader(_genContext, _stdLib, "__WIRE_SHADER__", mx::Color3(1.0f));
    }
    catch (std::exception& e)
    {
        _wireMaterial = nullptr;
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to generate wire shader", e.what());
    }

    // Generate shadow material.
    try
    {
        _shadowMaterial = Material::create();
        _shadowMaterial->generateDepthShader(_genContext, _stdLib, "__SHADOW_SHADER__");
    }
    catch (std::exception& e)
    {
        _shadowMaterial = nullptr;
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to generate shadow shader", e.what());
    }

    // Generate shadow blur material.
    try
    {
        _shadowBlurMaterial = Material::create();
        _shadowBlurMaterial->generateBlurShader(_genContext, _stdLib, "__SHADOW_BLUR_SHADER__", "gaussian", 1.0f);
    }
    catch (std::exception& e)
    {
        _shadowBlurMaterial = nullptr;
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to generate shadow blur shader", e.what());
    }

    // Initialize camera
    initCamera();
    setResizeCallback([this](ng::Vector2i size)
    {
        _arcball.setSize(size);
    });

    // Update geometry selections.
    updateGeometrySelections();

    // Load the requested material document.
    loadDocument(_materialFilename, _stdLib);

    // Finalize the UI.
    _propertyEditor.setVisible(false);
    performLayout();
}

void Viewer::loadEnvironmentLight()
{
    std::string message;

    // Load the requested radiance map.
    mx::ImagePtr envRadianceMap = _imageHandler->acquireImage(_searchPath.find(_envRadiancePath), true, nullptr, &message);
    if (!envRadianceMap)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to load environment light", message);
        return;
    }

    // If requested, normalize the environment upon loading.
    if (_normalizeEnvironment)
    {
        envRadianceMap = mx::normalizeEnvironment(envRadianceMap, IDEAL_ENV_MAP_RADIANCE, MAX_ENV_TEXEL_RADIANCE);
        if (_saveGeneratedLights)
        {
            _imageHandler->saveImage("NormalizedRadiance.hdr", envRadianceMap);
        }
    }

    // If requested, split the environment into indirect and direct components.
    if (_splitDirectLight)
    {
        splitDirectLight(envRadianceMap, envRadianceMap, _lightRigDoc);
        if (_saveGeneratedLights)
        {
            _imageHandler->saveImage("IndirectRadiance.hdr", envRadianceMap);
            mx::writeToXmlFile(_lightRigDoc, "DirectLightRig.mtlx");
        }
    }

    // Look for an irradiance map using an expected filename convention.
    mx::ImagePtr envIrradianceMap;
    if (!envIrradianceMap && !_normalizeEnvironment && !_splitDirectLight)
    {
        mx::FilePath envIrradiancePath = _envRadiancePath.getParentPath() / IRRADIANCE_MAP_FOLDER / _envRadiancePath.getBaseName();
        envIrradianceMap = _imageHandler->acquireImage(_searchPath.find(envIrradiancePath), true, nullptr, &message);
    }

    // If not found, then generate an irradiance map via spherical harmonics.
    if (!envIrradianceMap)
    {
        if (_generateReferenceIrradiance)
        {
            envIrradianceMap = mx::renderReferenceIrradiance(envRadianceMap, IRRADIANCE_MAP_WIDTH, IRRADIANCE_MAP_HEIGHT);
            if (_saveGeneratedLights)
            {
                _imageHandler->saveImage("ReferenceIrradiance.hdr", envIrradianceMap);
            }
        }
        else
        {
            mx::Sh3ColorCoeffs shIrradiance = mx::projectEnvironment(envRadianceMap, true);
            envIrradianceMap = mx::renderEnvironment(shIrradiance, IRRADIANCE_MAP_WIDTH, IRRADIANCE_MAP_HEIGHT);
            if (_saveGeneratedLights)
            {
                _imageHandler->saveImage("SphericalHarmonicIrradiance.hdr", envIrradianceMap);
            }
        }
    }

    // Release any existing environment maps and store the new ones.
    _imageHandler->releaseRenderResources(_lightHandler->getEnvRadianceMap());
    _imageHandler->releaseRenderResources(_lightHandler->getEnvIrradianceMap());
    _lightHandler->setEnvRadianceMap(envRadianceMap);
    _lightHandler->setEnvIrradianceMap(envIrradianceMap);

    // Look for a light rig using an expected filename convention.
    if (!_splitDirectLight)
    {
        _lightRigFilename = mx::removeExtension(_envRadiancePath) + "." + mx::MTLX_EXTENSION;
        if (_lightRigFilename.exists())
        {
            _lightRigDoc = mx::createDocument();
            mx::readFromXmlFile(_lightRigDoc, _lightRigFilename, _searchPath);
        }
        else
        {
            _lightRigDoc = nullptr;
        }
    }

    const mx::MeshList& meshes = _envGeometryHandler->getMeshes();
    if (!meshes.empty())
    {
        // Set up world matrix for drawing
        const float scaleFactor = 300.0f;
        const float rotationRadians = PI / 2.0f; // 90 degree rotation 
        _envMatrix = mx::Matrix44::createScale(mx::Vector3(scaleFactor)) * mx::Matrix44::createRotationY(rotationRadians);

        // Create environment shader.
        mx::FilePath envFilename = _searchPath.find(
            mx::FilePath("resources/Materials/TestSuite/lights/envmap_shader.mtlx"));
        _envMaterial = Material::create();
        _envMaterial->generateEnvironmentShader(_genContext, envFilename, _stdLib, _envRadiancePath);
        _envMaterial->bindMesh(_envGeometryHandler->getMeshes()[0]);
    }
}

void Viewer::applyDirectLights(mx::DocumentPtr doc)
{
    if (_lightRigDoc)
    {
        mx::CopyOptions copyOptions;
        copyOptions.skipConflictingElements = true;
        doc->importLibrary(_lightRigDoc, &copyOptions);
        _xincludeFiles.insert(_lightRigFilename);
    }

    try 
    {
        std::vector<mx::NodePtr> lights;
        _lightHandler->findLights(doc, lights);
        _lightHandler->registerLights(doc, lights, _genContext);
        _lightHandler->setLightSources(lights);
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to set up lighting", e.what());
    }
}

void Viewer::assignMaterial(mx::MeshPartitionPtr geometry, MaterialPtr material)
{
    const mx::MeshList& meshes = _geometryHandler->getMeshes();
    if (meshes.empty() || !geometry)
    {
        return;
    }

    if (material)
    {
        material->bindMesh(meshes[0]);
        _materialAssignments[geometry] = material;
    }
    else
    {
        _materialAssignments.erase(geometry);
    }

    if (geometry == getSelectedGeometry())
    {
        setSelectedMaterial(material);
        updateDisplayedProperties();
    }
}

void Viewer::createLoadMeshInterface(Widget* parent, const std::string& label)
{
    ng::Button* meshButton = new ng::Button(parent, label);
    meshButton->setIcon(ENTYPO_ICON_FOLDER);
    meshButton->setCallback([this]()
    {
        mProcessEvents = false;
        std::string filename = ng::file_dialog({ { "obj", "Wavefront OBJ" } }, false);
        if (!filename.empty())
        {
            _geometryHandler->clearGeometry();
            if (_geometryHandler->loadGeometry(filename))
            {
                _meshFilename = filename;
                const mx::MeshList& meshes = _geometryHandler->getMeshes();
                if (!meshes.empty())
                {
                    if (_splitByUdims)
                    {
                        meshes[0]->splitByUdims();
                    }
                }

                updateGeometrySelections();

                // Assign the selected material to all geometries.
                _materialAssignments.clear();
                MaterialPtr material = getSelectedMaterial();
                if (material)
                {
                    for (mx::MeshPartitionPtr geom : _geometryList)
                    {
                        assignMaterial(geom, material);
                    }
                }

                initCamera();

                _imageHandler->releaseRenderResources(_shadowMap);
                _shadowMap = nullptr;
            }
            else
            {
                new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Mesh Loading Error", filename);
            }
        }
        mProcessEvents = true;
    });
}

void Viewer::createLoadMaterialsInterface(Widget* parent, const std::string& label)
{
    ng::Button* materialButton = new ng::Button(parent, label);
    materialButton->setIcon(ENTYPO_ICON_FOLDER);
    materialButton->setCallback([this]()
    {
        mProcessEvents = false;
        std::string filename = ng::file_dialog({ { "mtlx", "MaterialX" } }, false);
        if (!filename.empty())
        {
            _materialFilename = filename;
            assignMaterial(getSelectedGeometry(), nullptr);
            loadDocument(_materialFilename, _stdLib);
        }
        mProcessEvents = true;
    });
}

void Viewer::createLoadEnvironmentInterface(Widget* parent, const std::string& label)
{
    ng::Button* envButton = new ng::Button(parent, label);
    envButton->setIcon(ENTYPO_ICON_FOLDER);
    envButton->setCallback([this]()
    {
        mProcessEvents = false;
        mx::StringSet extensions;
        _imageHandler->supportedExtensions(extensions);
        std::vector<std::pair<std::string, std::string>> filetypes;
        for (const auto& extension : extensions)
        {
            filetypes.push_back(std::make_pair(extension, extension));
        }
        std::string filename = ng::file_dialog(filetypes, false);
        if (!filename.empty())
        {
            _envRadiancePath = filename;
            loadEnvironmentLight();
            loadDocument(_materialFilename, _stdLib);

            _imageHandler->releaseRenderResources(_shadowMap);
            _shadowMap = nullptr;
        }
        mProcessEvents = true;
    });
}

void Viewer::createSaveMaterialsInterface(Widget* parent, const std::string& label)
{
    ng::Button* materialButton = new ng::Button(parent, label);
    materialButton->setIcon(ENTYPO_ICON_SAVE);
    materialButton->setCallback([this]()
    {
        mProcessEvents = false;
        MaterialPtr material = getSelectedMaterial();
        mx::FilePath filename = ng::file_dialog({ { "mtlx", "MaterialX" } }, true);

        // Save document
        if (material && !filename.isEmpty())
        {
            mx::DocumentPtr doc = material->getDocument();
            if (filename.getExtension() != mx::MTLX_EXTENSION)
            {
                filename = mx::FilePath(filename.asString() + "." + mx::MTLX_EXTENSION);
            }

            if (_bakeTextures && material->getMaterialElement())
            {

                mx::FileSearchPath searchPath = _searchPath;
                if (material->getDocument())
                {
                    mx::FilePath documentFilename = material->getDocument()->getSourceUri();
                    searchPath.append(documentFilename.getParentPath());
                }

                mx::ImageHandlerPtr imageHandler = mx::GLTextureHandler::create(mx::StbImageLoader::create());
                imageHandler->setSearchPath(searchPath);
                if (!material->getUdim().empty())
                {
                    mx::StringResolverPtr resolver = mx::StringResolver::create();
                    resolver->setUdimString(material->getUdim());
                    imageHandler->setFilenameResolver(resolver);
                }

                mx::TextureBakerPtr baker = mx::TextureBaker::create();
                baker->setImageHandler(imageHandler);

                mx::ShaderRefPtr shaderRef = material->getElement()->asA<mx::ShaderRef>();
                if (shaderRef)
                {
                    baker->bakeShaderInputs(shaderRef, _genContext, filename.getParentPath());
                    baker->writeBakedDocument(shaderRef, filename);
                }
                mx::NodePtr shader = material->getElement()->asA<mx::Node>();
                if (shader)
                {
                    baker->bakeShaderInputs(shader, _genContext, filename.getParentPath());
                    baker->writeBakedDocument(shader, filename);
                }
            }
            else
            {
                // Add element predicate to prune out writing elements from included files
                auto skipXincludes = [this](mx::ConstElementPtr elem)
                {
                    if (elem->hasSourceUri())
                    {
                        return (_xincludeFiles.count(elem->getSourceUri()) == 0);
                    }
                    return true;
                };
                mx::XmlWriteOptions writeOptions;
                writeOptions.writeXIncludeEnable = true;
                writeOptions.elementPredicate = skipXincludes;
                mx::writeToXmlFile(doc, filename, &writeOptions);
            }

            // Update material file name
            _materialFilename = filename;
        }
        mProcessEvents = true;
    });
}

void Viewer::createPropertyEditorInterface(Widget* parent, const std::string& label)
{
    ng::Button* editorButton = new ng::Button(parent, label);
    editorButton->setFlags(ng::Button::ToggleButton);
    editorButton->setChangeCallback([this](bool state)
    {
        _propertyEditor.setVisible(state);
        performLayout();
    });
}

void Viewer::createAdvancedSettings(Widget* parent)
{
    ng::PopupButton* advancedButton = new ng::PopupButton(parent, "Advanced Settings");
    advancedButton->setIcon(ENTYPO_ICON_TOOLS);
    advancedButton->setChevronIcon(-1);
    ng::Popup* advancedPopup = advancedButton->popup();
    advancedPopup->setLayout(new ng::GroupLayout());

    new ng::Label(advancedPopup, "Mesh Options");

    ng::CheckBox* splitUdimsBox = new ng::CheckBox(advancedPopup, "Split By UDIMs");
    splitUdimsBox->setChecked(_splitByUdims);
    splitUdimsBox->setCallback([this](bool enable)
    {
        _splitByUdims = enable;
    });

    new ng::Label(advancedPopup, "Material Options");

    ng::CheckBox* mergeMaterialsBox = new ng::CheckBox(advancedPopup, "Merge Materials");
    mergeMaterialsBox->setChecked(_mergeMaterials);
    mergeMaterialsBox->setCallback([this](bool enable)
    {
        _mergeMaterials = enable;
    });    

    ng::CheckBox* bakeTexturesBox = new ng::CheckBox(advancedPopup, "Bake Textures");
    bakeTexturesBox->setChecked(_bakeTextures);
    bakeTexturesBox->setCallback([this](bool enable)
    {
        _bakeTextures = enable;
    });    

    Widget* unitGroup = new Widget(advancedPopup);
    unitGroup->setLayout(new ng::BoxLayout(ng::Orientation::Horizontal));
    new ng::Label(unitGroup, "Distance Unit:");
    _distanceUnitBox = new ng::ComboBox(unitGroup, _distanceUnitOptions);
    _distanceUnitBox->setFixedSize(ng::Vector2i(100, 20));
    _distanceUnitBox->setChevronIcon(-1);
    _distanceUnitBox->setSelectedIndex(_distanceUnitConverter->getUnitAsInteger("meter"));
    _distanceUnitBox->setCallback([this](int index)
    {
        mProcessEvents = false;
        _genContext.getOptions().targetDistanceUnit = _distanceUnitOptions[index];
        for (MaterialPtr material : _materials)
        {
            material->bindUnits(_unitRegistry, _genContext);
        }
        mProcessEvents = true;
    });

    new ng::Label(advancedPopup, "Lighting Options");

    ng::CheckBox* directLightingBox = new ng::CheckBox(advancedPopup, "Direct Lighting");
    directLightingBox->setChecked(_directLighting);
    directLightingBox->setCallback([this](bool enable)
    {
        _directLighting = enable;
    });

    ng::CheckBox* indirectLightingBox = new ng::CheckBox(advancedPopup, "Indirect Lighting");
    indirectLightingBox->setChecked(_indirectLighting);
    indirectLightingBox->setCallback([this](bool enable)
    {
        _indirectLighting = enable;
    });

    ng::CheckBox* normalizeEnvironmentBox = new ng::CheckBox(advancedPopup, "Normalize Environment");
    normalizeEnvironmentBox->setChecked(_normalizeEnvironment);
    normalizeEnvironmentBox->setCallback([this](bool enable)
    {
        _normalizeEnvironment = enable;
    });

    ng::CheckBox* splitDirectLightBox = new ng::CheckBox(advancedPopup, "Split Direct Light");
    splitDirectLightBox->setChecked(_splitDirectLight);
    splitDirectLightBox->setCallback([this](bool enable)
    {
        _splitDirectLight = enable;
    });

    new ng::Label(advancedPopup, "Shadowing Options");

    ng::CheckBox* shadowMapBox = new ng::CheckBox(advancedPopup, "Shadow Map");
    shadowMapBox->setChecked(_genContext.getOptions().hwShadowMap);
    shadowMapBox->setCallback([this](bool enable)
    {
        _genContext.getOptions().hwShadowMap = enable;
        reloadShaders();
    });

    ng::CheckBox* ambientOcclusionBox = new ng::CheckBox(advancedPopup, "Ambient Occlusion");
    ambientOcclusionBox->setChecked(_genContext.getOptions().hwAmbientOcclusion);
    ambientOcclusionBox->setCallback([this](bool enable)
    {
        _genContext.getOptions().hwAmbientOcclusion = enable;
        reloadShaders();
    });

    ng::Widget* ambientOcclusionGainRow = new ng::Widget(advancedPopup);
    ambientOcclusionGainRow->setLayout(new ng::BoxLayout(ng::Orientation::Horizontal));
    ng::FloatBox<float>* ambientOcclusionGainBox = createFloatWidget(ambientOcclusionGainRow, "AO Gain:",
        _ambientOcclusionGain, nullptr, [this](float value)
    {
        _ambientOcclusionGain = value;
    });
    ambientOcclusionGainBox->setEditable(true);
    ambientOcclusionGainBox->setMinMaxValues(0.0f, 1.0f);

    new ng::Label(advancedPopup, "Render Options");

    ng::CheckBox* outlineSelectedGeometryBox = new ng::CheckBox(advancedPopup, "Outline Selected Geometry");
    outlineSelectedGeometryBox->setChecked(_outlineSelection);
    outlineSelectedGeometryBox->setCallback([this](bool enable)
    {
        _outlineSelection = enable;
    });

    ng::CheckBox* drawEnvironmentBox = new ng::CheckBox(advancedPopup, "Render Environment");
    drawEnvironmentBox->setChecked(_drawEnvironment);
    drawEnvironmentBox->setCallback([this](bool enable)
    {
        _drawEnvironment = enable;
    });

    if (_specularEnvironmentMethod == mx::SPECULAR_ENVIRONMENT_FIS)
    {
        Widget* sampleGroup = new Widget(advancedPopup);
        sampleGroup->setLayout(new ng::BoxLayout(ng::Orientation::Horizontal));
        new ng::Label(sampleGroup, "Environment Samples:");
        mx::StringVec sampleOptions;
        for (int i = MIN_ENV_SAMPLES; i <= MAX_ENV_SAMPLES; i *= 4)
        {
            mProcessEvents = false;
            sampleOptions.push_back(std::to_string(i));
            mProcessEvents = true;
        }
        ng::ComboBox* sampleBox = new ng::ComboBox(sampleGroup, sampleOptions);
        sampleBox->setChevronIcon(-1);
        sampleBox->setSelectedIndex((int)std::log2(DEFAULT_ENV_SAMPLES / MIN_ENV_SAMPLES) / 2);
        sampleBox->setCallback([this](int index)
        {
            _envSamples = MIN_ENV_SAMPLES * (int) std::pow(4, index);
        });
    }

    new ng::Label(advancedPopup, "Property Editor Options");

    ng::CheckBox* showAdvancedProperties = new ng::CheckBox(advancedPopup, "Show Advanced Properties");
    showAdvancedProperties->setChecked(_showAdvancedProperties);
    showAdvancedProperties->setCallback([this](bool enable)
    {
        _showAdvancedProperties = enable;
        updateDisplayedProperties();
    });
}

void Viewer::updateGeometrySelections()
{
    _geometryList.clear();
    if (_geometryHandler->getMeshes().empty())
    {
        return;
    }
    mx::MeshPtr mesh = _geometryHandler->getMeshes()[0];

    if (_wireMaterial)
    {
        _wireMaterial->bindMesh(mesh);
    }
    if (_shadowMaterial)
    {
        _shadowMaterial->bindMesh(mesh);
    }

    for (size_t partIndex = 0; partIndex < mesh->getPartitionCount(); partIndex++)
    {
        mx::MeshPartitionPtr part = mesh->getPartition(partIndex);
        _geometryList.push_back(part);
    }

    std::vector<std::string> items;
    for (const mx::MeshPartitionPtr& part : _geometryList)
    {
        std::string geomName = part->getIdentifier();
        mx::StringVec geomSplit = mx::splitString(geomName, ":");
        if (!geomSplit.empty() && !geomSplit[geomSplit.size() - 1].empty())
        {
            geomName = geomSplit[geomSplit.size() - 1];
        }
        items.push_back(geomName);
    }
    _geometrySelectionBox->setItems(items);

    _geomLabel->setVisible(items.size() > 1);
    _geometrySelectionBox->setVisible(items.size() > 1);
    _selectedGeom = 0;

    performLayout();
}

void Viewer::updateMaterialSelections()
{
    std::vector<std::string> items;
    for (const auto& material : _materials)
    {
        mx::ElementPtr displayElem = material->getMaterialElement();
        if (!displayElem)
            displayElem = material->getElement();
        std::string displayName = displayElem->getNamePath();
        if (!material->getUdim().empty())
        {
            displayName += " (" + material->getUdim() + ")";
        }
        items.push_back(displayName);
    }
    _materialSelectionBox->setItems(items);

    _materialLabel->setVisible(items.size() > 1);
    _materialSelectionBox->setVisible(items.size() > 1);

    performLayout();
}

void Viewer::updateMaterialSelectionUI()
{
    for (const auto& pair : _materialAssignments)
    {
        if (pair.first == getSelectedGeometry())
        {
            for (size_t i = 0; i < _materials.size(); i++)
            {
                if (_materials[i] == pair.second)
                {
                    _materialSelectionBox->setSelectedIndex((int) i);
                    break;
                }
            }
        }
    }
}

void Viewer::loadDocument(const mx::FilePath& filename, mx::DocumentPtr libraries)
{
    // Set up read options.
    mx::XmlReadOptions readOptions;
    readOptions.desiredMajorVersion = 1;
    readOptions.desiredMinorVersion = 38;
    readOptions.skipConflictingElements = true;
    readOptions.readXIncludeFunction = [](mx::DocumentPtr doc, const mx::FilePath& filename,
                                          const mx::FileSearchPath& searchPath, const mx::XmlReadOptions* options)
    {
        mx::FilePath resolvedFilename = searchPath.find(filename);
        if (resolvedFilename.exists())
        {
            readFromXmlFile(doc, resolvedFilename, searchPath, options);
        }
        else
        {
            std::cerr << "Include file not found: " << filename.asString() << std::endl;
        }
    };

    // Clear user data on the generator.
    _genContext.clearUserData();

    // Clear materials if merging is not requested.
    if (!_mergeMaterials)
    {
        for (mx::MeshPartitionPtr geom : _geometryList)
        {
            if (_materialAssignments.count(geom))
            {
                assignMaterial(geom, nullptr);
            }
        }
        _materials.clear();
    }

    std::vector<MaterialPtr> newMaterials;
    try
    {
        // Load source document.
        mx::DocumentPtr doc = mx::createDocument();
        mx::readFromXmlFile(doc, filename, _searchPath, &readOptions);

        // Import libraries.
        mx::CopyOptions copyOptions; 
        copyOptions.skipConflictingElements = true;
        doc->importLibrary(libraries, &copyOptions);

        // Apply direct lights.
        applyDirectLights(doc);

        // Apply modifiers to the content document.
        applyModifiers(doc, _modifiers);

        // Validate the document.
        std::string message;
        if (!doc->validate(&message))
        {
            std::cerr << "*** Validation warnings for " << _materialFilename.getBaseName() << " ***" << std::endl;
            std::cerr << message;
        }

        // Find new renderable elements.
        mx::StringVec renderablePaths;
        std::vector<mx::TypedElementPtr> elems;
        std::vector<mx::TypedElementPtr> materials;
        mx::findRenderableElements(doc, elems);
        for (mx::TypedElementPtr elem : elems)
        {
            mx::TypedElementPtr renderableElem = elem;
            mx::NodePtr node = elem->asA<mx::Node>();
            if (node && node->getType() == mx::MATERIAL_TYPE_STRING)
            {
                std::vector<mx::NodePtr> shaderNodes = getShaderNodes(node, mx::SURFACE_SHADER_TYPE_STRING);
                if (!shaderNodes.empty())
                {
                    renderableElem = shaderNodes[0];
                }
                materials.push_back(node);
            }
            else
            {
                mx::ShaderRefPtr shaderRef = elem->asA<mx::ShaderRef>();
                mx::TypedElementPtr materialRef = (shaderRef ? shaderRef->getParent()->asA<mx::TypedElement>() : nullptr);
                materials.push_back(materialRef);
            }
            renderablePaths.push_back(renderableElem->getNamePath());
        }

        // Check for any udim set.
        mx::ValuePtr udimSetValue = doc->getGeomPropValue("udimset");

        // Create new materials.
        mx::TypedElementPtr udimElement;
        for (size_t i=0; i<renderablePaths.size(); i++)
        {
            const auto& renderablePath = renderablePaths[i];
            mx::ElementPtr elem = doc->getDescendant(renderablePath);
            mx::TypedElementPtr typedElem = elem ? elem->asA<mx::TypedElement>() : nullptr;
            if (!typedElem)
            {
                continue;
            }
            if (udimSetValue && udimSetValue->isA<mx::StringVec>())
            {
                for (const std::string& udim : udimSetValue->asA<mx::StringVec>())
                {
                    MaterialPtr mat = Material::create();
                    mat->setDocument(doc);
                    mat->setElement(typedElem);
                    mat->setMaterialElement(materials[i]);
                    mat->setUdim(udim);
                    newMaterials.push_back(mat);
                    
                    udimElement = typedElem;
                }
            }
            else
            {
                MaterialPtr mat = Material::create();
                mat->setDocument(doc);
                mat->setElement(typedElem);
                mat->setMaterialElement(materials[i]);
                newMaterials.push_back(mat);
            }
        }

        if (!newMaterials.empty())
        {
            // Add new materials to the global vector.
            _materials.insert(_materials.end(), newMaterials.begin(), newMaterials.end());

            // Set the default image search path.
            mx::FilePath materialFolder = _materialFilename.getParentPath();
            _imageHandler->setSearchPath(mx::FileSearchPath(materialFolder));

            mx::MeshPtr mesh = _geometryHandler->getMeshes()[0];
            MaterialPtr udimMaterial = nullptr;
            for (MaterialPtr mat : newMaterials)
            {
                // Clear cached implementations, in case libraries on the file system have changed.
                _genContext.clearNodeImplementations();

                mx::TypedElementPtr elem = mat->getElement();

                std::string udim = mat->getUdim();
                if (!udim.empty())
                {
                    if ((udimElement == elem) && udimMaterial)
                    {
                        // Reuse existing material for all udims
                        mat->copyShader(udimMaterial);
                    }
                    else
                    {
                        // Generate a shader for the new material.
                        mat->generateShader(_genContext);
                        if (udimElement == elem)
                        {
                            udimMaterial = mat;
                        }
                    }
                }
                else
                {
                    // Generate a shader for the new material.
                    mat->generateShader(_genContext);
                }

                // Apply geometric assignments specified in the document, if any.
                mx::ShaderRefPtr shaderRef = elem->asA<mx::ShaderRef>();
                mx::MaterialPtr materialRef = shaderRef ? shaderRef->getParent()->asA<mx::Material>() : nullptr;
                if (materialRef)
                {
                    for (mx::MeshPartitionPtr part : _geometryList)
                    {
                        std::string partGeomName = part->getIdentifier();
                        if (!materialRef->getGeometryBindings(partGeomName).empty())
                        {
                            assignMaterial(part, mat);
                        }
                    }
                }
                else if (mat && mat->getMaterialElement())
                {
                    mx::NodePtr materialNode = mat->getMaterialElement()->asA<mx::Node>();
                    if (materialNode)
                    {
                        for (mx::MeshPartitionPtr part : _geometryList)
                        {
                            std::string partGeomName = part->getIdentifier();
                            if (!getGeometryBindings(materialNode, partGeomName).empty())
                            {
                                assignMaterial(part, mat);
                            }
                        }
                    }
                }

                // Apply implicit udim assignments, if any.
                if (!udim.empty())
                {
                    for (mx::MeshPartitionPtr geom : _geometryList)
                    {
                        if (!_materialAssignments[geom] && geom->getIdentifier() == udim)
                        {
                            assignMaterial(geom, mat);
                        }
                    }
                }
            }

            // Apply fallback assignments.
            for (mx::MeshPartitionPtr geom : _geometryList)
            {
                if (!_materialAssignments[geom])
                {
                    assignMaterial(geom, newMaterials[0]);
                }
            }
        }
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to load material", e.what());
        return;
    }

    // Update material UI.
    updateMaterialSelections();
    updateMaterialSelectionUI();

    performLayout();
}

void Viewer::reloadShaders()
{
    try
    {
        const mx::MeshList& meshes = _geometryHandler->getMeshes();
        for (MaterialPtr material : _materials)
        {
            material->generateShader(_genContext);
            if (!meshes.empty())
            {
                material->bindMesh(meshes[0]);
            }
        }
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Shader Generation Error", e.what());
    }
}

void Viewer::saveShaderSource()
{
    try
    {
        MaterialPtr material = getSelectedMaterial();
        mx::TypedElementPtr elem = material ? material->getElement() : nullptr;
        if (elem)
        {
            mx::ShaderPtr shader = createShader(elem->getNamePath(), _genContext, elem);
            if (shader)
            {
                std::string vertexShader = shader->getSourceCode(mx::Stage::VERTEX);
                std::string pixelShader = shader->getSourceCode(mx::Stage::PIXEL);
                std::string baseName = _searchPath[0] / elem->getName();
                writeTextFile(vertexShader,  baseName + "_vs.glsl");
                writeTextFile(pixelShader, baseName + "_ps.glsl");
                new ng::MessageDialog(this, ng::MessageDialog::Type::Information, "Saved GLSL source: ", baseName);
            }
        }
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Cannot save source for material", e.what());
    }
}

void Viewer::loadShaderSource()
{
    try
    {
        MaterialPtr material = getSelectedMaterial();
        mx::TypedElementPtr elem = material ? material->getElement() : nullptr;
        if (elem)
        {
            std::string elementName = elem->getName();
            std::string baseName = _searchPath[0] / elementName;
            std::string vertexShaderFile = baseName + "_vs.glsl";
            std::string pixelShaderFile = baseName + "_ps.glsl";
            bool hasTransparency = false;
            if (material->loadSource(vertexShaderFile, pixelShaderFile, baseName, hasTransparency))
            {
                assignMaterial(getSelectedGeometry(), material);
            }
        }
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Cannot load source for material", e.what());
    }
}

void Viewer::saveDotFiles()
{
    try
    {
        MaterialPtr material = getSelectedMaterial();
        mx::TypedElementPtr elem = material ? material->getElement() : nullptr;
        if (elem)
        {
            mx::ShaderRefPtr shaderRef = elem->asA<mx::ShaderRef>();
            if (shaderRef)
            {
                for (mx::BindInputPtr bindInput : shaderRef->getBindInputs())
                {
                    mx::OutputPtr output = bindInput->getConnectedOutput();
                    mx::ConstNodeGraphPtr nodeGraph = output ? output->getAncestorOfType<mx::NodeGraph>() : nullptr;
                    if (nodeGraph)
                    {
                        std::string dot = nodeGraph->asStringDot();
                        std::string baseName = _searchPath[0] / nodeGraph->getName();
                        writeTextFile(dot, baseName + ".dot");
                    }
                }

                mx::NodeDefPtr nodeDef = shaderRef->getNodeDef();
                mx::InterfaceElementPtr implement = nodeDef ? nodeDef->getImplementation() : nullptr;
                mx::NodeGraphPtr nodeGraph = implement ? implement->asA<mx::NodeGraph>() : nullptr;
                if (nodeGraph)
                {
                    std::string dot = nodeGraph->asStringDot();
                    std::string baseName = _searchPath[0] / nodeDef->getName();
                    writeTextFile(dot, baseName + ".dot");
                }
            }
            else
            {
                mx::NodePtr shaderNode = elem->asA<mx::Node>();
                if (shaderNode && material->getMaterialElement())
                {
                    for (mx::InputPtr input : shaderNode->getInputs())
                    {
                        mx::OutputPtr output = input->getConnectedOutput();
                        mx::ConstNodeGraphPtr nodeGraph = output ? output->getAncestorOfType<mx::NodeGraph>() : nullptr;
                        if (nodeGraph)
                        {
                            std::string dot = nodeGraph->asStringDot();
                            std::string baseName = _searchPath[0] / nodeGraph->getName();
                            writeTextFile(dot, baseName + ".dot");
                        }
                    }

                    mx::NodeDefPtr nodeDef = shaderNode->getNodeDef();
                    mx::InterfaceElementPtr implement = nodeDef ? nodeDef->getImplementation() : nullptr;
                    mx::NodeGraphPtr nodeGraph = implement ? implement->asA<mx::NodeGraph>() : nullptr;
                    if (nodeGraph)
                    {
                        std::string dot = nodeGraph->asStringDot();
                        std::string baseName = _searchPath[0] / nodeDef->getName();
                        writeTextFile(dot, baseName + ".dot");
                    }
                }
            }
        }
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Cannot save dot file for material", e.what());
    }
}

void Viewer::loadStandardLibraries()
{
    // Initialize the standard library.
    _stdLib = loadLibraries(_libraryFolders, _searchPath);
    for (std::string sourceUri : _stdLib->getReferencedSourceUris())
    {
        _xincludeFiles.insert(sourceUri);
    }

    // Initialize color management.
    mx::DefaultColorManagementSystemPtr cms = mx::DefaultColorManagementSystem::create(_genContext.getShaderGenerator().getLanguage());
    cms->loadLibrary(_stdLib);
    _genContext.registerSourceCodeSearchPath(_searchPath);
    _genContext.getShaderGenerator().setColorManagementSystem(cms);

    // Initialize unit management.
    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(_genContext.getShaderGenerator().getLanguage());
    unitSystem->loadLibrary(_stdLib);
    unitSystem->setUnitConverterRegistry(_unitRegistry);
    _genContext.getShaderGenerator().setUnitSystem(unitSystem);
    mx::UnitTypeDefPtr distanceTypeDef = _stdLib->getUnitTypeDef("distance");
    _distanceUnitConverter = mx::LinearUnitConverter::create(distanceTypeDef);
    _unitRegistry->addUnitConverter(distanceTypeDef, _distanceUnitConverter);
    mx::UnitTypeDefPtr angleTypeDef = _stdLib->getUnitTypeDef("angle");
    mx::LinearUnitConverterPtr angleConverter = mx::LinearUnitConverter::create(angleTypeDef);
    _unitRegistry->addUnitConverter(angleTypeDef, angleConverter);
    _genContext.getOptions().targetDistanceUnit = "meter";

    // Create the list of supported distance units.
    auto unitScales = _distanceUnitConverter->getUnitScale();
    _distanceUnitOptions.resize(unitScales.size());
    for (auto unitScale : unitScales)
    {
        int location = _distanceUnitConverter->getUnitAsInteger(unitScale.first);
        _distanceUnitOptions[location] = unitScale.first;
    }
}

bool Viewer::keyboardEvent(int key, int scancode, int action, int modifiers)
{
    if (Screen::keyboardEvent(key, scancode, action, modifiers))
    {
        return true;
    }

    // Reload the current document, and optionally the standard libraries, from
    // the file system.
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        MaterialPtr material = getSelectedMaterial();
        mx::DocumentPtr doc = material ? material->getDocument() : nullptr;
        mx::FilePath filename = doc ? mx::FilePath(doc->getSourceUri()) : _materialFilename;
        if (modifiers == GLFW_MOD_SHIFT)
        {
            loadStandardLibraries();
        }
        loadDocument(filename, _stdLib);
        return true;
    }

    // Save the current shader source to file.
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        saveShaderSource();
        return true;
    }

    // Load shader source from file.  Editing the source files before loading
    // provides a way to debug and experiment with shader source code.
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        loadShaderSource();
        return true;
    }

    // Save each node graph in the current material as a dot file.
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        saveDotFiles();
        return true;
    }

    // Capture the current frame and save to file.
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        mx::StringSet extensions;
        _imageHandler->supportedExtensions(extensions);
        if (!extensions.empty())
        {
            std::vector<std::pair<std::string, std::string>> filetypes;
            for (const auto& extension : extensions)
            {
                filetypes.push_back(std::make_pair(extension, extension));
            }
            std::string fileName = ng::file_dialog(filetypes, true);
            if (!fileName.empty())
            {
                std::string fileExtension = mx::FilePath(fileName).getExtension();
                if (extensions.count(fileExtension) == 0)
                {
                    fileName += "." + *extensions.begin();
                }
                _captureFrameFilename = fileName;
                _captureFrame = true;
            }
        }
    }

    // Allow left and right keys to cycle through the renderable elements
    if ((key == GLFW_KEY_RIGHT || key == GLFW_KEY_LEFT) && action == GLFW_PRESS)
    {
        size_t materialCount = _materials.size();
        size_t materialIndex = _selectedMaterial;
        if (materialCount > 1)
        {
            if (key == GLFW_KEY_RIGHT)
            {
                _selectedMaterial = (materialIndex < materialCount - 1) ? materialIndex + 1 : 0;
            }
            else
            {
                _selectedMaterial = (materialIndex > 0) ? materialIndex - 1 : materialCount - 1;
            }
            if (!_geometryList.empty())
            {
                assignMaterial(getSelectedGeometry(), getSelectedMaterial());
                updateMaterialSelectionUI();
            }
        }
        return true;
    }

    return false;
}

void Viewer::drawContents()
{
    if (_geometryList.empty() || _materials.empty())
    {
        return;
    }

    updateViewHandlers();

    checkGlErrors("before viewer render");

    // Initialize OpenGL state
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_FRAMEBUFFER_SRGB);

    // Update shadow state
    ShadowState shadowState;
    shadowState.ambientOcclusionGain = _ambientOcclusionGain;
    mx::NodePtr dirLight = _lightHandler->getFirstLightOfCategory(DIR_LIGHT_NODE_CATEGORY);
    if (_genContext.getOptions().hwShadowMap && dirLight)
    {
        updateShadowMap();
        shadowState.shadowMap = _shadowMap;
        shadowState.shadowMatrix = _cameraViewHandler->worldMatrix.getInverse() *
                                   _shadowViewHandler->worldMatrix *
                                   _shadowViewHandler->viewMatrix *
                                   _shadowViewHandler->projectionMatrix;
    }

    const mx::Matrix44& world = _cameraViewHandler->worldMatrix;
    const mx::Matrix44& view = _cameraViewHandler->viewMatrix;
    const mx::Matrix44& proj = _cameraViewHandler->projectionMatrix;

    glEnable(GL_FRAMEBUFFER_SRGB);

    // Environment background
    if (_drawEnvironment && _envMaterial)
    {
        auto meshes = _envGeometryHandler->getMeshes();
        auto envPart = !meshes.empty() ? meshes[0]->getPartition(0) : nullptr;
        if (_envMaterial && envPart)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            _envMaterial->bindShader();
            _envMaterial->bindViewInformation(_envMatrix, view, proj);
            _envMaterial->bindImages(_imageHandler, _searchPath);
            _envMaterial->drawPartition(envPart);
            glDisable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
    }

    // Opaque pass
    for (const auto& assignment : _materialAssignments)
    {
        mx::MeshPartitionPtr geom = assignment.first;
        MaterialPtr material = assignment.second;
        shadowState.ambientOcclusionMap = getAmbientOcclusionImage(material);
        if (!material || material->hasTransparency())
        {
            continue;
        }

        material->bindShader();
        material->bindViewInformation(world, view, proj);
        material->bindLights(_lightHandler, _imageHandler,
                             _directLighting, _indirectLighting, shadowState,
                             _specularEnvironmentMethod, _envSamples);
        material->bindImages(_imageHandler, _searchPath);
        material->drawPartition(geom);
        material->unbindImages(_imageHandler);
    }

    // Transparent pass
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto& assignment : _materialAssignments)
    {
        mx::MeshPartitionPtr geom = assignment.first;
        MaterialPtr material = assignment.second;
        shadowState.ambientOcclusionMap = getAmbientOcclusionImage(material);
        if (!material || !material->hasTransparency())
        {
            continue;
        }

        material->bindShader();
        material->bindViewInformation(world, view, proj);
        material->bindLights(_lightHandler, _imageHandler,
                             _directLighting, _indirectLighting, ShadowState(),
                             _specularEnvironmentMethod, _envSamples);
        material->bindImages(_imageHandler, _searchPath);
        material->drawPartition(geom);
        material->unbindImages(_imageHandler);
    }
    
    glDisable(GL_BLEND);
    glDisable(GL_FRAMEBUFFER_SRGB);

    // Wireframe pass
    if (_outlineSelection)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        _wireMaterial->bindShader();
        _wireMaterial->bindViewInformation(world, view, proj);
        _wireMaterial->drawPartition(getSelectedGeometry());
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Frame capture
    if (_captureFrame)
    {
        _captureFrame = false;

        glFlush();
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        // Create an image with dimensions adjusted for device DPI.
        mx::ImagePtr image = mx::Image::create((unsigned int) (mSize.x() * mPixelRatio),
                                               (unsigned int) (mSize.y() * mPixelRatio), 3);
        image->createResourceBuffer();

        // Read pixels into the image buffer.
        glReadPixels(0, 0, image->getWidth(), image->getHeight(), GL_RGB, GL_UNSIGNED_BYTE, image->getResourceBuffer());

        // Save the image to disk.
        bool saved = _imageHandler->saveImage(_captureFrameFilename, image, true);
        if (!saved)
        {
            new ng::MessageDialog(this, ng::MessageDialog::Type::Information,
                "Failed to save frame to disk: ", _captureFrameFilename.asString());
        }
    }

    // Texture baking
    if (_bakeRequested)
    {
        _bakeRequested = false;

        MaterialPtr material = getSelectedMaterial();
        mx::ShaderRefPtr shaderRef = material->getElement()->asA<mx::ShaderRef>();
        mx::FileSearchPath searchPath = _searchPath;
        if (material->getDocument())
        {
            mx::FilePath documentFilename = material->getDocument()->getSourceUri();
            searchPath.append(documentFilename.getParentPath());
        }

        mx::ImageHandlerPtr imageHandler = mx::GLTextureHandler::create(mx::StbImageLoader::create());
        imageHandler->setSearchPath(searchPath);
        if (!material->getUdim().empty())
        {
            mx::StringResolverPtr resolver = mx::StringResolver::create();
            resolver->setUdimString(material->getUdim());
            imageHandler->setFilenameResolver(resolver);
        }

        try
        {
            mx::TextureBakerPtr baker = mx::TextureBaker::create();
            baker->setImageHandler(imageHandler);
            baker->bakeShaderInputs(shaderRef, _genContext, _bakeFilename.getParentPath());
            baker->writeBakedDocument(shaderRef, _bakeFilename);
        }
        catch (mx::Exception& e)
        {
            new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to bake textures", e.what());
        }

        glfwMakeContextCurrent(mGLFWWindow);
        glfwGetFramebufferSize(mGLFWWindow, &mFBSize[0], &mFBSize[1]);
        glViewport(0, 0, mFBSize[0], mFBSize[1]);
    }

    checkGlErrors("after viewer render");
}

bool Viewer::scrollEvent(const ng::Vector2i& p, const ng::Vector2f& rel)
{
    if (!Screen::scrollEvent(p, rel))
    {
        _userZoom = std::max(0.1f, _userZoom * ((rel.y() > 0) ? 1.1f : 0.9f));;
    }
    return true;
}

bool Viewer::mouseMotionEvent(const ng::Vector2i& p,
                              const ng::Vector2i& rel,
                              int button,
                              int modifiers)
{
    if (Screen::mouseMotionEvent(p, rel, button, modifiers))
    {
        return true;
    }

    if (_arcball.motion(p))
    {
        return true;
    }

    if (_userTranslationActive)
    {
        updateViewHandlers();
        const mx::Matrix44& world = _cameraViewHandler->worldMatrix;
        const mx::Matrix44& view = _cameraViewHandler->viewMatrix;
        const mx::Matrix44& proj = _cameraViewHandler->projectionMatrix;
        mx::Matrix44 worldView = world * view;;

        mx::MeshPtr mesh = _geometryHandler->getMeshes()[0];
        mx::Vector3 boxMin = mesh->getMinimumBounds();
        mx::Vector3 boxMax = mesh->getMaximumBounds();
        mx::Vector3 sphereCenter = (boxMax + boxMin) / 2.0f;

        float zval = ng::project(ng::Vector3f(sphereCenter.data()),
                                 ng::Matrix4f(worldView.data()),
                                 ng::Matrix4f(proj.data()),
                                 mSize).z();
        ng::Vector3f pos1 = ng::unproject(ng::Vector3f((float) p.x(),
                                                       (float) (mSize.y() - p.y()),
                                                       (float) zval),
                                          ng::Matrix4f(worldView.data()),
                                          ng::Matrix4f(proj.data()),
                                          mSize);
        ng::Vector3f pos0 = ng::unproject(ng::Vector3f((float) _userTranslationPixel.x(),
                                                       (float) (mSize.y() - _userTranslationPixel.y()),
                                                       (float) zval),
                                          ng::Matrix4f(worldView.data()),
                                          ng::Matrix4f(proj.data()),
                                          mSize);
        ng::Vector3f delta = pos1 - pos0;
        _userTranslation = _userTranslationStart +
                            mx::Vector3(delta.data(), delta.data() + delta.size());

        return true;
    }

    return false;
}

bool Viewer::mouseButtonEvent(const ng::Vector2i& p, int button, bool down, int modifiers)
{
    if (Screen::mouseButtonEvent(p, button, down, modifiers))
    {
        return true;
    }

    if (button == GLFW_MOUSE_BUTTON_1 && !modifiers)
    {
        _arcball.button(p, down);
    }
    else if (button == GLFW_MOUSE_BUTTON_2 ||
            (button == GLFW_MOUSE_BUTTON_1 && modifiers == GLFW_MOD_SHIFT))
    {
        _userTranslationStart = _userTranslation;
        _userTranslationActive = true;
        _userTranslationPixel = p;
    }
    if (button == GLFW_MOUSE_BUTTON_1 && !down)
    {
        _arcball.button(p, false);
    }
    if (!down)
    {
        _userTranslationActive = false;
    }
    return true;
}

void Viewer::initCamera()
{
    _arcball = ng::Arcball();
    _arcball.setSize(mSize);

    if (_geometryHandler->getMeshes().empty())
    {
        return;
    }
    mx::MeshPtr mesh = _geometryHandler->getMeshes()[0];

    mx::Vector3 boxMin = mesh->getMinimumBounds();
    mx::Vector3 boxMax = mesh->getMaximumBounds();
    mx::Vector3 sphereCenter = (boxMax + boxMin) / 2.0f;
    float sphereRadius = (sphereCenter - boxMin).getMagnitude();
    _modelZoom = MODEL_SPHERE_RADIUS / sphereRadius;
    _modelTranslation = sphereCenter * -1.0f;
}

void Viewer::updateViewHandlers()
{
    float fH = std::tan(_viewAngle / 360.0f * PI) * _nearDist;
    float fW = fH * (float) mSize.x() / (float) mSize.y();

    ng::Matrix4f ngArcball = _arcball.matrix();
    mx::Matrix44 arcball = mx::Matrix44(ngArcball.data(), ngArcball.data() + ngArcball.size());

    _cameraViewHandler->worldMatrix = mx::Matrix44::createTranslation(_modelTranslation + _userTranslation);
    _cameraViewHandler->worldMatrix *= mx::Matrix44::createScale(mx::Vector3(_modelZoom * _userZoom));
    _cameraViewHandler->viewMatrix = arcball * mx::ViewHandler::createViewMatrix(_eye, _center, _up);
    _cameraViewHandler->projectionMatrix = mx::ViewHandler::createPerspectiveMatrix(-fW, fW, -fH, fH, _nearDist, _farDist);

    mx::NodePtr dirLight = _lightHandler->getFirstLightOfCategory(DIR_LIGHT_NODE_CATEGORY);
    if (dirLight)
    {
        const float r = MODEL_SPHERE_RADIUS;
        _shadowViewHandler->worldMatrix = mx::Matrix44::createTranslation(_modelTranslation);
        _shadowViewHandler->worldMatrix *= mx::Matrix44::createScale(mx::Vector3(_modelZoom));
        _shadowViewHandler->projectionMatrix = mx::ViewHandler::createOrthographicMatrix(-r, r, -r, r, 0.0f, r * 2.0f);
        mx::ValuePtr dir = dirLight->getInputValue("direction");
        if (dir->isA<mx::Vector3>())
        {
            _shadowViewHandler->viewMatrix = mx::ViewHandler::createViewMatrix(
                dir->asA<mx::Vector3>() * -r, mx::Vector3(0.0f), _up);
        }
    }
}

void Viewer::updateDisplayedProperties()
{
    _propertyEditor.updateContents(this);
    createSaveMaterialsInterface(_propertyEditor.getWindow(), "Save Material");
}

mx::ImagePtr Viewer::getAmbientOcclusionImage(MaterialPtr material)
{
    const mx::string AO_FILENAME_SUFFIX = "_ao";
    const mx::string AO_FILENAME_EXTENSION = "png";
    const mx::Color4 AO_FALLBACK_COLOR(1.0f);

    if (!material || !_genContext.getOptions().hwAmbientOcclusion)
    {
        return nullptr;
    }

    std::string aoSuffix = material->getUdim().empty() ? AO_FILENAME_SUFFIX : AO_FILENAME_SUFFIX + "_" + material->getUdim();
    mx::FilePath aoFilename = mx::removeExtension(_meshFilename) + aoSuffix + "." + AO_FILENAME_EXTENSION;

    _imageHandler->setSearchPath(_searchPath);
    return _imageHandler->acquireImage(aoFilename, true, &AO_FALLBACK_COLOR);
}

void Viewer::splitDirectLight(mx::ImagePtr envRadianceMap, mx::ImagePtr& indirectMap, mx::DocumentPtr& dirLightDoc)
{
    mx::Vector3 lightDir;
    mx::Color3 lightColor;
    mx::ImagePair imagePair = envRadianceMap->splitByLuminance(ENV_MAP_SPLIT_RADIANCE);

    mx::computeDominantLight(imagePair.second, lightDir, lightColor);
    float lightIntensity = std::max(std::max(lightColor[0], lightColor[1]), lightColor[2]);
    lightColor /= lightIntensity;

    dirLightDoc = mx::createDocument();
    mx::NodePtr dirLightNode = dirLightDoc->addNode(DIR_LIGHT_NODE_CATEGORY, "dir_light", mx::LIGHT_SHADER_TYPE_STRING);
    dirLightNode->setInputValue("direction", lightDir);
    dirLightNode->setInputValue("color", lightColor);
    dirLightNode->setInputValue("intensity", lightIntensity);
    indirectMap = imagePair.first;
}

void Viewer::updateShadowMap()
{
    if (_shadowMap || !_shadowMaterial)
    {
        return;
    }

    if (!_shadowFramebuffer)
    {
        _shadowFramebuffer = mx::GLFramebuffer::create(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 2, mx::Image::BaseType::FLOAT);
    }

    const mx::Matrix44& world = _shadowViewHandler->worldMatrix;
    const mx::Matrix44& view = _shadowViewHandler->viewMatrix;
    const mx::Matrix44& proj = _shadowViewHandler->projectionMatrix;

    mx::ImageSamplingProperties blurSamplingProperties;
    blurSamplingProperties.uaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
    blurSamplingProperties.vaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
    blurSamplingProperties.filterType = mx::ImageSamplingProperties::FilterType::CLOSEST;

    // Clear shadow framebuffer.
    _shadowFramebuffer->bind();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Render shadow geometry.
    _shadowMaterial->bindShader();
    _shadowMaterial->bindViewInformation(world, view, proj);
    mx::MeshPtr mesh = _geometryHandler->getMeshes()[0];
    for (size_t i = 0; i < mesh->getPartitionCount(); i++)
    {
        mx::MeshPartitionPtr geom = mesh->getPartition(i);
        _shadowMaterial->drawPartition(geom);
    }
    _shadowMap = _shadowFramebuffer->createColorImage();

    // Apply Gaussian blurring.
    for (unsigned int i = 0; i < _shadowSoftness; i++)
    {
        _shadowFramebuffer->bind();
        _shadowBlurMaterial->bindShader();
        if (_imageHandler->bindImage(_shadowMap, blurSamplingProperties))
        {
            mx::GLTextureHandlerPtr textureHandler = std::static_pointer_cast<mx::GLTextureHandler>(_imageHandler);
            int textureLocation = textureHandler->getBoundTextureLocation(_shadowMap->getResourceId());
            if (textureLocation >= 0)
            {
                _shadowBlurMaterial->getShader()->setUniform("image_file", textureLocation);
            }
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        mx::GlslRenderer::drawScreenSpaceQuad();
        _imageHandler->releaseRenderResources(_shadowMap);
        _shadowMap = _shadowFramebuffer->createColorImage();
    }

    // Restore state for scene rendering.
    glViewport(0, 0, mFBSize[0], mFBSize[1]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
}

void Viewer::checkGlErrors(const std::string& context)
{
    for (GLenum error = glGetError(); error; error = glGetError())
    {
        std::cerr << "OpenGL error " << context << ": " << std::to_string(error) << std::endl;
    }
}
