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

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <MaterialXCore/MaterialNode.h>
#include <MaterialXFormat/Environ.h>
#include <MaterialXFormat/Util.h>

#include <nanogui/button.h>
#include <nanogui/combobox.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/messagedialog.h>
#include <nanogui/vscrollpanel.h>

#include <fstream>
#include <iostream>

const mx::Vector3 DEFAULT_CAMERA_POSITION(0.0f, 0.0f, 5.0f);
const float DEFAULT_CAMERA_VIEW_ANGLE = 45.0f;
const float DEFAULT_CAMERA_ZOOM = 1.0f;
const int DEFAULT_ENV_SAMPLES = 16;

namespace {

const int MIN_ENV_SAMPLES = 4;
const int MAX_ENV_SAMPLES = 1024;

const int SHADOW_MAP_SIZE = 2048;
const int ALBEDO_TABLE_SIZE = 64;
const int IRRADIANCE_MAP_WIDTH = 256;
const int IRRADIANCE_MAP_HEIGHT = 128;

const int MIN_TEXTURE_RES = 256;
const int MAX_TEXTURE_RES = 8192;
const int DEFAULT_TEXTURE_RES = 1024;

const std::string DIR_LIGHT_NODE_CATEGORY = "directional_light";
const std::string IRRADIANCE_MAP_FOLDER = "irradiance";

const float ENV_MAP_SPLIT_RADIANCE = 16.0f;
const float MAX_ENV_TEXEL_RADIANCE = 100000.0f;
const float IDEAL_ENV_MAP_RADIANCE = 6.0f;

const float IDEAL_MESH_SPHERE_RADIUS = 2.0f;

const float PI = std::acos(-1.0f);

void writeTextFile(const std::string& text, const std::string& filePath)
{
    std::ofstream file;
    file.open(filePath);
    file << text;
    file.close();
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
            if (!mx::stringEndsWith(filePrefix, modifiers.filePrefixTerminator))
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
               const mx::Vector3& meshRotation,
               float meshScale,
               const mx::Vector3& cameraPosition,
               const mx::Vector3& cameraTarget,
               float cameraViewAngle,
               float cameraZoom,    
               const std::string& envRadiancePath,
               mx::HwSpecularEnvironmentMethod specularEnvironmentMethod,
               int envSampleCount,
               float lightRotation,
               const mx::FilePathVec& libraryFolders,
               const mx::FileSearchPath& searchPath,
               const DocumentModifiers& modifiers,
               int screenWidth,
               int screenHeight,
               const mx::Color3& screenColor,
               int multiSampleCount) :
    ng::Screen(ng::Vector2i(screenWidth, screenHeight), "MaterialXView",
        true, false,
        8, 8, 24, 8,
        multiSampleCount),
    _meshRotation(meshRotation),
    _meshScale(meshScale),
    _cameraPosition(cameraPosition),
    _cameraTarget(cameraTarget),
    _cameraUp(0.0f, 1.0f, 0.0f),
    _cameraViewAngle(cameraViewAngle),
    _cameraNearDist(0.05f),
    _cameraFarDist(5000.0f),
    _userCameraEnabled(true),
    _userTranslationActive(false),
    _userTranslationPixel(0, 0),
    _userScale(cameraZoom),
    _libraryFolders(libraryFolders),
    _searchPath(searchPath),
    _materialFilename(materialFilename),
    _modifiers(modifiers),
    _envRadiancePath(envRadiancePath),
    _lightRotation(lightRotation),
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
#if MATERIALX_BUILD_GEN_OSL
    _genContextOsl(mx::OslShaderGenerator::create()),
#endif
#if MATERIALX_BUILD_GEN_MDL
    _genContextMdl(mx::MdlShaderGenerator::create()),
#endif
    _unitRegistry(mx::UnitConverterRegistry::create()),
    _splitByUdims(true),
    _mergeMaterials(false),
    _renderTransparency(true),
    _renderDoubleSided(true),
    _outlineSelection(false),
    _envSamples(envSampleCount),
    _drawEnvironment(false),
    _captureRequested(false),
    _wedgeRequested(false),
    _wedgePropertyName("specular_roughness"),
    _wedgePropertyMin(0.0f),
    _wedgePropertyMax(1.0f),
    _wedgeImageCount(8),
    _bakeTextures(false),
    _bakeHdr(false),
    _bakeAverage(false),
    _bakeOptimize(true),
    _bakeTextureRes(DEFAULT_TEXTURE_RES),
    _bakeRequested(false)
{
    _window = new ng::Window(this, "Viewer Options");
    _window->setPosition(ng::Vector2i(15, 15));
    _window->setLayout(new ng::GroupLayout());

    // Set the requested background color.
    setBackground(ng::Color(screenColor[0], screenColor[1], screenColor[2], 1.0f));

    // Initialize the standard libraries and color/unit management.
    loadStandardLibraries();

    // Set default generator options.
    _genContext.getOptions().hwSpecularEnvironmentMethod = specularEnvironmentMethod;
    _genContext.getOptions().hwDirectionalAlbedoMethod = mx::DIRECTIONAL_ALBEDO_TABLE;
    _genContext.getOptions().hwShadowMap = true;
    _genContext.getOptions().targetColorSpaceOverride = "lin_rec709";
    _genContext.getOptions().fileTextureVerticalFlip = true;

    // Set OSL/MDL generator options.
#if MATERIALX_BUILD_GEN_OSL
    _genContextOsl.getOptions().targetColorSpaceOverride = "lin_rec709";
    _genContextOsl.getOptions().fileTextureVerticalFlip = false;
#endif
#if MATERIALX_BUILD_GEN_MDL
    _genContextMdl.getOptions().targetColorSpaceOverride = "lin_rec709";
    _genContextMdl.getOptions().fileTextureVerticalFlip = false;
#endif

    // Initialize image handler.
    _imageHandler = mx::GLTextureHandler::create(mx::StbImageLoader::create());
#if MATERIALX_BUILD_OIIO
    _imageHandler->addLoader(mx::OiioImageLoader::create());
#endif
    _imageHandler->setSearchPath(_searchPath);

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
        mx::ShaderPtr hwShader = mx::createConstantShader(_genContext, _stdLib, "__WIRE_SHADER__", mx::Color3(1.0f));
        _wireMaterial = Material::create();
        _wireMaterial->generateShader(hwShader);
    }
    catch (std::exception& e)
    {
        std::cerr << "Failed to generate wireframe shader: " << e.what() << std::endl;
        _wireMaterial = nullptr;
    }

    // Generate shadow material.
    try
    {
        mx::ShaderPtr hwShader = mx::createDepthShader(_genContext, _stdLib, "__SHADOW_SHADER__");
        _shadowMaterial = Material::create();
        _shadowMaterial->generateShader(hwShader);
    }
    catch (std::exception& e)
    {
        std::cerr << "Failed to generate shadow shader: " << e.what() << std::endl;
        _shadowMaterial = nullptr;
    }

    // Generate shadow blur material.
    try
    {
        mx::ShaderPtr hwShader = mx::createBlurShader(_genContext, _stdLib, "__SHADOW_BLUR_SHADER__", "gaussian", 1.0f);
        _shadowBlurMaterial = Material::create();
        _shadowBlurMaterial->generateShader(hwShader);
    }
    catch (std::exception& e)
    {
        std::cerr << "Failed to generate shadow blur shader: " << e.what() << std::endl;
        _shadowBlurMaterial = nullptr;
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
    // Load the requested radiance map.
    mx::ImagePtr envRadianceMap = _imageHandler->acquireImage(_envRadiancePath);
    if (!envRadianceMap)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to load environment light");
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
    if (!_normalizeEnvironment && !_splitDirectLight)
    {
        mx::FilePath envIrradiancePath = _envRadiancePath.getParentPath() / IRRADIANCE_MAP_FOLDER / _envRadiancePath.getBaseName();
        envIrradianceMap = _imageHandler->acquireImage(envIrradiancePath);
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
        _lightRigFilename = _envRadiancePath;
        _lightRigFilename.removeExtension();
        _lightRigFilename.addExtension(mx::MTLX_EXTENSION);
        if (_searchPath.find(_lightRigFilename).exists())
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
        // Create environment shader.
        mx::FilePath envFilename = _searchPath.find(
            mx::FilePath("resources/Materials/TestSuite/lights/envmap_shader.mtlx"));
        try
        {
            _envMaterial = Material::create();
            _envMaterial->generateEnvironmentShader(_genContext, envFilename, _stdLib, _envRadiancePath);
            _envMaterial->bindMesh(_envGeometryHandler->getMeshes()[0]);
        }
        catch (std::exception& e)
        {
            std::cerr << "Failed to generate environment shader: " << e.what() << std::endl;
            _envMaterial = nullptr;
        }
    }
}

void Viewer::applyDirectLights(mx::DocumentPtr doc)
{
    if (_lightRigDoc)
    {
        doc->importLibrary(_lightRigDoc);
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

                _meshRotation = mx::Vector3();
                _meshScale = 1.0f;
                _cameraPosition = DEFAULT_CAMERA_POSITION;
                _cameraTarget = mx::Vector3();
                _cameraViewAngle = DEFAULT_CAMERA_VIEW_ANGLE;
                initCamera();

                invalidateShadowMap();
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
        mx::StringSet extensions = _imageHandler->supportedExtensions();
        std::vector<std::pair<std::string, std::string>> filetypes;
        for (const auto& extension : extensions)
        {
            filetypes.emplace_back(extension, extension);
        }
        std::string filename = ng::file_dialog(filetypes, false);
        if (!filename.empty())
        {
            _envRadiancePath = filename;
            loadEnvironmentLight();
            loadDocument(_materialFilename, _stdLib);
            invalidateShadowMap();
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
            if (filename.getExtension() != mx::MTLX_EXTENSION)
            {
                filename.addExtension(mx::MTLX_EXTENSION);
            }

            if (_bakeTextures)
            {
                _bakeRequested = true;
                _bakeFilename = filename;
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
                mx::writeToXmlFile(material->getDocument(), filename, &writeOptions);
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
    ng::Popup* advancedPopupParent = advancedButton->popup();
    advancedPopupParent->setLayout(new ng::GroupLayout());

    ng::VScrollPanel* scrollPanel = new ng::VScrollPanel(advancedPopupParent);
    scrollPanel->setFixedHeight(500);
    ng::Widget* advancedPopup = new ng::Widget(scrollPanel);
    advancedPopup->setLayout(new ng::GroupLayout(13));

    ng::Label* meshLabel = new ng::Label(advancedPopup, "Mesh Options");
    meshLabel->setFontSize(20);
    meshLabel->setFont("sans-bold");

    ng::CheckBox* splitUdimsBox = new ng::CheckBox(advancedPopup, "Split By UDIMs");
    splitUdimsBox->setChecked(_splitByUdims);
    splitUdimsBox->setCallback([this](bool enable)
    {
        _splitByUdims = enable;
    });

    ng::Label* materialLabel = new ng::Label(advancedPopup, "Material Options");
    materialLabel->setFontSize(20);
    materialLabel->setFont("sans-bold");

    ng::CheckBox* mergeMaterialsBox = new ng::CheckBox(advancedPopup, "Merge Materials");
    mergeMaterialsBox->setChecked(_mergeMaterials);
    mergeMaterialsBox->setCallback([this](bool enable)
    {
        _mergeMaterials = enable;
    });    

    Widget* unitGroup = new Widget(advancedPopup);
    unitGroup->setLayout(new ng::BoxLayout(ng::Orientation::Horizontal));
    new ng::Label(unitGroup, "Distance Unit:");
    _distanceUnitBox = new ng::ComboBox(unitGroup, _distanceUnitOptions);
    _distanceUnitBox->setFixedSize(ng::Vector2i(100, 20));
    _distanceUnitBox->setChevronIcon(-1);
    if (_distanceUnitConverter)
    {
        _distanceUnitBox->setSelectedIndex(_distanceUnitConverter->getUnitAsInteger("meter"));
    }
    _distanceUnitBox->setCallback([this](int index)
    {
        mProcessEvents = false;
        _genContext.getOptions().targetDistanceUnit = _distanceUnitOptions[index];
#if MATERIALX_BUILD_GEN_OSL
        _genContextOsl.getOptions().targetDistanceUnit = _distanceUnitOptions[index];
#endif
#if MATERIALX_BUILD_GEN_MDL
        _genContextMdl.getOptions().targetDistanceUnit = _distanceUnitOptions[index];
#endif
        for (MaterialPtr material : _materials)
        {
            material->bindUnits(_unitRegistry, _genContext);
        }
        mProcessEvents = true;
    });

    ng::Label* lightingLabel = new ng::Label(advancedPopup, "Lighting Options");
    lightingLabel->setFontSize(20);
    lightingLabel->setFont("sans-bold");

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

    ng::Widget* lightRotationRow = new ng::Widget(advancedPopup);
    lightRotationRow->setLayout(new ng::BoxLayout(ng::Orientation::Horizontal));
    mx::UIProperties ui;
    ui.uiMin = mx::Value::createValue(0.0f);
    ui.uiMax = mx::Value::createValue(360.0f);
    ng::FloatBox<float>* lightRotationBox = createFloatWidget(lightRotationRow, "Light Rotation:",
        _lightRotation, &ui, [this](float value)
    {
        _lightRotation = value;
        invalidateShadowMap();
    });
    lightRotationBox->setEditable(true);

    ng::Label* shadowingLabel = new ng::Label(advancedPopup, "Shadowing Options");
    shadowingLabel->setFontSize(20);
    shadowingLabel->setFont("sans-bold");

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

    ng::Label* renderLabel = new ng::Label(advancedPopup, "Render Options");
    renderLabel->setFontSize(20);
    renderLabel->setFont("sans-bold");

    ng::CheckBox* transparencyBox = new ng::CheckBox(advancedPopup, "Render Transparency");
    transparencyBox->setChecked(_renderTransparency);
    transparencyBox->setCallback([this](bool enable)
    {
        _renderTransparency = enable;
    });

    ng::CheckBox* doubleSidedBox = new ng::CheckBox(advancedPopup, "Render Double-Sided");
    doubleSidedBox->setChecked(_renderDoubleSided);
    doubleSidedBox->setCallback([this](bool enable)
    {
        _renderDoubleSided = enable;
    });

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

    ng::CheckBox* referenceQualityBox = new ng::CheckBox(advancedPopup, "Reference Quality");
    referenceQualityBox->setChecked(false);
    referenceQualityBox->setCallback([this](bool enable)
    {
        _genContext.getOptions().hwDirectionalAlbedoMethod = enable ? mx::DIRECTIONAL_ALBEDO_IS : mx::DIRECTIONAL_ALBEDO_TABLE;
        reloadShaders();
    });

    if (_genContext.getOptions().hwSpecularEnvironmentMethod == mx::SPECULAR_ENVIRONMENT_FIS)
    {
        Widget* sampleGroup = new Widget(advancedPopup);
        sampleGroup->setLayout(new ng::BoxLayout(ng::Orientation::Horizontal));
        new ng::Label(sampleGroup, "Environment Samples:");
        mx::StringVec sampleOptions;
        _genContext.getOptions().hwMaxRadianceSamples = MAX_ENV_SAMPLES;
        for (int i = MIN_ENV_SAMPLES; i <= MAX_ENV_SAMPLES; i *= 4)
        {
            mProcessEvents = false;
            sampleOptions.push_back(std::to_string(i));
            mProcessEvents = true;
        }
        ng::ComboBox* sampleBox = new ng::ComboBox(sampleGroup, sampleOptions);
        sampleBox->setChevronIcon(-1);
        sampleBox->setSelectedIndex((int)std::log2(_envSamples / MIN_ENV_SAMPLES) / 2);
        sampleBox->setCallback([this](int index)
        {
            _envSamples = MIN_ENV_SAMPLES * (int) std::pow(4, index);
        });
    }

    ng::Label* textureLabel = new ng::Label(advancedPopup, "Texture Baking Options");
    textureLabel->setFontSize(20);
    textureLabel->setFont("sans-bold");

    ng::CheckBox* bakeTexturesBox = new ng::CheckBox(advancedPopup, "Bake Textures");
    bakeTexturesBox->setChecked(_bakeTextures);
    bakeTexturesBox->setCallback([this](bool enable)
    {
        _bakeTextures = enable;
    });

    ng::CheckBox* bakeHdrBox = new ng::CheckBox(advancedPopup, "Bake HDR Textures");
    bakeHdrBox->setChecked(_bakeTextures);
    bakeHdrBox->setCallback([this](bool enable)
    {
        _bakeHdr = enable;
    });

    Widget* textureResGroup = new Widget(advancedPopup);
    textureResGroup->setLayout(new ng::BoxLayout(ng::Orientation::Horizontal));
    new ng::Label(textureResGroup, "Texture Res:");
    mx::StringVec textureResOptions;
    for (int i = MIN_TEXTURE_RES; i <= MAX_TEXTURE_RES; i *= 2)
    {
        mProcessEvents = false;
        textureResOptions.push_back(std::to_string(i));
        mProcessEvents = true;
    }
    ng::ComboBox* textureResBox = new ng::ComboBox(textureResGroup, textureResOptions);
    textureResBox->setChevronIcon(-1);
    textureResBox->setSelectedIndex((int)std::log2(DEFAULT_TEXTURE_RES / MIN_TEXTURE_RES));
    textureResBox->setCallback([this](int index)
    {
        _bakeTextureRes = MIN_TEXTURE_RES * (int) std::pow(2, index);
    });

    ng::Label* wedgeLabel = new ng::Label(advancedPopup, "Wedge Rendering Options");
    wedgeLabel->setFontSize(20);
    wedgeLabel->setFont("sans-bold");

    ng::Widget* wedgeMin = new ng::Widget(advancedPopup);
    wedgeMin->setLayout(new ng::BoxLayout(ng::Orientation::Horizontal));
    mx::UIProperties wedgeProp;
    wedgeProp.uiSoftMin = mx::Value::createValue(0.0f);
    wedgeProp.uiSoftMax = mx::Value::createValue(1.0f);
    ng::FloatBox<float>* wedgeMinBox = createFloatWidget(wedgeMin, "Wedge Minimum Value:",
        _wedgePropertyMax, &wedgeProp, [this](float value)
    {
        _wedgePropertyMin = value;
    });
    wedgeMinBox->setValue(0.0);
    wedgeMinBox->setEditable(true);

    ng::Widget* wedgeMax = new ng::Widget(advancedPopup);
    wedgeMax->setLayout(new ng::BoxLayout(ng::Orientation::Horizontal));
    ng::FloatBox<float>* wedgeMaxBox = createFloatWidget(wedgeMax, "Wedge Maximum Value:",
        _wedgePropertyMax, &wedgeProp, [this](float value)
    {
        _wedgePropertyMax = value;
    });
    wedgeMaxBox->setValue(1.0);
    wedgeMaxBox->setEditable(true);

    ng::Widget* wedgeCount = new ng::Widget(advancedPopup);
    wedgeCount->setLayout(new ng::BoxLayout(ng::Orientation::Horizontal));
    mx::UIProperties wedgeCountProp;
    wedgeCountProp.uiMin = mx::Value::createValue(1);
    wedgeCountProp.uiSoftMax = mx::Value::createValue(8);
    wedgeCountProp.uiStep = mx::Value::createValue(1);
    ng::IntBox<int>* wedgeCountBox = createIntWidget(wedgeCount, "Wedge Count:",
        _wedgeImageCount, &wedgeCountProp, [this](int value)
    {
        _wedgeImageCount = value;
    });
    wedgeCountBox->setValue(8);
    wedgeCountBox->setEditable(true);
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
        mx::ElementPtr displayElem = material->getMaterialElement() ?
                                     material->getMaterialElement() :
                                     material->getElement();
        std::string displayName = displayElem->getName();
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
    performLayout();
}

void Viewer::loadDocument(const mx::FilePath& filename, mx::DocumentPtr libraries)
{
    // Set up read options.
    mx::XmlReadOptions readOptions;
    readOptions.applyFutureUpdates = true;
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
        doc->importLibrary(libraries);

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
        if (elems.empty())
        {
            throw mx::Exception("No renderable elements found in " + _materialFilename.getBaseName());
        }
        for (mx::TypedElementPtr elem : elems)
        {
            mx::TypedElementPtr renderableElem = elem;
            mx::NodePtr node = elem->asA<mx::Node>();
            if (node && node->getType() == mx::MATERIAL_TYPE_STRING)
            {
                std::unordered_set<mx::NodePtr> shaderNodes = getShaderNodes(node, mx::SURFACE_SHADER_TYPE_STRING);
                if (!shaderNodes.empty())
                {
                    renderableElem = *shaderNodes.begin();
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
            // Extend the image search path to include this material folder.
            mx::FilePath materialFolder = _materialFilename.getParentPath();
            mx::FileSearchPath materialSearchPath = _searchPath;
            materialSearchPath.append(materialFolder);
            _imageHandler->setSearchPath(materialSearchPath);

            // Add new materials to the global vector.
            _materials.insert(_materials.end(), newMaterials.begin(), newMaterials.end());

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
                        try
                        {
                            mat->generateShader(_genContext);
                        }
                        catch (std::exception& e)
                        {
                            new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to generate shader", e.what());
                            continue;
                        }
                        if (udimElement == elem)
                        {
                            udimMaterial = mat;
                        }
                    }
                }
                else
                {
                    // Generate a shader for the new material.
                    try
                    {
                        mat->generateShader(_genContext);
                    }
                    catch (std::exception& e)
                    {
                        mat->copyShader(_wireMaterial);
                        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to generate shader", e.what());
                    }
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
                        if (geom->getIdentifier() == udim)
                        {
                            assignMaterial(geom, mat);
                        }
                    }
                }
            }

            // Apply fallback assignments.
            MaterialPtr fallbackMaterial = newMaterials[0];
            if (fallbackMaterial->getUdim().empty())
            {
                for (mx::MeshPartitionPtr geom : _geometryList)
                {
                    if (!_materialAssignments[geom])
                    {
                        assignMaterial(geom, fallbackMaterial);
                    }
                }
            }
        }
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to load material", e.what());
        return;
    }

    // Restore the original image search path.
    _imageHandler->setSearchPath(_searchPath);

    // Update material UI.
    updateMaterialSelections();
    updateMaterialSelectionUI();

    invalidateShadowMap();
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

void Viewer::saveShaderSource(mx::GenContext& context)
{
    try
    {
        MaterialPtr material = getSelectedMaterial();
        mx::TypedElementPtr elem = material ? material->getElement() : nullptr;
        if (elem)
        {
            mx::ShaderPtr shader = createShader(elem->getNamePath(), context, elem);
            if (shader)
            {
                const std::string path = mx::getEnviron("MATERIALX_VIEW_OUTPUT_PATH");
                const std::string baseName = (path.empty() ? _searchPath[0] : mx::FilePath(path)) / elem->getName();
                if (context.getShaderGenerator().getLanguage() == mx::GlslShaderGenerator::LANGUAGE && context.getShaderGenerator().getTarget() == mx::GlslShaderGenerator::TARGET)
                {
                    const std::string& vertexShader = shader->getSourceCode(mx::Stage::VERTEX);
                    const std::string& pixelShader = shader->getSourceCode(mx::Stage::PIXEL);
                    writeTextFile(vertexShader, baseName + "_vs.glsl");
                    writeTextFile(pixelShader, baseName + "_ps.glsl");
                    new ng::MessageDialog(this, ng::MessageDialog::Type::Information, "Saved GLSL source: ", baseName);
                }
#if MATERIALX_BUILD_GEN_OSL
                else if (context.getShaderGenerator().getLanguage() == mx::OslShaderGenerator::LANGUAGE)
                {
                    const std::string& pixelShader = shader->getSourceCode(mx::Stage::PIXEL);
                    writeTextFile(pixelShader, baseName + ".osl");
                    new ng::MessageDialog(this, ng::MessageDialog::Type::Information, "Saved OSL source: ", baseName);
                }
#endif
#if MATERIALX_BUILD_GEN_MDL
                else if (context.getShaderGenerator().getLanguage() == mx::MdlShaderGenerator::LANGUAGE)
                {
                    const std::string& pixelShader = shader->getSourceCode(mx::Stage::PIXEL);
                    writeTextFile(pixelShader, baseName + ".mdl");
                    new ng::MessageDialog(this, ng::MessageDialog::Type::Information, "Saved MDL source: ", baseName);
                }
#endif
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
            const std::string path = mx::getEnviron("MATERIALX_VIEW_OUTPUT_PATH");
            const std::string baseName = (path.empty() ? _searchPath[0] : mx::FilePath(path)) / elem->getName();
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

void Viewer::initContext(mx::GenContext& context)
{
    // Initialize search paths.
    context.registerSourceCodeSearchPath(_searchPath);

    // Initialize color management.
    mx::DefaultColorManagementSystemPtr cms = mx::DefaultColorManagementSystem::create(context.getShaderGenerator().getLanguage());
    cms->loadLibrary(_stdLib);
    context.getShaderGenerator().setColorManagementSystem(cms);

    // Initialize unit management.
    mx::UnitSystemPtr unitSystem = mx::UnitSystem::create(context.getShaderGenerator().getLanguage());
    unitSystem->loadLibrary(_stdLib);
    unitSystem->setUnitConverterRegistry(_unitRegistry);
    context.getShaderGenerator().setUnitSystem(unitSystem);
    context.getOptions().targetDistanceUnit = "meter";
}

void Viewer::loadStandardLibraries()
{
    // Initialize the standard library.
    try
    {
        mx::XmlReadOptions readOptions;
        readOptions.applyFutureUpdates = true;
        _stdLib = mx::createDocument();
        _xincludeFiles = mx::loadLibraries(_libraryFolders, _searchPath, _stdLib, mx::StringSet(), &readOptions);
        if (_xincludeFiles.empty())
        {
            std::cerr << "Could not find standard data libraries on the given search path: " << _searchPath.asString() << std::endl;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Failed to load standard data libraries: " << e.what() << std::endl;
        return;
    }

    // Initialize unit management.
    mx::UnitTypeDefPtr distanceTypeDef = _stdLib->getUnitTypeDef("distance");
    _distanceUnitConverter = mx::LinearUnitConverter::create(distanceTypeDef);
    _unitRegistry->addUnitConverter(distanceTypeDef, _distanceUnitConverter);
    mx::UnitTypeDefPtr angleTypeDef = _stdLib->getUnitTypeDef("angle");
    mx::LinearUnitConverterPtr angleConverter = mx::LinearUnitConverter::create(angleTypeDef);
    _unitRegistry->addUnitConverter(angleTypeDef, angleConverter);

    // Create the list of supported distance units.
    auto unitScales = _distanceUnitConverter->getUnitScale();
    _distanceUnitOptions.resize(unitScales.size());
    for (auto unitScale : unitScales)
    {
        int location = _distanceUnitConverter->getUnitAsInteger(unitScale.first);
        _distanceUnitOptions[location] = unitScale.first;
    }

    // Initialize the generator contexts.
    initContext(_genContext);
#if MATERIALX_BUILD_GEN_OSL
    initContext(_genContextOsl);
#endif
#if MATERIALX_BUILD_GEN_MDL
    initContext(_genContextMdl);
#endif
}

bool Viewer::keyboardEvent(int key, int scancode, int action, int modifiers)
{
    if (Screen::keyboardEvent(key, scancode, action, modifiers))
    {
        return true;
    }

    // Adjust camera zoom.
    if (_userCameraEnabled)
    {
        if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS)
        {
            _userScale *= 1.1f;
        }
        if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS)
        {
            _userScale = std::max(0.1f, _userScale * 0.9f);
        }
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
        saveShaderSource(_genContext);
        return true;
    }

#if MATERIALX_BUILD_GEN_OSL
    // Save OSL shader source to file.
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        saveShaderSource(_genContextOsl);
        return true;
    }
#endif

#if MATERIALX_BUILD_GEN_MDL
    // Save MDL shader source to file.
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        saveShaderSource(_genContextMdl);
        return true;
    }
#endif

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

    // Capture the current frame or render wedge and save as an image file.
    if ((key == GLFW_KEY_F || key == GLFW_KEY_W) && action == GLFW_PRESS)
    {
        mx::StringSet extensions = _imageHandler->supportedExtensions();
        std::vector<std::pair<std::string, std::string>> filetypes;
        for (const auto& extension : extensions)
        {
            filetypes.push_back(std::make_pair(extension, extension));
        }
        if (key == GLFW_KEY_F)
        {
            _captureFilename = ng::file_dialog(filetypes, true);
            if (!_captureFilename.isEmpty())
            {
                if (_captureFilename.getExtension().empty())
                {
                    _captureFilename.addExtension(mx::ImageLoader::TGA_EXTENSION);
                }
                _captureRequested = true;
            }
        }
        else
        {
            _wedgeFilename = ng::file_dialog(filetypes, true);
            if (!_wedgeFilename.isEmpty())
            {
                // There are issues with using the STB loader and png files
                // so use another format if PNG specified or if no extension specified
                const std::string extension = _wedgeFilename.getExtension();
                if (extension.empty())
                {                    
                    _wedgeFilename.addExtension(mx::ImageLoader::TGA_EXTENSION);
                }
                else if (extension == mx::ImageLoader::PNG_EXTENSION)
                {
                    _wedgeFilename.removeExtension();
                    _wedgeFilename.addExtension(mx::ImageLoader::TGA_EXTENSION);
                }
                _wedgeRequested = true;
            }
        }
    }

    // Up and down keys cycle through selected geometries.
    if ((key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) && action == GLFW_PRESS)
    {
        if (_geometryList.size() > 1)
        {
            if (key == GLFW_KEY_DOWN)
            {
                _selectedGeom = (_selectedGeom < _geometryList.size() - 1) ? _selectedGeom + 1 : 0;
            }
            else
            {
                _selectedGeom = (_selectedGeom > 0) ? _selectedGeom - 1 : _geometryList.size() - 1;
            }
        }
        _geometrySelectionBox->setSelectedIndex((int) _selectedGeom);
        updateMaterialSelectionUI();
        return true;
    }

    // Left and right keys cycle through selected materials.
    if ((key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) && action == GLFW_PRESS)
    {
        if (_materials.size() > 1)
        {
            if (key == GLFW_KEY_RIGHT)
            {
                _selectedMaterial = (_selectedMaterial < _materials.size() - 1) ? _selectedMaterial + 1 : 0;
            }
            else
            {
                _selectedMaterial = (_selectedMaterial > 0) ? _selectedMaterial - 1 : _materials.size() - 1;
            }
            assignMaterial(getSelectedGeometry(), getSelectedMaterial());
            updateMaterialSelectionUI();
       }
        return true;
    }

    return false;
}

void Viewer::renderFrame()
{
    // Initialize OpenGL state
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_FRAMEBUFFER_SRGB);

    // Update shading tables
    updateAlbedoTable();

    // Update lighting state.
    LightingState lightingState;
    lightingState.lightTransform = mx::Matrix44::createRotationY(_lightRotation / 180.0f * PI);
    lightingState.directLighting = _directLighting;
    lightingState.indirectLighting = _indirectLighting;
    lightingState.envSamples = _envSamples;

    // Update shadow state.
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
        mx::Matrix44 envWorld = mx::Matrix44::createScale(mx::Vector3(300.0f));
        float longitudeOffset = (_lightRotation / 360.0f) + 0.5f;
        _envMaterial->setUniformFloat("longitude/in2", longitudeOffset);

        if (envPart)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            _envMaterial->bindShader();
            _envMaterial->bindViewInformation(envWorld, view, proj);
            _envMaterial->bindImages(_imageHandler, _searchPath, false);
            _envMaterial->drawPartition(envPart);
            glDisable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
    }

    // Enable backface culling if requested.
    if (!_renderDoubleSided)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    // Opaque pass
    for (const auto& assignment : _materialAssignments)
    {
        mx::MeshPartitionPtr geom = assignment.first;
        MaterialPtr material = assignment.second;
        shadowState.ambientOcclusionMap = getAmbientOcclusionImage(material);
        if (!material)
        {
            continue;
        }

        if (material->getShader()->name() == "__WIRE_SHADER__")
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        material->bindShader();
        if (material->getShader()->uniform(mx::HW::ALPHA_THRESHOLD, false) != -1)
        {
            material->getShader()->setUniform(mx::HW::ALPHA_THRESHOLD, 0.99f);
        }
        material->bindViewInformation(world, view, proj);
        material->bindLights(_genContext, _lightHandler, _imageHandler, lightingState, shadowState);
        material->bindImages(_imageHandler, _searchPath);
        material->drawPartition(geom);
        material->unbindImages(_imageHandler);
        if (material->getShader()->name() == "__WIRE_SHADER__")
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    // Transparent pass
    if (_renderTransparency)
    {
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
            if (material->getShader()->uniform(mx::HW::ALPHA_THRESHOLD, false) != -1)
            {
                material->getShader()->setUniform(mx::HW::ALPHA_THRESHOLD, 0.001f);
            }
            material->bindViewInformation(world, view, proj);
            material->bindLights(_genContext, _lightHandler, _imageHandler, lightingState, shadowState);
            material->bindImages(_imageHandler, _searchPath);
            material->drawPartition(geom);
            material->unbindImages(_imageHandler);
        }
        glDisable(GL_BLEND);
    }

    if (!_renderDoubleSided)
    {
        glDisable(GL_CULL_FACE);
    }
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
}

mx::ImagePtr Viewer::getFrameImage()
{
    glFlush();
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    // Create an image with dimensions adjusted for device DPI.
    mx::ImagePtr image = mx::Image::create((unsigned int) (mSize.x() * mPixelRatio),
                                           (unsigned int) (mSize.y() * mPixelRatio), 3);
    image->createResourceBuffer();

    // Read pixels into the image buffer.
    glReadPixels(0, 0, image->getWidth(), image->getHeight(), GL_RGB, GL_UNSIGNED_BYTE, image->getResourceBuffer());

    return image;
}

mx::ImagePtr Viewer::renderWedge()
{
    MaterialPtr material = getSelectedMaterial();
    mx::ShaderPort* uniform = material ? material->findUniform(_wedgePropertyName) : nullptr;
    if (!uniform)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Material property not found", _wedgePropertyName);
        return nullptr;
    }

    mx::ValuePtr origPropertyValue = uniform->getValue();
    if (origPropertyValue)
    {
        if (!origPropertyValue->isA<int>() && !origPropertyValue->isA<float>() &&
            !origPropertyValue->isA<mx::Vector2>() &&
            !origPropertyValue->isA<mx::Color3>() && !origPropertyValue->isA<mx::Vector3>() &&
            !origPropertyValue->isA<mx::Color4>() && !origPropertyValue->isA<mx::Vector4>())
        {
            new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Material property type not supported", _wedgePropertyName);
            return nullptr;
        }

        std::vector<mx::ImagePtr> imageVec;
        float wedgePropertyStep = (_wedgePropertyMax - _wedgePropertyMin) / (_wedgeImageCount - 1);
        for (unsigned int i = 0; i < _wedgeImageCount; i++)
        {
            bool setValue = false;
            if (material)
            {
                float propertyValue = (i == _wedgeImageCount - 1) ? _wedgePropertyMax : _wedgePropertyMin + wedgePropertyStep * i;
                if (origPropertyValue->isA<int>())
                {
                    material->setUniformInt(_wedgePropertyName, static_cast<int>(propertyValue));
                    setValue = true;
                }
                else if (origPropertyValue->isA<float>())
                {
                    material->setUniformFloat(_wedgePropertyName, propertyValue);
                    setValue = true;
                }
                else if (origPropertyValue->isA<mx::Vector2>())
                {
                    ng::Vector2f val(propertyValue, propertyValue);
                    material->setUniformVec2(_wedgePropertyName, val);
                    setValue = true;
                }
                else if (origPropertyValue->isA<mx::Color3>() ||
                    origPropertyValue->isA<mx::Vector3>())
                {
                    ng::Vector3f val(propertyValue, propertyValue, propertyValue);
                    material->setUniformVec3(_wedgePropertyName, val);
                    setValue = true;
                }
                else if (origPropertyValue->isA<mx::Color4>() ||
                    origPropertyValue->isA<mx::Vector4>())
                {
                    ng::Vector4f val(propertyValue, propertyValue, propertyValue, origPropertyValue->isA<mx::Color4>() ? 1.0f : propertyValue);
                    material->setUniformVec4(_wedgePropertyName, val);
                    setValue = true;
                }
            }
            if (setValue)
            {
                renderFrame();
                imageVec.push_back(getFrameImage());
            }
        }

        if (material)
        {
            if (origPropertyValue->isA<int>())
            {
                material->setUniformInt(_wedgePropertyName, origPropertyValue->asA<int>());
            }
            else if (origPropertyValue->isA<float>())
            {
                material->setUniformFloat(_wedgePropertyName, origPropertyValue->asA<float>());
            }
            else if (origPropertyValue->isA<mx::Color3>())
            {
                const mx::Color3& val =  origPropertyValue->asA<mx::Color3>();
                ng::Vector3f ngval(val[0], val[1], val[2]);
                material->setUniformVec3(_wedgePropertyName, ngval);
            }
            else if (origPropertyValue->isA<mx::Vector2>())
            {
                const mx::Vector2& val = origPropertyValue->asA<mx::Vector2>();
                ng::Vector2f ngval(val[0], val[1]);
                material->setUniformVec2(_wedgePropertyName, ngval);
            }
            else if (origPropertyValue->isA<mx::Vector3>())
            {
                const mx::Vector3& val = origPropertyValue->asA<mx::Vector3>();
                ng::Vector3f ngval(val[0], val[1], val[2]);
                material->setUniformVec3(_wedgePropertyName, ngval);
            }
            else if (origPropertyValue->isA<mx::Color4>())
            {
                const mx::Color4& val = origPropertyValue->asA<mx::Color4>();
                ng::Vector4f ngval(val[0], val[1], val[1], val[2]);
                material->setUniformVec4(_wedgePropertyName, ngval);
            }
            else if (origPropertyValue->isA<mx::Vector4>())
            {
                const mx::Vector4& val = origPropertyValue->asA<mx::Vector4>();
                ng::Vector4f ngval(val[0], val[1], val[1], val[2]);
                material->setUniformVec4(_wedgePropertyName, ngval);
            }
        }

        return mx::createImageStrip(imageVec);
    }
    return nullptr;
}

void Viewer::bakeTextures()
{
    MaterialPtr material = getSelectedMaterial();
    if (!material->getMaterialElement())
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Information,
            "There are no material nodes to bake.");
        return;
    }

    mx::DocumentPtr origDoc = material->getDocument();
    if (!origDoc)
    {
        return;
    }
    // Make a copy before baking as baking process can change the document
    // layout.
    mx::DocumentPtr bakeDoc = mx::createDocument();
    bakeDoc->copyContentFrom(origDoc);
    
    // Create a unique image handler for baking.
    mx::FilePath documentFilename = origDoc->getSourceUri();
    mx::FileSearchPath searchPath = _searchPath;
    searchPath.append(documentFilename.getParentPath());
    mx::ImageHandlerPtr imageHandler = mx::GLTextureHandler::create(mx::StbImageLoader::create());
#if MATERIALX_BUILD_OIIO
    imageHandler->addLoader(mx::OiioImageLoader::create());
#endif
    imageHandler->setSearchPath(searchPath);

    // Compute material and UDIM lists.
    std::vector<MaterialPtr> materialsToBake;
    std::vector<std::string> udimSet;
    mx::ValuePtr udimSetValue = origDoc->getGeomPropValue("udimset");
    if (_materials.size() > 1 &&
        !material->getUdim().empty() &&
        udimSetValue &&
        !_bakeAverage)
    {
        materialsToBake = _materials;
        udimSet = udimSetValue->asA<std::vector<std::string>>();
    }
    else
    {
        materialsToBake.push_back(material);
    }

    {
        // Construct a texture baker.
        mx::Image::BaseType baseType = _bakeHdr ? mx::Image::BaseType::FLOAT : mx::Image::BaseType::UINT8;
        mx::TextureBakerPtr baker = mx::TextureBaker::create(_bakeTextureRes, _bakeTextureRes, baseType);
        baker->setupUnitSystem(_stdLib);
        baker->setTargetUnitSpace(_genContext.getOptions().targetDistanceUnit);
        baker->setAverageImages(_bakeAverage);
        baker->setOptimizeConstants(_bakeOptimize);

        mx::StringResolverPtr resolver = mx::StringResolver::create();

        // Bake each material in the list.
        for (MaterialPtr mat : materialsToBake)
        {
            mx::NodePtr origMaterialNode = mat->getMaterialElement()->asA<mx::Node>();
            mx::ElementPtr materialElement = bakeDoc->getDescendant(origMaterialNode->getNamePath());
            mx::NodePtr materialNode = materialElement ? materialElement->asA<mx::Node>() : nullptr;
            if (materialNode)
            {
                std::unordered_set<mx::NodePtr> shaderNodes = mx::getShaderNodes(materialNode, mx::SURFACE_SHADER_TYPE_STRING);
                if (!shaderNodes.empty())
                {
                    resolver->setUdimString(mat->getUdim());
                    imageHandler->setFilenameResolver(resolver);
                    try
                    {
                        // TODO: Only bake first surface shader 
                        mx::NodePtr shader = *shaderNodes.begin();
                        baker->setImageHandler(imageHandler);
                        baker->setOutputResourcePath(_bakeFilename.getParentPath());
                        baker->setBakedGraphName("NG_" + shader->getName());
                        baker->setBakedGeomInfoName("GI_" + shader->getName());
                        baker->bakeShaderInputs(materialNode, shader, _genContext, mat->getUdim());
                        // Optimize baked textures.
                        baker->optimizeBakedTextures(shader);
                        // Write the baked document and textures.
                        mx::DocumentPtr bakedDocument = baker->getBakedMaterial(shader, udimSet);
                        if (bakedDocument)
                        {
                            mx::writeToXmlFile(bakedDocument, _bakeFilename);
                            std::cout << "Write baked document: " << _bakeFilename.asString() << std::endl;
                        }
                    }
                    catch (mx::Exception& e)
                    {
                        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to bake textures", e.what());
                    }
                }
            }
        }
    }

    // Restore state for scene rendering.
    glfwMakeContextCurrent(mGLFWWindow);
    glfwGetFramebufferSize(mGLFWWindow, &mFBSize[0], &mFBSize[1]);
    glViewport(0, 0, mFBSize[0], mFBSize[1]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
}

void Viewer::drawContents()
{
    if (_geometryList.empty() || _materials.empty())
    {
        return;
    }

    updateViewHandlers();

    checkGlErrors("before viewer render");

    // Render a wedge for the current material.
    if (_wedgeRequested)
    {
        _wedgeRequested = false;
        mx::ImagePtr wedgeImage = renderWedge();
        if (!wedgeImage || !_imageHandler->saveImage(_wedgeFilename, wedgeImage, true))
        {
            new ng::MessageDialog(this, ng::MessageDialog::Type::Information,
                "Failed to save wedge to disk: ", _wedgeFilename.asString());
        }
    }

    // Render the current frame.
    renderFrame();

    // Capture the current frame.
    if (_captureRequested)
    {
        _captureRequested = false;
        mx::ImagePtr frameImage = getFrameImage();
        if (!frameImage || !_imageHandler->saveImage(_captureFilename, frameImage, true))
        {
            new ng::MessageDialog(this, ng::MessageDialog::Type::Information,
                "Failed to save frame to disk: ", _captureFilename.asString());
        }
    }

    // Bake textures for the current material.
    if (_bakeRequested)
    {
        _bakeRequested = false;
        bakeTextures();
    }

    checkGlErrors("after viewer render");
}

bool Viewer::scrollEvent(const ng::Vector2i& p, const ng::Vector2f& rel)
{
    if (Screen::scrollEvent(p, rel))
    {
        return true;
    }

    if (_userCameraEnabled)
    {
        _userScale = std::max(0.1f, _userScale * ((rel.y() > 0) ? 1.1f : 0.9f));
        return true;
    }

    return false;
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

    // Disable user camera controls when non-centered views are requested.
    _userCameraEnabled = _cameraTarget == mx::Vector3(0.0) &&
                         _meshScale == 1.0f;

    if (_geometryHandler->getMeshes().empty())
    {
        return;
    }
    mx::MeshPtr mesh = _geometryHandler->getMeshes()[0];
    mx::Matrix44 meshRotation = mx::Matrix44::createRotationZ(_meshRotation[2] / 180.0f * PI) *
                                mx::Matrix44::createRotationY(_meshRotation[1] / 180.0f * PI) *
                                mx::Matrix44::createRotationX(_meshRotation[0] / 180.0f * PI);

    if (_userCameraEnabled)
    {
        _meshTranslation = -meshRotation.transformPoint(mesh->getSphereCenter());
        _meshScale = IDEAL_MESH_SPHERE_RADIUS / mesh->getSphereRadius();
    }
}

void Viewer::updateViewHandlers()
{
    float fH = std::tan(_cameraViewAngle / 360.0f * PI) * _cameraNearDist;
    float fW = fH * (float) mSize.x() / (float) mSize.y();

    mx::Matrix44 meshRotation = mx::Matrix44::createRotationZ(_meshRotation[2] / 180.0f * PI) *
                                mx::Matrix44::createRotationY(_meshRotation[1] / 180.0f * PI) *
                                mx::Matrix44::createRotationX(_meshRotation[0] / 180.0f * PI);

    mx::Matrix44 arcball = mx::Matrix44::IDENTITY;
    if (_userCameraEnabled)
    {
        ng::Matrix4f ngArcball = _arcball.matrix();
        arcball = mx::Matrix44(ngArcball.data(), ngArcball.data() + ngArcball.size());
    }

    _cameraViewHandler->worldMatrix = meshRotation *
                                      mx::Matrix44::createTranslation(_meshTranslation + _userTranslation) *
                                      mx::Matrix44::createScale(mx::Vector3(_meshScale * _userScale));
    _cameraViewHandler->viewMatrix = arcball * mx::ViewHandler::createViewMatrix(_cameraPosition, _cameraTarget, _cameraUp);
    _cameraViewHandler->projectionMatrix = mx::ViewHandler::createPerspectiveMatrix(-fW, fW, -fH, fH, _cameraNearDist, _cameraFarDist);

    mx::NodePtr dirLight = _lightHandler->getFirstLightOfCategory(DIR_LIGHT_NODE_CATEGORY);
    if (dirLight)
    {
        mx::MeshPtr mesh = _geometryHandler->getMeshes()[0];
        float r = mesh->getSphereRadius();
        _shadowViewHandler->worldMatrix = meshRotation *
                                          mx::Matrix44::createTranslation(-mesh->getSphereCenter());
        _shadowViewHandler->projectionMatrix = mx::ViewHandler::createOrthographicMatrix(-r, r, -r, r, 0.0f, r * 2.0f);
        mx::ValuePtr value = dirLight->getInputValue("direction");
        if (value->isA<mx::Vector3>())
        {
            mx::Vector3 dir = mx::Matrix44::createRotationY(_lightRotation / 180.0f * PI).transformVector(value->asA<mx::Vector3>());
            _shadowViewHandler->viewMatrix = mx::ViewHandler::createViewMatrix(dir * -r, mx::Vector3(0.0f), _cameraUp);
        }
    }
}

void Viewer::createWedgeParameterInterface(Widget* parent, const std::string& label)
{
    ng::Label* wedgeLabel = new ng::Label(parent, label);
    _wedgeSelectionBox = new ng::ComboBox(parent, { "None" });
    _wedgeSelectionBox->setChevronIcon(-1);
    _wedgeSelectionBox->setFontSize(15);
    _wedgeSelectionBox->setCallback([this](int choice)
    {
        size_t index = (size_t)choice;
        const std::vector<std::string> &wedgeItems = this->_wedgeSelectionBox->items();
        if (!wedgeItems.empty())
        {
            this->_wedgePropertyName = wedgeItems[index];
        }
    });

    std::vector<std::string> wedgeItems;
    MaterialPtr material = getSelectedMaterial();
    if (material)
    {
        mx::DocumentPtr doc = material->getDocument();

        const mx::VariableBlock* publicUniforms = material->getPublicUniforms();
        if (publicUniforms)
        {
            mx::UIPropertyGroup groups;
            mx::UIPropertyGroup unnamedGroups;
            const std::string pathSeparator(":");
            mx::createUIPropertyGroups(*publicUniforms, doc, material->getElement(),
                pathSeparator, groups, unnamedGroups, true);

            for (auto it = groups.begin(); it != groups.end(); ++it)
            {
                const mx::UIPropertyItem& item = it->second;
                if (material->findUniform(item.variable->getPath()) &&
                    (item.value->isA<float>() ||
                    item.value->isA<int>() ||
                    item.value->isA<mx::Vector2>() ||
                    item.value->isA<mx::Vector3>() ||
                    item.value->isA<mx::Color3>() ||
                    item.value->isA<mx::Vector4>() ||
                    item.value->isA<mx::Color4>()))
                {
                    wedgeItems.push_back(item.variable->getPath());
                }
            }
            for (auto it2 = unnamedGroups.begin(); it2 != unnamedGroups.end(); ++it2)
            {
                const mx::UIPropertyItem& item = it2->second;
                if (material->findUniform(item.variable->getPath()) &&
                    (item.value->isA<float>() ||
                    item.value->isA<int>() ||
                    item.value->isA<mx::Vector2>() ||
                    item.value->isA<mx::Vector3>() ||
                    item.value->isA<mx::Color3>() ||
                    item.value->isA<mx::Vector4>() ||
                    item.value->isA<mx::Color4>()))
                {
                    wedgeItems.push_back(item.variable->getPath());
                }
            }
            _wedgeSelectionBox->setItems(wedgeItems);
        }
    }

    _wedgePropertyName.clear();
    bool haveItems = !wedgeItems.empty();
    if (haveItems)
    {
        _wedgePropertyName = wedgeItems[0];
    }
    wedgeLabel->setVisible(haveItems);
    _wedgeSelectionBox->setVisible(haveItems);
}

void Viewer::updateDisplayedProperties()
{
    _propertyEditor.updateContents(this);
    createSaveMaterialsInterface(_propertyEditor.getWindow(), "Save Material");
    createWedgeParameterInterface(_propertyEditor.getWindow(), "Wedge Parameter");
    performLayout();
}

mx::ImagePtr Viewer::getAmbientOcclusionImage(MaterialPtr material)
{
    const mx::string AO_FILENAME_SUFFIX = "_ao";
    const mx::string AO_FILENAME_EXTENSION = "png";

    if (!material || !_genContext.getOptions().hwAmbientOcclusion)
    {
        return nullptr;
    }

    std::string aoSuffix = material->getUdim().empty() ? AO_FILENAME_SUFFIX : AO_FILENAME_SUFFIX + "_" + material->getUdim();
    mx::FilePath aoFilename = _meshFilename;
    aoFilename.removeExtension();
    aoFilename = aoFilename.asString() + aoSuffix;
    aoFilename.addExtension(AO_FILENAME_EXTENSION);
    return _imageHandler->acquireImage(aoFilename, true);
}

void Viewer::splitDirectLight(mx::ImagePtr envRadianceMap, mx::ImagePtr& indirectMap, mx::DocumentPtr& dirLightDoc)
{
    mx::Vector3 lightDir;
    mx::Color3 lightColor;
    mx::ImagePair imagePair = envRadianceMap->splitByLuminance(ENV_MAP_SPLIT_RADIANCE);

    mx::computeDominantLight(imagePair.second, lightDir, lightColor);
    float lightIntensity = std::max(std::max(lightColor[0], lightColor[1]), lightColor[2]);
    if (lightIntensity)
    {
        lightColor /= lightIntensity;
    }

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

    const mx::Matrix44& world = _shadowViewHandler->worldMatrix;
    const mx::Matrix44& view = _shadowViewHandler->viewMatrix;
    const mx::Matrix44& proj = _shadowViewHandler->projectionMatrix;

    mx::ImageSamplingProperties blurSamplingProperties;
    blurSamplingProperties.uaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
    blurSamplingProperties.vaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
    blurSamplingProperties.filterType = mx::ImageSamplingProperties::FilterType::CLOSEST;

    // Create framebuffer.
    mx::GLFrameBufferPtr framebuffer = mx::GLFramebuffer::create(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 2, mx::Image::BaseType::FLOAT);
    framebuffer->bind();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Render shadow geometry.
    _shadowMaterial->bindShader();
    _shadowMaterial->bindViewInformation(world, view, proj);
    for (const auto& assignment : _materialAssignments)
    {
        mx::MeshPartitionPtr geom = assignment.first;
        _shadowMaterial->drawPartition(geom);
    }
    _shadowMap = framebuffer->createColorImage();

    // Apply Gaussian blurring.
    for (unsigned int i = 0; i < _shadowSoftness; i++)
    {
        framebuffer->bind();
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
        _shadowMap = framebuffer->createColorImage();
    }

    // Restore state for scene rendering.
    glViewport(0, 0, mFBSize[0], mFBSize[1]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
}

void Viewer::invalidateShadowMap()
{
    if (_shadowMap)
    {
        _imageHandler->releaseRenderResources(_shadowMap);
        _shadowMap = nullptr;
    }
}

void Viewer::updateAlbedoTable()
{
    if (_lightHandler->getAlbedoTable())
    {
        return;
    }

    // Create framebuffer.
    mx::GLFrameBufferPtr framebuffer = mx::GLFramebuffer::create(ALBEDO_TABLE_SIZE, ALBEDO_TABLE_SIZE, 2, mx::Image::BaseType::FLOAT);
    framebuffer->bind();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Create shader.
    mx::ShaderPtr hwShader = mx::createAlbedoTableShader(_genContext, _stdLib, "__ALBEDO_TABLE_SHADER__");
    MaterialPtr material = Material::create();
    try
    {
        material->generateShader(hwShader);
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to generate albedo table shader", e.what());
        return;
    }

    // Render albedo table.
    material->bindShader();
    if (material->getShader()->uniform(mx::HW::ALBEDO_TABLE_SIZE, false) != -1)
    {
        material->getShader()->setUniform(mx::HW::ALBEDO_TABLE_SIZE, ALBEDO_TABLE_SIZE);
    }
    mx::GlslRenderer::drawScreenSpaceQuad();

    // Store albedo table image.
    _imageHandler->releaseRenderResources(_lightHandler->getAlbedoTable());
    _lightHandler->setAlbedoTable(framebuffer->createColorImage());
    if (_saveGeneratedLights)
    {
        _imageHandler->saveImage("AlbedoTable.exr", _lightHandler->getAlbedoTable());
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
