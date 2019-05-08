#include <MaterialXView/Viewer.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXRender/Util.h>
#include <MaterialXRender/OiioImageLoader.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/TinyObjLoader.h>

#include <nanogui/button.h>
#include <nanogui/combobox.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/messagedialog.h>

#include <iostream>
#include <fstream>

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

void writeTextFile(const std::string& text, const std::string& filePath)
{
    std::ofstream file;
    file.open(filePath);
    file << text;
    file.close();
}

mx::DocumentPtr loadLibraries(const mx::StringVec& libraryFolders, const mx::FileSearchPath& searchPath)
{
    mx::DocumentPtr doc = mx::createDocument();
    for (const std::string& libraryFolder : libraryFolders)
    {
        mx::FilePath path = searchPath.find(libraryFolder);
        mx::FilePathVec filenames = path.getFilesInDirectory("mtlx");

        for (const std::string& filename : filenames)
        {
            mx::FilePath file = path / filename;
            mx::DocumentPtr libDoc = mx::createDocument();
            mx::readFromXmlFile(libDoc, file);
            libDoc->setSourceUri(file);
            mx::CopyOptions copyOptions;
            copyOptions.skipDuplicateElements = true;
            doc->importLibrary(libDoc, &copyOptions);
        }
    }
    return doc;
}

} // anonymous namespace

//
// Viewer methods
//

Viewer::Viewer(const mx::StringVec& libraryFolders,
               const mx::FileSearchPath& searchPath,
               const std::string& meshFilename,
               const std::string& materialFilename,
               const DocumentModifiers& modifiers,
               mx::HwSpecularEnvironmentMethod specularEnvironmentMethod,
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
    _directLighting(false),
    _indirectLighting(true),
    _selectedGeom(0),
    _selectedMaterial(0),
    _genContext(mx::GlslShaderGenerator::create()),
    _splitByUdims(false),
    _mergeMaterials(false),
    _assignLooks(false),
    _outlineSelection(false),
    _specularEnvironmentMethod(specularEnvironmentMethod),
    _envSamples(DEFAULT_ENV_SAMPLES),
    _drawEnvironment(false),
    _captureFrame(false)
{
    _window = new ng::Window(this, "Viewer Options");
    _window->setPosition(ng::Vector2i(15, 15));
    _window->setLayout(new ng::GroupLayout());

    createLoadMeshInterface(_window, "Load Mesh");
    createLoadMaterialsInterface(_window, "Load Material");

    ng::Button* editorButton = new ng::Button(_window, "Property Editor");
    editorButton->setFlags(ng::Button::ToggleButton);
    editorButton->setChangeCallback([this](bool state)
    {
        _propertyEditor.setVisible(state);
        performLayout();
    });

    createAdvancedSettings(_window);

    _geomLabel = new ng::Label(_window, "Select Geometry");
    _geometryListBox = new ng::ComboBox(_window, { "None" });
    _geometryListBox->setChevronIcon(-1);
    _geometryListBox->setCallback([this](int choice)
    {
        setGeometrySelection(choice);
    });

    _materialLabel = new ng::Label(_window, "Assigned Material");
    _materialSelectionBox = new ng::ComboBox(_window, { "None" });
    _materialSelectionBox->setChevronIcon(-1);
    _materialSelectionBox->setCallback([this](int choice)
    {
        setMaterialSelection(choice);
        assignMaterial(_materials[_selectedMaterial], _geometryList[_selectedGeom]);
    });

    // Set default generator options.
    _genContext.getOptions().hwSpecularEnvironmentMethod = _specularEnvironmentMethod;
    _genContext.getOptions().targetColorSpaceOverride = "lin_rec709";
    _genContext.getOptions().fileTextureVerticalFlip = true;

    // Set default light information before initialization
    _lightFileName = "resources/Materials/TestSuite/Utilities/Lights/default_viewer_lights.mtlx";
    _envRadiancePath = "resources/Images/san_giuseppe_bridge.hdr";
    _envIrradiancePath = "resources/Images/san_giuseppe_bridge_diffuse.hdr";

    // Load in standard library and light handler and create top level document
    _stdLib = loadLibraries(_libraryFolders, _searchPath);
    initializeDocument(_stdLib);

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

    // Construct the appropriate image handler for this build.
#if MATERIALX_BUILD_OIIO
    mx::ImageLoaderPtr imageLoader = mx::OiioImageLoader::create();
#else
    mx::ImageLoaderPtr imageLoader = mx::StbImageLoader::create();
#endif
    _imageHandler = mx::GLTextureHandler::create(imageLoader);

    mx::TinyObjLoaderPtr loader = mx::TinyObjLoader::create();
    _geometryHandler = mx::GeometryHandler::create();
    _geometryHandler->addLoader(loader);
    _geometryHandler->loadGeometry(_searchPath.find(meshFilename));
    updateGeometrySelections();

    _envGeometryHandler = mx::GeometryHandler::create();
    _envGeometryHandler->addLoader(loader);
    mx::FilePath envSphere("resources/Geometry/sphere.obj");
    _envGeometryHandler->loadGeometry(_searchPath.find(envSphere));
    const mx::MeshList& meshes = _envGeometryHandler->getMeshes();
    if (!meshes.empty())
    {
        // Invert u and rotate 90 degrees.
        mx::MeshPtr mesh = meshes[0];
        mx::MeshStreamPtr stream = mesh->getStream(mx::MeshStream::TEXCOORD_ATTRIBUTE, 0);
        mx::MeshFloatBuffer &buffer = stream->getData();
        size_t stride = stream->getStride();
        for (size_t i = 0; i < buffer.size() / stride; i++)
        {
            float val = (buffer[i*stride] * -1.0f) + 1.25f;
            buffer[i*stride] = (val < 0.0f) ? (val + 1.0f) : ((val > 1.0f) ? val -= 1.0f : val);
        }

        // Set up world matrix for drawing
        const float scaleFactor = 300.0f;
        _envMatrix = mx::Matrix44::createScale(mx::Vector3(scaleFactor));

        const std::string envShaderName("__ENV_SHADER_NAME__");
        _envMaterial = Material::create();
        try
        {
            const std::string addressMode("clamp");
            _envMaterial->generateImageShader(_genContext, _stdLib, envShaderName, _envRadiancePath, addressMode);
            _envMaterial->bindMesh(_envGeometryHandler->getMeshes()[0]);
        }
        catch (std::exception& e)
        {
            _envMaterial = nullptr;
            new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to generate environment shader", e.what());
        }
    }

    // Initialize camera
    initCamera();
    setResizeCallback([this](ng::Vector2i size)
    {
        _arcball.setSize(size);
    });

    try
    {
        Material::loadDocument(_doc, _searchPath.find(_materialFilename), _stdLib, _modifiers, _materials);
        updateMaterialSelections();
        setMaterialSelection(0);
        if (!_materials.empty())
        {
            assignMaterial(_materials[0]);
        }
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to load materials", e.what());
    }

    updatePropertyEditor();
    _propertyEditor.setVisible(false);
    performLayout();
}

void Viewer::setupLights(mx::DocumentPtr doc, const std::string& envRadiancePath, const std::string& envIrradiancePath)
{
    mx::CopyOptions copyOptions;
    copyOptions.skipDuplicateElements = true;

    // Import lights
    mx::DocumentPtr lightDoc = mx::createDocument();
    mx::FilePath path = _searchPath.find(_lightFileName);
    if (!path.isEmpty())
    {
        try
        {
            mx::readFromXmlFile(lightDoc, path.asString());
            lightDoc->setSourceUri(path);
            _doc->importLibrary(lightDoc, &copyOptions);
        }
        catch (std::exception& e)
        {
            new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Cannot load light library file: " + path.asString(), e.what());
        }
    }

    // Scan for lights
    std::vector<mx::NodePtr> lights;
    for (mx::NodePtr node : doc->getNodes())
    {
        const mx::TypeDesc* type = mx::TypeDesc::get(node->getType());
        if (type == mx::Type::LIGHTSHADER)
        {
            lights.push_back(node);
        }
    }

    // Create a new light handler
    _lightHandler = mx::LightHandler::create();

    try 
    {
        // Set lights on the generator. Set to empty if no lights found
        _lightHandler->setLightSources(lights);

        if (!lights.empty())
        {
            // Create a list of unique nodedefs and ids for them
            std::unordered_map<std::string, unsigned int> identifiers;
            _lightHandler->mapNodeDefToIdentiers(lights, identifiers);
            for (auto id : identifiers)
            {
                mx::NodeDefPtr nodeDef = _doc->getNodeDef(id.first);
                if (nodeDef)
                {
                    mx::HwShaderGenerator::bindLightShader(*nodeDef, id.second, _genContext);
                }
                else
                {
                    new ng::MessageDialog(this, ng::MessageDialog::Type::Information, "Failed to light nodedef: ", id.first);
                }
            }

            // Clamp the number of light sources to the number found
            unsigned int lightSourceCount = static_cast<unsigned int>(_lightHandler->getLightSources().size());
            _genContext.getOptions().hwMaxActiveLightSources = lightSourceCount;
        }

        // Set up IBL inputs
        _lightHandler->setLightEnvRadiancePath(_searchPath.find(envRadiancePath));
        _lightHandler->setLightEnvIrradiancePath(_searchPath.find(envIrradiancePath));
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to set up lighting", e.what());
    }
}

void Viewer::initializeDocument(mx::DocumentPtr libraries)
{
    _doc = mx::createDocument();
    mx::CopyOptions copyOptions;
    copyOptions.skipDuplicateElements = true;
    _doc->importLibrary(libraries, &copyOptions);

    // Add color management to generator
    mx::DefaultColorManagementSystemPtr cms = mx::DefaultColorManagementSystem::create(_genContext.getShaderGenerator().getLanguage());
    cms->loadLibrary(_doc);
    for (size_t i = 0; i < _searchPath.size(); i++)
    {
        // TODO: The registerSourceCodeSearchPath method should probably take a
        //       full FileSearchPath rather than a single FilePath.
        _genContext.registerSourceCodeSearchPath(_searchPath[i]);
    }
    _genContext.getShaderGenerator().setColorManagementSystem(cms);

    // Clear user data if previously used.
    _genContext.clearUserData();

    // Add lighting 
    setupLights(_doc, _envRadiancePath, _envIrradiancePath);

    // Clear state
    _materials.clear();
    _selectedMaterial = 0;
    _materialAssignments.clear();
}

void Viewer::assignMaterial(MaterialPtr material, mx::MeshPartitionPtr geometry)
{
    const mx::MeshList& meshes = _geometryHandler->getMeshes();
    if (meshes.empty())
    {
        return;
    }

    material->bindMesh(meshes[0]);

    // Assign to a given mesh
    if (geometry)
    {
        if (_materialAssignments.count(geometry))
        {
            _materialAssignments[geometry] = material;
        }
    }
    // Assign to all meshes
    else
    {
        for (auto geom : _geometryList)
        {
            _materialAssignments[geom] = material;
        }
    }
}

void Viewer::createLoadMeshInterface(Widget* parent, const std::string label)
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
                const mx::MeshList& meshes = _geometryHandler->getMeshes();
                if (!meshes.empty())
                {
                    if (_splitByUdims)
                    {
                        meshes[0]->splitByUdims();
                    }
                }

                updateGeometrySelections();
                // Clear out any previous assignments
                _materialAssignments.clear();
                // Bind the currently selected material (if any) to the geometry loaded in
                MaterialPtr material = getSelectedMaterial();
                if (material)
                {
                    assignMaterial(material);
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

void Viewer::createLoadMaterialsInterface(Widget* parent, const std::string label)
{
    ng::Button* materialButton = new ng::Button(parent, label);
    materialButton->setIcon(ENTYPO_ICON_FOLDER);
    materialButton->setCallback([this]()
    {
        mProcessEvents = false;
        std::string filename = ng::file_dialog({ { "mtlx", "MaterialX" } }, false);
        if (!filename.empty())
        {
            // Try loading new materials first
            {
                _materialFilename = filename;
                try
                {
                    if (!_mergeMaterials)
                    {
                        initializeDocument(_stdLib);
                    }
                    size_t newRenderables = Material::loadDocument(_doc, _searchPath.find(_materialFilename), _stdLib, _modifiers, _materials);
                    if (newRenderables > 0)
                    {
                        updateMaterialSelections();

                        // Clear cached implementations in case a nodedef 
                        // or nodegraph has changed on disc.
                        _genContext.clearNodeImplementations();

                        mx::MeshPtr mesh = _geometryHandler->getMeshes()[0];
                        for (auto m : _materials)
                        {
                            m->generateShader(_genContext);
                            if (mesh)
                            {
                                m->bindMesh(mesh);
                            }
                        }
                        if (!_mergeMaterials)
                        {
                            setMaterialSelection(0);
                            if (!_materials.empty())
                            {
                                assignMaterial(_materials[0]);
                            }
                        }
                    }
                }
                catch (std::exception& e)
                {
                    new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Material Assignment Error", e.what());
                }
            }

            // Then try loading looks
            if (_assignLooks)
            {
                try
                {
                    // Assign any materials found to geometry
                    mx::DocumentPtr lookDoc = mx::createDocument();
                    mx::readFromXmlFile(lookDoc, filename);
                    std::vector<mx::LookPtr> looks = lookDoc->getLooks();
                    // For now only handle the first look as sets of looks are not stored
                    if (!looks.empty())
                    {
                        auto look = looks[0];
                        std::vector<mx::MaterialAssignPtr> assignments = look->getMaterialAssigns();
                        for (auto assignment : assignments)
                        {
                            // Find if material exists
                            std::string materialName = assignment->getMaterial();

                            MaterialPtr assignedMaterial = nullptr;
                            size_t assigneMaterialIndex = 0;
                            for (size_t i = 0; i<_materials.size(); i++)
                            {
                                MaterialPtr mat = _materials[i];
                                mx::TypedElementPtr elem = mat->getElement();
                                mx::ShaderRefPtr shaderRef = elem->asA<mx::ShaderRef>();
                                mx::MaterialPtr materialRef = shaderRef ? shaderRef->getParent()->asA<mx::Material>() : nullptr;
                                if (materialRef)
                                {
                                    if (materialRef->getName() == materialName)
                                    {
                                        assignedMaterial = mat;
                                        assigneMaterialIndex = i;
                                        break;
                                    }
                                }
                            }
                            if (!assignedMaterial)
                            {
                                continue;
                            }

                            // Find if geometry to assign exists and if so assign
                            // material to it.
                            mx::StringVec geomList;
                            std::string geom = assignment->getGeom();
                            if (!geom.empty())
                            {
                                geomList.push_back(geom);
                            }
                            else
                            {
                                mx::CollectionPtr collection = assignment->getCollection();
                                const std::string geomListString = collection->getIncludeGeom();
                                geomList = mx::splitString(geomListString, ",");
                            }

                            for (auto geomName : geomList)
                            {
                                for (size_t p = 0; p < _geometryList.size(); p++)
                                {
                                    auto partition = _geometryList[p];
                                    const std::string& id = partition->getIdentifier();
                                    if (geomName == id)
                                    {
                                        // Mimic what is done manually by choosing the geometry 
                                        // and then choosing the material to assign to that geometry
                                        setGeometrySelection(p);
                                        setMaterialSelection(assigneMaterialIndex);
                                    }
                                }
                            }

                        }
                    }
                }
                catch (std::exception& e)
                {
                    new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Look Assignment Error", e.what());
                }
            }
        }
        mProcessEvents = true;
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

    ng::CheckBox* mergeMaterialsBox = new ng::CheckBox(advancedPopup, "Add Materials");
    mergeMaterialsBox->setChecked(_mergeMaterials);
    mergeMaterialsBox->setCallback([this](bool enable)
    {
        _mergeMaterials = enable;
    });    

    ng::CheckBox* assignLooksBox = new ng::CheckBox(advancedPopup, "Assign Looks");
    assignLooksBox->setChecked(_assignLooks);
    assignLooksBox->setCallback([this](bool enable)
    {
        _assignLooks = enable;
    });

    new ng::Label(advancedPopup, "Lighting Options");

    ng::CheckBox* directLightingBox = new ng::CheckBox(advancedPopup, "Direct lighting");
    directLightingBox->setChecked(_directLighting);
    directLightingBox->setCallback([this](bool enable)
    {
        _directLighting = enable;
    });

    ng::CheckBox* indirectLightingBox = new ng::CheckBox(advancedPopup, "Indirect lighting");
    indirectLightingBox->setChecked(_indirectLighting);
    indirectLightingBox->setCallback([this](bool enable)
    {
        _indirectLighting = enable;
    });

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
            _envSamples = MIN_ENV_SAMPLES * (int)std::pow(4, index);
        });
    }
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
    _geometryListBox->setItems(items);

    _geomLabel->setVisible(items.size() > 1);
    _geometryListBox->setVisible(items.size() > 1);
    _selectedGeom = 0;

    performLayout();
}

bool Viewer::setGeometrySelection(size_t index)
{
    if (index < _geometryList.size())
    {
        _geometryListBox->setSelectedIndex((int)index);
        _selectedGeom = index;
        return true;
    }
    return false;
}

void Viewer::updateMaterialSelections()
{
    std::vector<std::string> items;
    for (auto material : _materials)
    {
        mx::ElementPtr displayElem = material->getElement();
        if (displayElem->isA<mx::ShaderRef>())
        {
            displayElem = displayElem->getParent();
        }
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

    _selectedMaterial = 0;

    performLayout();
}

MaterialPtr Viewer::setMaterialSelection(size_t index)
{
    if (index >= _materials.size())
    {
        return nullptr;
    }

    // Update UI selection and internal value
    _selectedMaterial = index;
    _materialSelectionBox->setSelectedIndex((int)index);

    // Update shader if needed
    MaterialPtr material = _materials[index];
    if (material->generateShader(_genContext))
    {
        // Record assignment
        if (!_geometryList.empty())
        {
            assignMaterial(material, _geometryList[_selectedGeom]);
        }

        // Update property editor for material
        updatePropertyEditor();
        return material;
    }
    return nullptr;
}

void Viewer::reloadDocument()
{
    try
    {
        if (!_materialFilename.isEmpty())
        {
            initializeDocument(_stdLib);
            size_t newRenderables = Material::loadDocument(_doc, _searchPath.find(_materialFilename), _stdLib, _modifiers, _materials);
            if (newRenderables)
            {
                updateMaterialSelections();
                setMaterialSelection(0);
                if (!_materials.empty())
                {
                    assignMaterial(_materials[0]);
                }
            }
        }
    }
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Failed to load document", e.what());
    }
    try
    {
        updateMaterialSelections();
        setMaterialSelection(_selectedMaterial);
        updatePropertyEditor();
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
                assignMaterial(material, _geometryList[_selectedGeom]);
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

                mx::NodeDefPtr nodeDef = shaderRef ? shaderRef->getNodeDef() : nullptr;
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
    catch (std::exception& e)
    {
        new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Cannot save dot file for material", e.what());
    }
}

bool Viewer::keyboardEvent(int key, int scancode, int action, int modifiers)
{
    if (Screen::keyboardEvent(key, scancode, action, modifiers))
    {
        return true;
    }

    // Reload the current document from file.
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        reloadDocument();
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
            for (auto extension : extensions)
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
            try
            {
                setMaterialSelection(_selectedMaterial);
            }
            catch (std::exception& e)
            {
                new ng::MessageDialog(this, ng::MessageDialog::Type::Warning, "Cannot assign material.", e.what());
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

    mx::Matrix44 world, view, proj;
    computeCameraMatrices(world, view, proj);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glEnable(GL_FRAMEBUFFER_SRGB);

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
            _envMaterial->bindImages(_imageHandler, _searchPath, _envMaterial->getUdim());
            _envMaterial->drawPartition(envPart);
            glDisable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
    }

    mx::TypedElementPtr lastBoundShader;
    for (auto assignment : _materialAssignments)
    {
        mx::MeshPartitionPtr geom = assignment.first;
        MaterialPtr material = assignment.second;
        mx::TypedElementPtr shader = material->getElement();

        material->bindShader();
        if (material->hasTransparency())
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
        {
            glDisable(GL_BLEND);
        }
        material->bindViewInformation(world, view, proj);
        material->bindLights(_lightHandler, _imageHandler, _searchPath, _directLighting, _indirectLighting,
                             _specularEnvironmentMethod, _envSamples);
        material->bindImages(_imageHandler, _searchPath, material->getUdim());
        material->drawPartition(geom);
    }

    glDisable(GL_BLEND);
    glDisable(GL_FRAMEBUFFER_SRGB);

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

        // Must multipy by pixel ratio to handle device DPI
        int w = mSize.x() * static_cast<int>(mPixelRatio);
        int h = mSize.y() * static_cast<int>(mPixelRatio);
        size_t bufferSize = w * h * 3;
        std::vector<uint8_t> buffer(bufferSize);
        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());

        mx::ImageDesc desc;
        desc.width = w;
        desc.height = h;
        desc.channelCount = 3;
        desc.resourceBuffer = buffer.data();
        desc.baseType = mx::ImageDesc::BASETYPE_UINT8;

        bool saved = _imageHandler->saveImage(_captureFrameFileName, desc, true);
        desc.resourceBuffer = nullptr;
        if (!saved)
        {
            new ng::MessageDialog(this, ng::MessageDialog::Type::Information,
                "Failed to save frame to disk: ", _captureFrameFileName.asString());
        }
    }
}

bool Viewer::scrollEvent(const ng::Vector2i& p, const ng::Vector2f& rel)
{
    if (!Screen::scrollEvent(p, rel))
    {
        _zoom = std::max(0.1f, _zoom * ((rel.y() > 0) ? 1.1f : 0.9f));
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
    if (!Screen::mouseButtonEvent(p, button, down, modifiers))
    {
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

void Viewer::updatePropertyEditor()
{
    _propertyEditor.updateContents(this);
}
