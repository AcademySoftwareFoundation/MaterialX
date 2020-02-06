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

namespace {

mx::Matrix44 createViewMatrix(const mx::Vector3& eye,
                              const mx::Vector3& target,
                              const mx::Vector3& up)
{
    mx::Vector3 z = (target - eye).getNormalized();
    mx::Vector3 x = z.cross(up).getNormalized();
    mx::Vector3 y = x.cross(z);

    return mx::Matrix44(
         x[0],  x[1],  x[2], -x.dot(eye),
         y[0],  y[1],  y[2], -y.dot(eye),
        -z[0], -z[1], -z[2],  z.dot(eye),
         0.0f,  0.0f,  0.0f,  1.0f);
}

mx::Matrix44 createPerspectiveMatrix(float left, float right,
                                     float bottom, float top,
                                     float nearP, float farP)
{
    return mx::Matrix44(
        (2.0f * nearP) / (right - left), 0.0f, (right + left) / (right - left), 0.0f,
        0.0f, (2.0f * nearP) / (top - bottom), (top + bottom) / (top - bottom), 0.0f,
        0.0f, 0.0f, -(farP + nearP) / (farP - nearP), -(2.0f * farP * nearP) / (farP - nearP),
        0.0f, 0.0f, -1.0f, 0.0f);
}

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
    _zoom(1.0f),
    _viewAngle(45.0f),
    _nearDist(0.05f),
    _farDist(5000.0f),
    _modelZoom(1.0f),
    _translationActive(false),
    _translationStart(0, 0),
    _libraryFolders(libraryFolders),
    _searchPath(searchPath),
    _materialFilename(materialFilename),
    _modifiers(modifiers),
    _envRadiancePath(envRadiancePath),
    _directLighting(false),
    _indirectLighting(true),
    _ambientOcclusionGain(0.85f),
    _meshFilename(meshFilename),
    _selectedGeom(0),
    _selectedMaterial(0),
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
    _drawUVGeometry(false),
    _uvScale(2.0f, 2.0f, 1.0f),
    _uvTranslation(-0.5f, 0.5f, 0.0f),
    _uvZoom(1.0f)
{
    _window = new ng::Window(this, "Viewer Options");
    _window->setPosition(ng::Vector2i(15, 15));
    _window->setLayout(new ng::GroupLayout());

    // Initialize the standard libraries and color/unit management.
    loadStandardLibraries();

    // Set default generator options.
    _genContext.getOptions().hwTransparency = true;
    _genContext.getOptions().hwSpecularEnvironmentMethod = _specularEnvironmentMethod;
    _genContext.getOptions().targetColorSpaceOverride = "lin_rec709";
    _genContext.getOptions().fileTextureVerticalFlip = true;

    // Initialize image handler.
#if MATERIALX_BUILD_OIIO
    mx::ImageLoaderPtr imageLoader = mx::OiioImageLoader::create();
#else
    mx::ImageLoaderPtr imageLoader = mx::StbImageLoader::create();
#endif
    _imageHandler = mx::GLTextureHandler::create(imageLoader);

    // Initialize light handler.
    _lightHandler = mx::LightHandler::create();
    _lightFilename = "resources/Materials/TestSuite/Utilities/Lights/default_viewer_lights.mtlx";
    _lightDoc = mx::createDocument();
    mx::XmlReadOptions options;
    mx::readFromXmlFile(_lightDoc, _lightFilename, _searchPath, &options);

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
    updateGeometrySelections();

    // Create environment geometry handler.
    _envGeometryHandler = mx::GeometryHandler::create();
    _envGeometryHandler->addLoader(loader);
    mx::FilePath envSphere("resources/Geometry/sphere.obj");
    _envGeometryHandler->loadGeometry(_searchPath.find(envSphere));

    // Initialize environment light.
    loadEnvironmentLight();

    // Generate wireframe material.
    const std::string constantShaderName("__WIRE_SHADER_NAME__");
    const mx::Color3 color(1.0f);
    _wireMaterial = Material::create();
    try
    {
        _wireMaterial->generateConstantShader(_genContext, _stdLib, constantShaderName, color);
    }
    catch (std::exception& e)
    {
        _wireMaterial = nullptr;
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to generate wire shader", e.what());
    }

    // Initialize camera
    initCamera();
    setResizeCallback([this](ng::Vector2i size)
    {
        _arcball.setSize(size);
    });

    loadDocument(_materialFilename, _stdLib);
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

    try
    {
        // Look for an irradiance map using an expected filename convention.
        const std::string IRRADIANCE_MAP_SUFFIX("_diffuse");
        mx::FilePath envIrradiancePath = _envRadiancePath.getParentPath();
        std::string envIrradianceName = mx::removeExtension(_envRadiancePath.getBaseName()) +
            IRRADIANCE_MAP_SUFFIX + "." + _envRadiancePath.getExtension();
        envIrradiancePath = envIrradiancePath / envIrradianceName;
        mx::ImagePtr envIrradianceMap = _imageHandler->acquireImage(_searchPath.find(envIrradiancePath), true, nullptr, &message);

        // If not found, then generate an irradiance map via spherical harmonics.
        if (!envIrradianceMap)
        {
            mx::Sh3ColorCoeffs shIrradiance = mx::projectEnvironment(envRadianceMap, true);
            envIrradianceMap = mx::renderEnvironment(shIrradiance, 256, 128);
        }

        // Release any existing environment maps and store the new ones.
        _imageHandler->releaseRenderResources(_lightHandler->getEnvRadianceMap());
        _imageHandler->releaseRenderResources(_lightHandler->getEnvIrradianceMap());
        _lightHandler->setEnvRadianceMap(envRadianceMap);
        _lightHandler->setEnvIrradianceMap(envIrradianceMap);

        const mx::MeshList& meshes = _envGeometryHandler->getMeshes();
        if (!meshes.empty())
        {
            // Set up world matrix for drawing
            const float scaleFactor = 300.0f;
            const float rotationRadians = PI / 2.0f; // 90 degree rotation 
            _envMatrix = mx::Matrix44::createScale(mx::Vector3(scaleFactor)) * mx::Matrix44::createRotationY(rotationRadians);

            // Create environment shader.
            mx::FilePath envFilename = _searchPath.find(
                mx::FilePath("resources/Materials/TestSuite/Utilities/Lights/envmap_shader.mtlx"));
            _envMaterial = Material::create();
            _envMaterial->generateEnvironmentShader(_genContext, envFilename, _stdLib, _envRadiancePath);
            _envMaterial->bindMesh(_envGeometryHandler->getMeshes()[0]);
        }
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to load environment light", e.what());
    }
}

void Viewer::applyDirectLights(mx::DocumentPtr doc)
{
    if (!_lightDoc)
    {
        return;
    }

    mx::CopyOptions copyOptions;
    copyOptions.skipConflictingElements = true;
    doc->importLibrary(_lightDoc, &copyOptions);
    _xincludeFiles.insert(_lightFilename);

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

    ng::CheckBox* transparencyBox = new ng::CheckBox(advancedPopup, "Render Transparency");
    transparencyBox->setChecked(_genContext.getOptions().hwTransparency);
    transparencyBox->setCallback([this](bool enable)
    {
        _genContext.getOptions().hwTransparency = enable;
        reloadShaders();
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
        mx::ValuePtr udimSetValue = doc->getGeomAttrValue("udimset");

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
                else
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
            // Ignore transparency for now as we can't know from the source code 
            // if the shader is transparent or not.
            if (material->loadSource(vertexShaderFile, pixelShaderFile, baseName, false))
            {
                assignMaterial(getSelectedGeometry(), material);
                new ng::MessageDialog(this, ng::MessageDialog::Type::Information, "Loaded GLSL source: ", baseName);
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
                _captureFrameFileName = fileName;
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

    if ((key == GLFW_KEY_U) && (action == GLFW_PRESS))
    {
        _drawUVGeometry = !_drawUVGeometry;
        return true;
    }

    return false;
}

void Viewer::drawScene3D()
{
    mx::Matrix44 world, view, proj;
    computeCameraMatrices(world, view, proj);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glEnable(GL_FRAMEBUFFER_SRGB);

    // Environment background
    if (_drawEnvironment && _envMaterial)
    {
        GLShaderPtr envShader = _envMaterial->getShader();
        auto meshes = _envGeometryHandler->getMeshes();
        auto envPart = !meshes.empty() ? meshes[0]->getPartition(0) : nullptr;
        if (envShader && envPart)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            envShader->bind();
            _envMaterial->bindViewInformation(_envMatrix, view, proj);
            _envMaterial->bindImages(_imageHandler, _searchPath);
            _envMaterial->drawPartition(envPart);
            glDisable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
    }

    // Opaque pass
    glDisable(GL_BLEND);
    for (const auto& assignment : _materialAssignments)
    {
        mx::MeshPartitionPtr geom = assignment.first;
        MaterialPtr material = assignment.second;
        if (!material || material->hasTransparency())
        {
            continue;
        }

        material->bindShader();
        material->bindViewInformation(world, view, proj);
        material->bindLights(_lightHandler, _imageHandler,
                             _directLighting, _indirectLighting,
                             getAmbientOcclusionImage(material), _ambientOcclusionGain,
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
        if (!material || !material->hasTransparency())
        {
            continue;
        }

        material->bindShader();
        material->bindViewInformation(world, view, proj);
        material->bindLights(_lightHandler, _imageHandler,
                             _directLighting, _indirectLighting,
                             getAmbientOcclusionImage(material), _ambientOcclusionGain,
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
        GLShaderPtr shader = _wireMaterial->getShader();
        if (shader && (_selectedGeom < _geometryList.size()))
        {
            auto activeGeom = _geometryList[_selectedGeom];
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            shader->bind();
            _wireMaterial->bindViewInformation(world, view, proj);
            _wireMaterial->drawPartition(activeGeom);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

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
        bool saved = _imageHandler->saveImage(_captureFrameFileName, image, true);
        if (!saved)
        {
            new ng::MessageDialog(this, ng::MessageDialog::Type::Information,
                "Failed to save frame to disk: ", _captureFrameFileName.asString());
        }
    }
}

mx::MeshStreamPtr Viewer::createUvPositionStream(mx::MeshPtr mesh, 
                                                 const std::string& uvStreamName, 
                                                 unsigned int index,
                                                 const std::string& positionStreamName)
{
    // If there are no uvs to display then just return an empty 3d stream
    mx::MeshStreamPtr uvStream2D = mesh->getStream(uvStreamName, index);
    if (!uvStream2D)
    {
        return nullptr;
    }

    mx::MeshStreamPtr uvStream3D = mesh->getStream(positionStreamName);
    if (!uvStream3D)
    {
        uvStream3D = mx::MeshStream::create(positionStreamName, mx::MeshStream::POSITION_ATTRIBUTE, 0);
        mesh->addStream(uvStream3D);

        mx::MeshFloatBuffer &uvPos2D = uvStream2D->getData();
        mx::MeshFloatBuffer &uvPos3D = uvStream3D->getData();
        size_t uvCount = uvPos2D.size() / 2;
        uvPos3D.resize(uvCount * 3);
        const float MAX_FLOAT = std::numeric_limits<float>::max();
        mx::Vector3 boxMin = { MAX_FLOAT, MAX_FLOAT, 0.0f };
        mx::Vector3 boxMax = { -MAX_FLOAT, -MAX_FLOAT, 0.0f };
        for (size_t i = 0; i < uvCount; i++)
        {
            float u = uvPos2D[i * 2];
            uvPos3D[i * 3] = u;
            float v = uvPos2D[i * 2 + 1];
            uvPos3D[i * 3 + 1] = v;
            uvPos3D[i * 3 + 2] = 0.0f;

            boxMin[0] = std::min(u, boxMin[0]);
            boxMin[1] = std::min(v, boxMin[1]);
            boxMax[0] = std::max(u, boxMax[0]);
            boxMax[1] = std::max(v, boxMax[1]);
        }

        mx::Vector3 sphereCenter = (boxMax + boxMin) / 2.0;
        float sphereRadius = (sphereCenter - boxMin).getMagnitude();
        _uvScale[0] = 2.0f / sphereRadius;
        _uvScale[1] = 2.0f / sphereRadius;
        _uvScale[2] = 1.0f;
        _uvTranslation[0] = -sphereCenter[0];
        _uvTranslation[1] = -sphereCenter[1];
        _uvTranslation[2] = 0.0f;
    }

    return uvStream3D;
}

void Viewer::drawScene2D()
{
    // Create uv shader if it does not exist
    if (!_wireMaterialUV)
    {
        const std::string shaderName("__UV_WIRE_SHADER_NAME__");
        const mx::Color3 color(1.0f);
        _wireMaterialUV = Material::create();
        try
        {
            _wireMaterialUV->generateConstantShader(_genContext, _stdLib, shaderName, color);
        }
        catch (std::exception& e)
        {
            _wireMaterialUV = nullptr;
            std::cerr << "Failed to generate uv wire shader: " << e.what();
            return;
        }
    }

    GLShaderPtr shader = _wireMaterialUV->getShader();
    if (!shader || _geometryList.empty())
    {
        return;
    }
    if (shader->attrib("i_position") == -1)
    {
        return;
    }

    // Create and bind uvs as input positions
    mx::MeshPtr mesh = _geometryHandler->getMeshes()[0];
    const std::string uvStream3DName(mx::MeshStream::TEXCOORD_ATTRIBUTE + "_3D");
    mx::MeshStreamPtr uvStream3D = createUvPositionStream(mesh, 
                                                          mx::MeshStream::TEXCOORD_ATTRIBUTE, 0,
                                                          uvStream3DName);
    if (!uvStream3D)
    {
        return;
    }
    mx::MeshFloatBuffer &buffer = uvStream3D->getData();
    Eigen::Map<const ng::MatrixXf> positions(&buffer[0], uvStream3D->getStride(), buffer.size() / uvStream3D->getStride());

    shader->bind();
    shader->uploadAttrib("i_position", positions);

    // Compute matrices
    mx::Matrix44 world, view, proj;
    float fH = std::tan(_viewAngle / 360.0f * PI) * _nearDist;
    float fW = fH * (float)mSize.x() / (float)mSize.y();
    view = createViewMatrix(_eye, _center, _up);
    proj = createPerspectiveMatrix(-fW, fW, -fH, fH, _nearDist, _farDist);
    world = mx::Matrix44::createScale(_uvScale * _uvZoom);
    world *= mx::Matrix44::createTranslation(_uvTranslation).getTranspose();

    _wireMaterialUV->bindViewInformation(world, view, proj);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (_outlineSelection && (_selectedGeom < _geometryList.size()))
    {
        mx::MeshPartitionPtr activeGeom = _geometryList[_selectedGeom];
        _wireMaterialUV->drawPartition(activeGeom);
    }
    else
    {
        for (mx::MeshPartitionPtr geom : _geometryList)
        {
            _wireMaterialUV->drawPartition(geom);
        }
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Viewer::drawContents()
{
    if (_geometryList.empty() || _materials.empty())
    {
        return;
    }
    if (_drawUVGeometry)
    {
        drawScene2D();
    }
    else
    {
        drawScene3D();
    }
}

bool Viewer::scrollEvent(const ng::Vector2i& p, const ng::Vector2f& rel)
{
    if (!Screen::scrollEvent(p, rel))
    {
        if (_drawUVGeometry)
        {
            _uvZoom = std::max(0.1f, _uvZoom * ((rel.y() > 0) ? 1.1f : 0.9f));
        }
        else
        {
            _zoom = std::max(0.1f, _zoom * ((rel.y() > 0) ? 1.1f : 0.9f));;
        }
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

    if (_drawUVGeometry)
    {
        return true;
    }

    if (_arcball.motion(p))
    {
        return true;
    }

    if (_translationActive)
    {
        mx::Matrix44 world, view, proj;
        computeCameraMatrices(world, view, proj);
        mx::Matrix44 worldView = view * world;

        mx::MeshPtr mesh = _geometryHandler->getMeshes()[0];
        mx::Vector3 boxMin = mesh->getMinimumBounds();
        mx::Vector3 boxMax = mesh->getMaximumBounds();
        mx::Vector3 sphereCenter = (boxMax + boxMin) / 2.0;

        float zval = ng::project(ng::Vector3f(sphereCenter.data()),
                                 ng::Matrix4f(worldView.getTranspose().data()),
                                 ng::Matrix4f(proj.getTranspose().data()),
                                 mSize).z();
        ng::Vector3f pos1 = ng::unproject(ng::Vector3f((float) p.x(),
                                                       (float) (mSize.y() - p.y()),
                                                       (float) zval),
                                          ng::Matrix4f(worldView.getTranspose().data()),
                                          ng::Matrix4f(proj.getTranspose().data()),
                                          mSize);
        ng::Vector3f pos0 = ng::unproject(ng::Vector3f((float) _translationStart.x(),
                                                       (float) (mSize.y() - _translationStart.y()),
                                                       (float) zval),
                                          ng::Matrix4f(worldView.getTranspose().data()),
                                          ng::Matrix4f(proj.getTranspose().data()),
                                          mSize);
        ng::Vector3f delta = pos1 - pos0;
        _modelTranslation = _modelTranslationStart +
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

    if (_drawUVGeometry)
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
        _modelTranslationStart = _modelTranslation;
        _translationActive = true;
        _translationStart = p;
    }
    if (button == GLFW_MOUSE_BUTTON_1 && !down)
    {
        _arcball.button(p, false);
    }
    if (!down)
    {
        _translationActive = false;
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
    mx::Vector3 sphereCenter = (boxMax + boxMin) / 2.0;
    float sphereRadius = (sphereCenter - boxMin).getMagnitude();
    _modelZoom = 2.0f / sphereRadius;
    _modelTranslation = sphereCenter * -1.0f;
}

void Viewer::computeCameraMatrices(mx::Matrix44& world,
                                   mx::Matrix44& view,
                                   mx::Matrix44& proj)
{
    float fH = std::tan(_viewAngle / 360.0f * PI) * _nearDist;
    float fW = fH * (float) mSize.x() / (float) mSize.y();

    ng::Matrix4f ngArcball = _arcball.matrix();
    mx::Matrix44 arcball = mx::Matrix44(ngArcball.data(), ngArcball.data() + ngArcball.size()).getTranspose();

    view = createViewMatrix(_eye, _center, _up) * arcball;
    proj = createPerspectiveMatrix(-fW, fW, -fH, fH, _nearDist, _farDist);
    world = mx::Matrix44::createScale(mx::Vector3(_zoom * _modelZoom));
    world *= mx::Matrix44::createTranslation(_modelTranslation).getTranspose();
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
    mx::FilePath aoFilePath = _meshFilename.getParentPath();
    std::string aoFilename = mx::removeExtension(_meshFilename.getBaseName()) + aoSuffix + "." + AO_FILENAME_EXTENSION;

    _imageHandler->setSearchPath(_searchPath);
    return _imageHandler->acquireImage(aoFilePath / aoFilename, true, &AO_FALLBACK_COLOR);
}
