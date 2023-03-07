//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXView/Viewer.h>

#include <MaterialXRenderGlsl/GLFramebuffer.h>
#include <MaterialXRenderGlsl/GlslMaterial.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <MaterialXRenderGlsl/GLUtil.h>
#include <MaterialXRenderGlsl/TextureBaker.h>

#include <MaterialXRender/CgltfLoader.h>
#include <MaterialXRender/Harmonics.h>
#include <MaterialXRender/OiioImageLoader.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/TinyObjLoader.h>

#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/ShaderTranslator.h>

#if MATERIALX_BUILD_GEN_MDL
#include <MaterialXGenMdl/MdlShaderGenerator.h>
#endif
#if MATERIALX_BUILD_GEN_OSL
#include <MaterialXGenOsl/OslShaderGenerator.h>
#endif
#include <MaterialXGenGlsl/EsslShaderGenerator.h>

#include <MaterialXFormat/Environ.h>
#include <MaterialXFormat/Util.h>

#include <nanogui/icons.h>
#include <nanogui/messagedialog.h>
#include <nanogui/opengl.h>
#include <nanogui/vscrollpanel.h>

#include <fstream>
#include <iostream>
#include <iomanip>

const mx::Vector3 DEFAULT_CAMERA_POSITION(0.0f, 0.0f, 5.0f);
const float DEFAULT_CAMERA_VIEW_ANGLE = 45.0f;
const float DEFAULT_CAMERA_ZOOM = 1.0f;

namespace
{

const int SHADOW_MAP_SIZE = 2048;
const int ALBEDO_TABLE_SIZE = 128;

const float ORTHO_VIEW_DISTANCE = 1000.0f;
const float ORTHO_PROJECTION_HEIGHT = 1.8f;

const std::string DIR_LIGHT_NODE_CATEGORY = "directional_light";
const std::string IRRADIANCE_MAP_FOLDER = "irradiance";

const float PI = std::acos(-1.0f);

// ViewDir implementation for GLSL
// as needed for the environment shader.
class ViewDirGlsl : public mx::GlslImplementation
{
public:
    static  mx::ShaderNodeImplPtr create()
    {
        return std::make_shared<ViewDirGlsl>();
    }

    void createVariables(const  mx::ShaderNode&, mx::GenContext&, mx::Shader& shader) const override
    {
        mx::ShaderStage& vs = shader.getStage(mx::Stage::VERTEX);
        mx::ShaderStage& ps = shader.getStage(mx::Stage::PIXEL);
        addStageInput(mx::HW::VERTEX_INPUTS, mx::Type::VECTOR3, mx::HW::T_IN_POSITION, vs);
        addStageConnector(mx::HW::VERTEX_DATA, mx::Type::VECTOR3, mx::HW::T_POSITION_WORLD, vs, ps);
        addStageUniform(mx::HW::PRIVATE_UNIFORMS, mx::Type::VECTOR3, mx::HW::T_VIEW_POSITION, ps);
    }

    void emitFunctionCall(const  mx::ShaderNode& node, mx::GenContext& context, mx::ShaderStage& stage) const override
    {
        const mx::ShaderGenerator& shadergen = context.getShaderGenerator();

        DEFINE_SHADER_STAGE(stage, mx::Stage::VERTEX)
        {
            mx::VariableBlock& vertexData = stage.getOutputBlock(mx::HW::VERTEX_DATA);
            const mx::string prefix = vertexData.getInstance() + ".";
            mx::ShaderPort* position = vertexData[mx::HW::T_POSITION_WORLD];
            if (!position->isEmitted())
            {
                position->setEmitted();
                shadergen.emitLine(prefix + position->getVariable() + " = hPositionWorld.xyz", stage);
            }
        }

        DEFINE_SHADER_STAGE(stage, mx::Stage::PIXEL)
        {
            mx::VariableBlock& vertexData = stage.getInputBlock(mx::HW::VERTEX_DATA);
            const mx::string prefix = vertexData.getInstance() + ".";
            mx::ShaderPort* position = vertexData[mx::HW::T_POSITION_WORLD];
            shadergen.emitLineBegin(stage);
            shadergen.emitOutput(node.getOutput(), true, false, context, stage);
            shadergen.emitString(" = normalize(" + prefix + position->getVariable() + " - " + mx::HW::T_VIEW_POSITION + ")", stage);
            shadergen.emitLineEnd(stage);
        }
    }
};

} // anonymous namespace

//
// Viewer methods
//

Viewer::Viewer(const std::string& materialFilename,
               const std::string& meshFilename,
               const std::string& envRadianceFilename,
               const mx::FileSearchPath& searchPath,
               const mx::FilePathVec& libraryFolders,
               int screenWidth,
               int screenHeight,
               const mx::Color3& screenColor) :
    ng::Screen(ng::Vector2i(screenWidth, screenHeight), "MaterialXView",
        true, false, true, true, false, 4, 0),
    _window(nullptr),
    _materialFilename(materialFilename),
    _meshFilename(meshFilename),
    _envRadianceFilename(envRadianceFilename),
    _searchPath(searchPath),
    _libraryFolders(libraryFolders),
    _meshScale(1.0f),
    _turntableEnabled(false),
    _turntableSteps(360),
    _turntableStep(0),
    _cameraPosition(DEFAULT_CAMERA_POSITION),
    _cameraUp(0.0f, 1.0f, 0.0f),
    _cameraViewAngle(DEFAULT_CAMERA_VIEW_ANGLE),
    _cameraNearDist(0.05f),
    _cameraFarDist(5000.0f),
    _cameraZoom(DEFAULT_CAMERA_ZOOM),
    _userCameraEnabled(true),
    _userTranslationActive(false),
    _lightRotation(0.0f),
    _normalizeEnvironment(false),
    _splitDirectLight(false),
    _generateReferenceIrradiance(false),
    _saveGeneratedLights(false),
    _shadowSoftness(1),
    _ambientOcclusionGain(0.6f),
    _selectedGeom(0),
    _geomLabel(nullptr),
    _geometrySelectionBox(nullptr),
    _selectedMaterial(0),
    _materialLabel(nullptr),
    _materialSelectionBox(nullptr),
    _viewCamera(mx::Camera::create()),
    _envCamera(mx::Camera::create()),
    _shadowCamera(mx::Camera::create()),
    _lightHandler(mx::LightHandler::create()),
    _genContext(mx::GlslShaderGenerator::create()),
    _genContextEssl(mx::EsslShaderGenerator::create()),
#if MATERIALX_BUILD_GEN_OSL
    _genContextOsl(mx::OslShaderGenerator::create()),
#endif
#if MATERIALX_BUILD_GEN_MDL
    _genContextMdl(mx::MdlShaderGenerator::create()),
#endif
    _unitRegistry(mx::UnitConverterRegistry::create()),
    _drawEnvironment(false),
    _outlineSelection(false),
    _renderTransparency(true),
    _renderDoubleSided(true),
    _splitByUdims(true),
    _mergeMaterials(false),
    _showAllInputs(false),
    _flattenSubgraphs(false),
    _targetShader("standard_surface"),
    _captureRequested(false),
    _exitRequested(false),
    _wedgeRequested(false),
    _wedgePropertyName("base"),
    _wedgePropertyMin(0.0f),
    _wedgePropertyMax(1.0f),
    _wedgeImageCount(8),
    _bakeHdr(false),
    _bakeAverage(false),
    _bakeOptimize(true),
    _bakeRequested(false),
    _bakeWidth(0),
    _bakeHeight(0)
{
    // Resolve input filenames, taking both the provided search path and
    // current working directory into account.
    mx::FileSearchPath localSearchPath = searchPath;
    localSearchPath.append(mx::FilePath::getCurrentPath());
    _materialFilename = localSearchPath.find(_materialFilename);
    _meshFilename = localSearchPath.find(_meshFilename);
    _envRadianceFilename = localSearchPath.find(_envRadianceFilename);

    // Set the requested background color.
    set_background(ng::Color(screenColor[0], screenColor[1], screenColor[2], 1.0f));

    // Set default Glsl generator options.
    _genContext.getOptions().targetColorSpaceOverride = "lin_rec709";
    _genContext.getOptions().fileTextureVerticalFlip = true;
    _genContext.getOptions().hwShadowMap = true;
    _genContext.getOptions().hwImplicitBitangents = false;

    // Set Essl generator options
    _genContextEssl.getOptions().targetColorSpaceOverride = "lin_rec709";
    _genContextEssl.getOptions().fileTextureVerticalFlip = false;
    _genContextEssl.getOptions().hwMaxActiveLightSources = 1;

#if MATERIALX_BUILD_GEN_OSL
    // Set OSL generator options.
    _genContextOsl.getOptions().targetColorSpaceOverride = "lin_rec709";
    _genContextOsl.getOptions().fileTextureVerticalFlip = false;
#endif
#if MATERIALX_BUILD_GEN_MDL
    // Set MDL generator options.
    _genContextMdl.getOptions().targetColorSpaceOverride = "lin_rec709";
    _genContextMdl.getOptions().fileTextureVerticalFlip = false;
#endif

    // Register the GLSL implementation for <viewdir> used by the environment shader.
    _genContext.getShaderGenerator().registerImplementation("IM_viewdir_vector3_" + mx::GlslShaderGenerator::TARGET, ViewDirGlsl::create);
}

void Viewer::initFramebuffer()
{
}

void Viewer::resizeFramebuffer()
{
}

void Viewer::initImageHandler()
{
    _imageHandler = mx::GLTextureHandler::create(
        mx::StbImageLoader::create());
}

void Viewer::renderFrame()
{
    if (_geometryList.empty() || _materialAssignments.empty())
    {
        return;
    }

    // Initialize OpenGL state
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_FRAMEBUFFER_SRGB);

    // Update lighting state.
    _lightHandler->setLightTransform(mx::Matrix44::createRotationY(_lightRotation / 180.0f * PI));

    // Update shadow state.
    mx::ShadowState shadowState;
    shadowState.ambientOcclusionGain = _ambientOcclusionGain;
    mx::NodePtr dirLight = _lightHandler->getFirstLightOfCategory(DIR_LIGHT_NODE_CATEGORY);
    if (_genContext.getOptions().hwShadowMap && dirLight)
    {
        mx::ImagePtr shadowMap = getShadowMap();
        if (shadowMap)
        {
            shadowState.shadowMap = shadowMap;
            shadowState.shadowMatrix = _viewCamera->getWorldMatrix().getInverse() *
                _shadowCamera->getWorldViewProjMatrix();
        }
        else
        {
            _genContext.getOptions().hwShadowMap = false;
        }
    }

    glEnable(GL_FRAMEBUFFER_SRGB);

    // Environment background
    if (_drawEnvironment)
    {
        mx::MaterialPtr envMaterial = getEnvironmentMaterial();
        if (envMaterial)
        {
            const mx::MeshList& meshes = _envGeometryHandler->getMeshes();
            mx::MeshPartitionPtr envPart = !meshes.empty() ? meshes[0]->getPartition(0) : nullptr;
            if (envPart)
            {
                // Apply rotation to the environment shader.
                float longitudeOffset = (_lightRotation / 360.0f) + 0.5f;
                _envMaterial->modifyUniform("longitude/in2", mx::Value::createValue(longitudeOffset));

                // Render the environment mesh.
                glDepthMask(GL_FALSE);
                envMaterial->bindShader();
                envMaterial->bindMesh(meshes[0]);
                envMaterial->bindViewInformation(_envCamera);
                envMaterial->bindImages(_imageHandler, _searchPath, false);
                envMaterial->drawPartition(envPart);
                glDepthMask(GL_TRUE);
            }
        }
        else
        {
            _drawEnvironment = false;
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
        mx::GlslMaterialPtr material = std::static_pointer_cast<mx::GlslMaterial>(assignment.second);
        shadowState.ambientOcclusionMap = getAmbientOcclusionImage(material);
        if (!material)
        {
            continue;
        }

        material->bindShader();
        material->bindMesh(_geometryHandler->findParentMesh(geom));
        if (material->getProgram()->hasUniform(mx::HW::ALPHA_THRESHOLD))
        {
            material->getProgram()->bindUniform(mx::HW::ALPHA_THRESHOLD, mx::Value::createValue(0.99f));
        }
        material->bindViewInformation(_viewCamera);
        material->bindLighting(_lightHandler, _imageHandler, shadowState);
        material->bindImages(_imageHandler, _searchPath);
        material->drawPartition(geom);
        material->unbindImages(_imageHandler);
    }

    // Transparent pass
    if (_renderTransparency)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for (const auto& assignment : _materialAssignments)
        {
            mx::MeshPartitionPtr geom = assignment.first;
            mx::GlslMaterialPtr material = std::static_pointer_cast<mx::GlslMaterial>(assignment.second);
            shadowState.ambientOcclusionMap = getAmbientOcclusionImage(material);
            if (!material || !material->hasTransparency())
            {
                continue;
            }

            material->bindShader();
            material->bindMesh(_geometryHandler->findParentMesh(geom));
            if (material->getProgram()->hasUniform(mx::HW::ALPHA_THRESHOLD))
            {
                material->getProgram()->bindUniform(mx::HW::ALPHA_THRESHOLD, mx::Value::createValue(0.001f));
            }
            material->bindViewInformation(_viewCamera);
            material->bindLighting(_lightHandler, _imageHandler, shadowState);
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
        mx::MaterialPtr wireMaterial = getWireframeMaterial();
        if (wireMaterial)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            wireMaterial->bindShader();
            wireMaterial->bindMesh(_geometryHandler->findParentMesh(getSelectedGeometry()));
            wireMaterial->bindViewInformation(_viewCamera);
            wireMaterial->drawPartition(getSelectedGeometry());
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else
        {
            _outlineSelection = false;
        }
    }
}

void Viewer::bakeTextures()
{
    mx::MaterialPtr material = getSelectedMaterial();
    mx::DocumentPtr doc = material ? material->getDocument() : nullptr;
    if (!doc)
    {
        return;
    }

    {
        // Compute baking resolution.
        mx::ImageVec imageVec = _imageHandler->getReferencedImages(doc);
        auto maxImageSize = mx::getMaxDimensions(imageVec);
        unsigned int bakeWidth = std::max(maxImageSize.first, (unsigned int) 4);
        unsigned int bakeHeight = std::max(maxImageSize.second, (unsigned int) 4);
        if (_bakeWidth)
        {
            bakeWidth = std::max(_bakeWidth, (unsigned int) 4);
        }
        if (_bakeHeight)
        {
            bakeHeight = std::max(_bakeHeight, (unsigned int) 4);
        }

        // Construct a texture baker.
        mx::Image::BaseType baseType = _bakeHdr ? mx::Image::BaseType::FLOAT : mx::Image::BaseType::UINT8;
        mx::TextureBakerPtr baker = mx::TextureBakerGlsl::create(bakeWidth, bakeHeight, baseType);
        baker->setupUnitSystem(_stdLib);
        baker->setDistanceUnit(_genContext.getOptions().targetDistanceUnit);
        baker->setAverageImages(_bakeAverage);
        baker->setOptimizeConstants(_bakeOptimize);

        // Assign our existing image handler, releasing any existing render resources for cached images.
        _imageHandler->releaseRenderResources();
        baker->setImageHandler(_imageHandler);

        // Extend the image search path to include material source folders.
        mx::FileSearchPath extendedSearchPath = _searchPath;
        extendedSearchPath.append(_materialSearchPath);

        // Bake all materials in the active document.
        try
        {
            baker->bakeAllMaterials(doc, extendedSearchPath, _bakeFilename);
        }
        catch (std::exception& e)
        {
            std::cerr << "Error in texture baking: " << e.what() << std::endl;
        }

        // Release any render resources generated by the baking process.
        _imageHandler->releaseRenderResources();
    }

    // After the baker has been destructed, restore state for scene rendering.
    glfwMakeContextCurrent(m_glfw_window);
    glfwGetFramebufferSize(m_glfw_window, &m_fbsize[0], &m_fbsize[1]);
    glViewport(0, 0, m_fbsize[0], m_fbsize[1]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
}


void Viewer::updateCameras()
{
    mx::Matrix44 viewMatrix, projectionMatrix;
    float aspectRatio = (float) m_size.x() / (float) m_size.y();
    if (_cameraViewAngle != 0.0f)
    {
        viewMatrix = mx::Camera::createViewMatrix(_cameraPosition, _cameraTarget, _cameraUp);
        float fH = std::tan(_cameraViewAngle / 360.0f * PI) * _cameraNearDist;
        float fW = fH * aspectRatio;
        projectionMatrix = mx::Camera::createPerspectiveMatrix(-fW, fW, -fH, fH, _cameraNearDist, _cameraFarDist);
    }
    else
    {
        viewMatrix = mx::Matrix44::createTranslation(mx::Vector3(0.0f, 0.0f, -ORTHO_VIEW_DISTANCE));
        float fH = ORTHO_PROJECTION_HEIGHT;
        float fW = fH * aspectRatio;
        projectionMatrix = mx::Camera::createOrthographicMatrix(-fW, fW, -fH, fH, 0.0f, ORTHO_VIEW_DISTANCE + _cameraFarDist);
    }

    float turntableRotation = fmod((360.0f / _turntableSteps) * _turntableStep, 360.0f);
    float yRotation = _meshRotation[1] + (_turntableEnabled ? turntableRotation : 0.0f);
    mx::Matrix44 meshRotation = mx::Matrix44::createRotationZ(_meshRotation[2] / 180.0f * PI) *
                                mx::Matrix44::createRotationY(yRotation / 180.0f * PI) *
                                mx::Matrix44::createRotationX(_meshRotation[0] / 180.0f * PI);

    mx::Matrix44 arcball = mx::Matrix44::IDENTITY;
    if (_userCameraEnabled)
    {
        arcball = _viewCamera->arcballMatrix();
    }

    _viewCamera->setWorldMatrix(meshRotation *
                                mx::Matrix44::createTranslation(_meshTranslation + _userTranslation) *
                                mx::Matrix44::createScale(mx::Vector3(_meshScale * _cameraZoom)));
    _viewCamera->setViewMatrix(arcball * viewMatrix);
    _viewCamera->setProjectionMatrix(projectionMatrix);

    _envCamera->setWorldMatrix(mx::Matrix44::createScale(mx::Vector3(300.0f)));
    _envCamera->setViewMatrix(_viewCamera->getViewMatrix());
    _envCamera->setProjectionMatrix(_viewCamera->getProjectionMatrix());

    mx::NodePtr dirLight = _lightHandler->getFirstLightOfCategory(DIR_LIGHT_NODE_CATEGORY);
    if (dirLight)
    {
        mx::Vector3 sphereCenter = (_geometryHandler->getMaximumBounds() + _geometryHandler->getMinimumBounds()) * 0.5;
        float r = (sphereCenter - _geometryHandler->getMinimumBounds()).getMagnitude();
        _shadowCamera->setWorldMatrix(meshRotation * mx::Matrix44::createTranslation(-sphereCenter));
        _shadowCamera->setProjectionMatrix(mx::Camera::createOrthographicMatrix(-r, r, -r, r, 0.0f, r * 2.0f));
        mx::ValuePtr value = dirLight->getInputValue("direction");
        if (value->isA<mx::Vector3>())
        {
            mx::Vector3 dir = mx::Matrix44::createRotationY(_lightRotation / 180.0f * PI).transformVector(value->asA<mx::Vector3>());
            _shadowCamera->setViewMatrix(mx::Camera::createViewMatrix(dir * -r, mx::Vector3(0.0f), _cameraUp));
        }
    }
}

mx::MaterialPtr Viewer::getEnvironmentMaterial()
{
    if (!_envMaterial)
    {
        mx::FilePath envFilename = _searchPath.find(mx::FilePath("resources/Lights/envmap_shader.mtlx"));
        try
        {
            _envMaterial = mx::GlslMaterial::create();
            _envMaterial->generateEnvironmentShader(_genContext, envFilename, _stdLib, _envRadianceFilename);
        }
        catch (std::exception& e)
        {
            std::cerr << "Failed to generate environment shader: " << e.what() << std::endl;
            _envMaterial = nullptr;
        }
    }

    return _envMaterial;
}

mx::MaterialPtr Viewer::getWireframeMaterial()
{
    if (!_wireMaterial)
    {
        try
        {
            mx::ShaderPtr hwShader = mx::createConstantShader(_genContext, _stdLib, "__WIRE_SHADER__", mx::Color3(1.0f));
            _wireMaterial = mx::GlslMaterial::create();
            _wireMaterial->generateShader(hwShader);
        }
        catch (std::exception& e)
        {
            std::cerr << "Failed to generate wireframe shader: " << e.what() << std::endl;
            _wireMaterial = nullptr;
        }
    }

    return _wireMaterial;
}


mx::ImagePtr Viewer::getShadowMap()
{
    if (!_shadowMap)
    {
        // Generate shaders for shadow rendering.
        if (!_shadowMaterial)
        {
            try
            {
                mx::ShaderPtr hwShader = mx::createDepthShader(_genContext, _stdLib, "__SHADOW_SHADER__");
                _shadowMaterial = mx::GlslMaterial::create();
                _shadowMaterial->generateShader(hwShader);
            }
            catch (std::exception& e)
            {
                std::cerr << "Failed to generate shadow shader: " << e.what() << std::endl;
                _shadowMaterial = nullptr;
            }
        }
        if (!_shadowBlurMaterial)
        {
            try
            {
                mx::ShaderPtr hwShader = mx::createBlurShader(_genContext, _stdLib, "__SHADOW_BLUR_SHADER__", "gaussian", 1.0f);
                _shadowBlurMaterial = mx::GlslMaterial::create();
                _shadowBlurMaterial->generateShader(hwShader);
            }
            catch (std::exception& e)
            {
                std::cerr << "Failed to generate shadow blur shader: " << e.what() << std::endl;
                _shadowBlurMaterial = nullptr;
            }
        }

        if (_shadowMaterial && _shadowBlurMaterial)
        {
            // Create framebuffer.
            mx::GLFramebufferPtr framebuffer = mx::GLFramebuffer::create(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 2, mx::Image::BaseType::FLOAT);
            framebuffer->bind();
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            // Render shadow geometry.
            _shadowMaterial->bindShader();
            for (auto mesh : _geometryHandler->getMeshes())
            {
                _shadowMaterial->bindMesh(mesh);
                _shadowMaterial->bindViewInformation(_shadowCamera);
                for (size_t i = 0; i < mesh->getPartitionCount(); i++)
                {
                    mx::MeshPartitionPtr geom = mesh->getPartition(i);
                    _shadowMaterial->drawPartition(geom);
                }
            }
            _shadowMap = framebuffer->getColorImage();

            // Apply Gaussian blurring.
            mx::ImageSamplingProperties blurSamplingProperties;
            blurSamplingProperties.uaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
            blurSamplingProperties.vaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
            blurSamplingProperties.filterType = mx::ImageSamplingProperties::FilterType::CLOSEST;
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
                        std::static_pointer_cast<mx::GlslMaterial>(_shadowBlurMaterial)
                            ->getProgram()->bindUniform("image_file",
                                                        mx::Value::createValue(textureLocation));
                    }
                }
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                renderScreenSpaceQuad(_shadowBlurMaterial);
                _imageHandler->releaseRenderResources(_shadowMap);
                _shadowMap = framebuffer->getColorImage();
            }

            // Restore state for scene rendering.
            glViewport(0, 0, m_fbsize[0], m_fbsize[1]);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glDrawBuffer(GL_BACK);
        }
    }

    return _shadowMap;
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
    mx::GLFramebufferPtr framebuffer = mx::GLFramebuffer::create(ALBEDO_TABLE_SIZE, ALBEDO_TABLE_SIZE, 3, mx::Image::BaseType::FLOAT);
    framebuffer->bind();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Create shader.
    mx::ShaderPtr hwShader = mx::createAlbedoTableShader(_genContext, _stdLib, "__ALBEDO_TABLE_SHADER__");
    mx::GlslMaterialPtr material = mx::GlslMaterial::create();
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
    if (material->getProgram()->hasUniform(mx::HW::ALBEDO_TABLE_SIZE))
    {
        material->getProgram()->bindUniform(mx::HW::ALBEDO_TABLE_SIZE, mx::Value::createValue(ALBEDO_TABLE_SIZE));
    }
    renderScreenSpaceQuad(material);

    // Store albedo table image.
    _imageHandler->releaseRenderResources(_lightHandler->getAlbedoTable());
    _lightHandler->setAlbedoTable(framebuffer->getColorImage());
    if (_saveGeneratedLights)
    {
        _imageHandler->saveImage("AlbedoTable.exr", _lightHandler->getAlbedoTable());
    }

    // Restore state for scene rendering.
    glViewport(0, 0, m_fbsize[0], m_fbsize[1]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
}

void Viewer::renderScreenSpaceQuad(mx::MaterialPtr material)
{
    if (!_quadMesh)
        _quadMesh = mx::GeometryHandler::createQuadMesh();
    
    material->bindMesh(_quadMesh);
    material->drawPartition(_quadMesh->getPartition(0));
}

void Viewer::toggleTurntable(bool enable)
{
    _turntableEnabled = enable;

    if (enable)
    {
        _turntableTimer.startTimer();
    }
    else
    {
        float turntableRotation = fmod((360.0f / _turntableSteps) * _turntableStep, 360.0f);
        _meshRotation[1] = fmod(_meshRotation[1] + turntableRotation, 360.0f);
        _turntableTimer.endTimer();
    }
    invalidateShadowMap();
    _turntableStep = 0;
}
