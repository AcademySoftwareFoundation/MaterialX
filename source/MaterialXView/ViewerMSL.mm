//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXView/Viewer.h>

#include <MaterialXGenMsl/MslShaderGenerator.h>
#include <MaterialXRenderMsl/TextureBaker.h>
#include <MaterialXRenderMsl/MetalFramebuffer.h>

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

#include <GLFW/glfw3.h>
#include <nanogui/icons.h>
#include <nanogui/messagedialog.h>
#include <nanogui/metal.h>
#include <nanogui/vscrollpanel.h>

#include <MaterialXRenderMsl/MetalState.h>
#include <MaterialXRenderMsl/MslMaterial.h>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>

const mx::Vector3 DEFAULT_CAMERA_POSITION(0.0f, 0.0f, 5.0f);
const float DEFAULT_CAMERA_VIEW_ANGLE = 45.0f;
const float DEFAULT_CAMERA_ZOOM = 1.0f;

using namespace mx;

namespace {

const int ALBEDO_TABLE_SIZE = 128;
const int SHADOW_MAP_SIZE = 2048;
const std::string DIR_LIGHT_NODE_CATEGORY = "directional_light";
const std::string IRRADIANCE_MAP_FOLDER = "irradiance";
const float ORTHO_VIEW_DISTANCE = 1000.0f;
const float ORTHO_PROJECTION_HEIGHT = 1.8f;

// ViewDir implementation for MSL
// as needed for the environment shader.
class ViewDirMsl : public mx::MslImplementation
{
public:
    static  mx::ShaderNodeImplPtr create()
    {
        return std::make_shared<ViewDirMsl>();
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
        
        BEGIN_SHADER_STAGE(stage, mx::Stage::VERTEX)
        mx::VariableBlock& vertexData = stage.getOutputBlock(mx::HW::VERTEX_DATA);
        const mx::string prefix = vertexData.getInstance() + ".";
        mx::ShaderPort* position = vertexData[mx::HW::T_POSITION_WORLD];
        if (!position->isEmitted())
        {
            position->setEmitted();
            shadergen.emitLine(prefix + position->getVariable() + " = hPositionWorld.xyz", stage);
        }
        END_SHADER_STAGE(stage, mx::Stage::VERTEX)
        
        BEGIN_SHADER_STAGE(stage, mx::Stage::PIXEL)
        mx::VariableBlock& vertexData = stage.getInputBlock(mx::HW::VERTEX_DATA);
        const mx::string prefix = vertexData.getInstance() + ".";
        mx::ShaderPort* position = vertexData[mx::HW::T_POSITION_WORLD];
        shadergen.emitLineBegin(stage);
        shadergen.emitOutput(node.getOutput(), true, false, context, stage);
        shadergen.emitString(" = normalize(" + prefix + position->getVariable() + " - " + mx::HW::T_VIEW_POSITION + ")", stage);
        shadergen.emitLineEnd(stage);
        END_SHADER_STAGE(stage, mx::Stage::PIXEL)
    }
};
}

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
    _identityCamera(mx::Camera::create()),
    _viewCamera(mx::Camera::create()),
    _envCamera(mx::Camera::create()),
    _shadowCamera(mx::Camera::create()),
    _lightHandler(mx::LightHandler::create()),
    _genContext(mx::MslShaderGenerator::create()),
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
    
    MTL(initialize((id<MTLDevice>)ng::metal_device(),
                   (id<MTLCommandQueue>)ng::metal_command_queue()));

    // Set the requested background color.
    set_background(ng::Color(screenColor[0], screenColor[1], screenColor[2], 1.0f));

    // Set default Glsl generator options.
    _genContext.getOptions().targetColorSpaceOverride = "lin_rec709";
    _genContext.getOptions().fileTextureVerticalFlip = true;
    _genContext.getOptions().hwShadowMap = true;
    
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

    // Register the MSL implementation for <viewdir> used by the environment shader.
    _genContext.getShaderGenerator().registerImplementation("IM_viewdir_vector3_" + mx::MslShaderGenerator::TARGET, ViewDirMsl::create);
}

void Viewer::initFramebuffer()
{
    
    MTL_PUSH_FRAMEBUFFER(mx::MetalFramebuffer::create(
                            MTL(device),
                            width() * 2,
                            height() * 2,
                            4, mx::Image::BaseType::UINT8,
                            MTL(supportsTiledPipeline) ?
                            (id<MTLTexture>)metal_texture() : nil,
                            false,  MTLPixelFormatBGRA8Unorm));
}

void Viewer::resizeFramebuffer()
{
    MTL_POP_FRAMEBUFFER();
    initFramebuffer();
}

void Viewer::initImageHandler()
{
    _imageHandler = mx::MetalTextureHandler::create(
        MTL(device),
        mx::StbImageLoader::create());
}

void Viewer::renderFrame()
{
    // Update lighting state.
    _lightHandler->setLightTransform(mx::Matrix44::createRotationY(_lightRotation / 180.0f * M_PI));

    // Update shadow state.
    ShadowState shadowState;
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
    
    bool captureFrame = false;
    if(captureFrame)
        MTL_TRIGGER_CAPTURE;
    
    MTL(beginCommandBuffer());
    MTLRenderPassDescriptor* renderpassDesc = [MTLRenderPassDescriptor new];
    [renderpassDesc.colorAttachments[0] setTexture:
        MTL(supportsTiledPipeline) ?    (id <MTLTexture>)metal_texture() :
                                        MTL(currentFramebuffer())->getColorTexture()];
    [renderpassDesc.colorAttachments[0] setClearColor:MTLClearColorMake(m_background[0],
                                        m_background[1],
                                        m_background[2],
                                        m_background[3])];
    [renderpassDesc.colorAttachments[0] setLoadAction:MTLLoadActionClear];
    [renderpassDesc.colorAttachments[0] setStoreAction:MTLStoreActionStore];
    
    [renderpassDesc.depthAttachment setTexture:MTL(currentFramebuffer())->getDepthTexture()];
    [renderpassDesc.depthAttachment setClearDepth:1.0];
    [renderpassDesc.depthAttachment setLoadAction:MTLLoadActionClear];
    [renderpassDesc.depthAttachment setStoreAction:MTLStoreActionStore];
    [renderpassDesc setStencilAttachment:nil];
        
    MTL(beginEncoder(renderpassDesc));
        
    [MTL(renderCmdEncoder) setFrontFacingWinding:MTLWindingClockwise];

    // Environment background
    if (_drawEnvironment)
    {
        [MTL(renderCmdEncoder) setDepthStencilState:MTL_DEPTHSTENCIL_STATE(envMap)];
        MaterialPtr envMaterial = getEnvironmentMaterial();
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
                [MTL(renderCmdEncoder) setCullMode:MTLCullModeNone];
                envMaterial->bindShader();
                envMaterial->bindMesh(meshes[0]);
                envMaterial->bindViewInformation(_envCamera);
                envMaterial->bindImages(_imageHandler, _searchPath, false);
                std::static_pointer_cast<MslMaterial>
                (envMaterial)->prepareUsedResources(_envCamera,
                                        _envGeometryHandler,
                                        _imageHandler,
                                        _lightHandler);
                envMaterial->drawPartition(envPart);
                [MTL(renderCmdEncoder) setCullMode:MTLCullModeNone];
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
        [MTL(renderCmdEncoder) setCullMode:MTLCullModeBack];
    }

    // Opaque pass
    [MTL(renderCmdEncoder) setDepthStencilState:MTL_DEPTHSTENCIL_STATE(opaque)];
    for (const auto& assignment : _materialAssignments)
    {
        mx::MeshPartitionPtr geom = assignment.first;
        MslMaterialPtr material = std::static_pointer_cast<MslMaterial>(assignment.second);
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
        material->prepareUsedResources(_viewCamera,
                             _geometryHandler,
                             _imageHandler,
                             _lightHandler);
        material->drawPartition(geom);
        material->unbindImages(_imageHandler);
    }

    // Transparent pass
    if (_renderTransparency)
    {
        [MTL(renderCmdEncoder) setDepthStencilState:MTL_DEPTHSTENCIL_STATE(transparent)];
        for (const auto& assignment : _materialAssignments)
        {
            mx::MeshPartitionPtr geom = assignment.first;
            MslMaterialPtr material = std::static_pointer_cast<MslMaterial>(assignment.second);
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
            material->prepareUsedResources(_viewCamera,
                                 _geometryHandler,
                                 _imageHandler,
                                 _lightHandler);
            material->drawPartition(geom);
            material->unbindImages(_imageHandler);
        }
    }

    if (!_renderDoubleSided)
    {
        [MTL(renderCmdEncoder) setCullMode:MTLCullModeNone];
    }

    // Wireframe pass
    if (_outlineSelection)
    {
        MaterialPtr wireMaterial = getWireframeMaterial();
        if (wireMaterial)
        {
            [MTL(renderCmdEncoder) setCullMode:MTLCullModeNone];
            [MTL(renderCmdEncoder) setTriangleFillMode:MTLTriangleFillModeLines];
            wireMaterial->bindShader();
            wireMaterial->bindMesh(_geometryHandler->findParentMesh(getSelectedGeometry()));
            wireMaterial->bindViewInformation(_viewCamera);
            std::static_pointer_cast<MslMaterial>
            (wireMaterial)->prepareUsedResources(_viewCamera,
                                 _geometryHandler,
                                 _imageHandler,
                                 _lightHandler);
            wireMaterial->drawPartition(getSelectedGeometry());
            [MTL(renderCmdEncoder) setTriangleFillMode:MTLTriangleFillModeFill];
            [MTL(renderCmdEncoder) setCullMode:MTLCullModeNone];
        }
        else
        {
            _outlineSelection = false;
        }
    }
    
#if MAC_OS_VERSION_11_0
    if(MTL(supportsTiledPipeline))
    {
        [MTL(renderCmdEncoder) setRenderPipelineState:MTL(linearToSRGB_pso)];
        [MTL(renderCmdEncoder) dispatchThreadsPerTile:MTLSizeMake(
                                    MTL(renderCmdEncoder).tileWidth,
                                    MTL(renderCmdEncoder).tileHeight, 1)];
    }
    else
#endif
    {
        MTL(endEncoder());
        [renderpassDesc.colorAttachments[0]
            setTexture:(id<MTLTexture>)metal_texture()];
        MTL(beginEncoder(renderpassDesc));
        [MTL(renderCmdEncoder) setRenderPipelineState:MTL(linearToSRGB_pso)];
        [MTL(renderCmdEncoder)
            setFragmentTexture:MTL(currentFramebuffer())->getColorTexture()
            atIndex:0];
        [MTL(renderCmdEncoder) drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    }
    
    MTL(endCommandBuffer());
    
    if(captureFrame)
        MTL_STOP_CAPTURE;
}

void Viewer::bakeTextures()
{
    MaterialPtr material = getSelectedMaterial();
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
        mx::TextureBakerPtr baker = mx::TextureBakerMsl::create(bakeWidth, bakeHeight, baseType);
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
}

void Viewer::updateCameras()
{
    mx::Matrix44 viewMatrix, projectionMatrix;
    float aspectRatio = (float) m_size.x() / (float) m_size.y();
    if (_cameraViewAngle != 0.0f)
    {
        viewMatrix = mx::Camera::createViewMatrix(_cameraPosition, _cameraTarget, _cameraUp);
        float fH = std::tan(_cameraViewAngle / 360.0f * M_PI) * _cameraNearDist;
        float fW = fH * aspectRatio;
        projectionMatrix = mx::Camera::createPerspectiveMatrixZP(-fW, fW, -fH, fH, _cameraNearDist, _cameraFarDist);
    }
    else
    {
        viewMatrix = mx::Matrix44::createTranslation(mx::Vector3(0.0f, 0.0f, -ORTHO_VIEW_DISTANCE));
        float fH = ORTHO_PROJECTION_HEIGHT;
        float fW = fH * aspectRatio;
        projectionMatrix = mx::Camera::createOrthographicMatrixZP(-fW, fW, -fH, fH, 0.0f, ORTHO_VIEW_DISTANCE + _cameraFarDist);
    }
    projectionMatrix[1][1] = -projectionMatrix[1][1];
    float turntableRotation = fmod((360.0f / _turntableSteps) * _turntableStep, 360.0f);
    float yRotation = _meshRotation[1] + (_turntableEnabled ? turntableRotation : 0.0f);
    mx::Matrix44 meshRotation = mx::Matrix44::createRotationZ(_meshRotation[2] / 180.0f * M_PI) *
                                mx::Matrix44::createRotationY(yRotation / 180.0f * M_PI) *
                                mx::Matrix44::createRotationX(_meshRotation[0] / 180.0f * M_PI);

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
        _shadowCamera->setProjectionMatrix(mx::Camera::createOrthographicMatrixZP(-r, r, -r, r, 0.0f, r * 2.0f));
        mx::ValuePtr value = dirLight->getInputValue("direction");
        if (value->isA<mx::Vector3>())
        {
            mx::Vector3 dir = mx::Matrix44::createRotationY(_lightRotation / 180.0f * M_PI).transformVector(value->asA<mx::Vector3>());
            _shadowCamera->setViewMatrix(mx::Camera::createViewMatrix(dir * -r, mx::Vector3(0.0f), _cameraUp));
        }
    }
}

MaterialPtr Viewer::getEnvironmentMaterial()
{
    if (!_envMaterial)
    {
        mx::FilePath envFilename = _searchPath.find(mx::FilePath("resources/Lights/envmap_shader.mtlx"));
        try
        {
            _envMaterial = MslMaterial::create();
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

MaterialPtr Viewer::getWireframeMaterial()
{
    if (!_wireMaterial)
    {
        try
        {
            mx::ShaderPtr hwShader = mx::createConstantShader(_genContext, _stdLib, "__WIRE_SHADER__", mx::Color3(1.0f));
            _wireMaterial = MslMaterial::create();
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
        // Create framebuffer.
        mx::MetalFramebufferPtr  framebuffer = mx::MetalFramebuffer::create(
                                                    MTL(device),
                                                    SHADOW_MAP_SIZE,
                                                    SHADOW_MAP_SIZE,
                                                    2,
                                                    mx::Image::BaseType::FLOAT);
        MTL_PUSH_FRAMEBUFFER(framebuffer);
        
        // Generate shaders for shadow rendering.
        if (!_shadowMaterial)
        {
            try
            {
                mx::ShaderPtr hwShader = mx::createDepthShader(_genContext, _stdLib, "__SHADOW_SHADER__");
                _shadowMaterial = MslMaterial::create();
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
                _shadowBlurMaterial = MslMaterial::create();
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
            bool captureShadowGeneration = false;
            if(captureShadowGeneration)
                MTL_TRIGGER_CAPTURE;
            
            MTL(beginCommandBuffer());
            MTLRenderPassDescriptor* renderpassDesc = [MTLRenderPassDescriptor new];
            framebuffer->bind(renderpassDesc);
            MTL(beginEncoder(renderpassDesc));
            [MTL(renderCmdEncoder) setDepthStencilState:MTL_DEPTHSTENCIL_STATE(opaque)];

            // Render shadow geometry.
            _shadowMaterial->bindShader();
            for (auto mesh : _geometryHandler->getMeshes())
            {
                _shadowMaterial->bindMesh(mesh);
                _shadowMaterial->bindViewInformation(_shadowCamera);
                std::static_pointer_cast<MslMaterial>
                (_shadowMaterial)->prepareUsedResources(_shadowCamera,
                                            _geometryHandler,
                                            _imageHandler,
                                            _lightHandler);
                for (size_t i = 0; i < mesh->getPartitionCount(); i++)
                {
                    mx::MeshPartitionPtr geom = mesh->getPartition(i);
                    _shadowMaterial->drawPartition(geom);
                }
            }
            
            MTL(endCommandBuffer());
            
            _shadowMap = framebuffer->getColorImage(MTL(cmdQueue));
            
            MTL(beginCommandBuffer());
            MTL(beginEncoder(renderpassDesc));

            // Apply Gaussian blurring.
            mx::ImageSamplingProperties blurSamplingProperties;
            blurSamplingProperties.uaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
            blurSamplingProperties.vaddressMode = mx::ImageSamplingProperties::AddressMode::CLAMP;
            blurSamplingProperties.filterType = mx::ImageSamplingProperties::FilterType::CLOSEST;
            for (unsigned int i = 0; i < _shadowSoftness; i++)
            {
                framebuffer->bind(renderpassDesc);
                _shadowBlurMaterial->bindShader();
                std::static_pointer_cast<MslMaterial>
                (_shadowBlurMaterial)->getProgram()->bindTexture(_imageHandler,
                                        "image_file_tex",
                                        _shadowMap,
                                        blurSamplingProperties);
                std::static_pointer_cast<MslMaterial>
                (_shadowBlurMaterial)->prepareUsedResources(_identityCamera,
                                                _geometryHandler,
                                                _imageHandler,
                                                _lightHandler);
                _shadowBlurMaterial->unbindGeometry();
                renderScreenSpaceQuad(_shadowBlurMaterial);
                _imageHandler->releaseRenderResources(_shadowMap);
                _shadowMap = framebuffer->getColorImage(MTL(cmdQueue));
            }
            
            MTL(endCommandBuffer());

            MTL_POP_FRAMEBUFFER();
            if(captureShadowGeneration)
                MTL_STOP_CAPTURE;
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
    mx::MetalFramebufferPtr framebuffer = mx::MetalFramebuffer::create(MTL(device),
                            ALBEDO_TABLE_SIZE,
                            ALBEDO_TABLE_SIZE,
                            2,
                            mx::Image::BaseType::FLOAT);
        
        bool captureCommandBuffer = false;
        if(captureCommandBuffer)
            MTL_TRIGGER_CAPTURE;
    
        MTL_PUSH_FRAMEBUFFER(framebuffer);
        
        MTL(beginCommandBuffer());
        
        MTLRenderPassDescriptor* renderpassDesc = [MTLRenderPassDescriptor new];
        
        [renderpassDesc.colorAttachments[0] setTexture:framebuffer->getColorTexture()];
        [renderpassDesc.colorAttachments[0] setClearColor:MTLClearColorMake(0.0f, 0.0f, 0.0f, 0.0f)];
        [renderpassDesc.colorAttachments[0] setLoadAction:MTLLoadActionClear];
        [renderpassDesc.colorAttachments[0] setStoreAction:MTLStoreActionStore];
        
        [renderpassDesc.depthAttachment setTexture:framebuffer->getDepthTexture()];
        [renderpassDesc.depthAttachment setClearDepth:1.0];
        [renderpassDesc.depthAttachment setLoadAction:MTLLoadActionClear];
        [renderpassDesc.depthAttachment setStoreAction:MTLStoreActionStore];
        [renderpassDesc setStencilAttachment:nil];
        
        MTL(beginEncoder(renderpassDesc));

    // Create shader.
    mx::ShaderPtr hwShader = mx::createAlbedoTableShader(_genContext, _stdLib, "__ALBEDO_TABLE_SHADER__");
    MslMaterialPtr material = MslMaterial::create();
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
    material->getProgram()->prepareUsedResources(
                    MTL(renderCmdEncoder),
                    _identityCamera,
                    nullptr,
                    _imageHandler,
                    _lightHandler);
    renderScreenSpaceQuad(material);
    
    MTL(endCommandBuffer());
    
    MTL_POP_FRAMEBUFFER();
    
    if(captureCommandBuffer)
        MTL_STOP_CAPTURE;

    // Store albedo table image.
    _imageHandler->releaseRenderResources(_lightHandler->getAlbedoTable());
    _lightHandler->setAlbedoTable(framebuffer->getColorImage(MTL(cmdQueue)));
    if (_saveGeneratedLights)
    {
        _imageHandler->saveImage("AlbedoTable.exr", _lightHandler->getAlbedoTable());
    }
}

void Viewer::renderScreenSpaceQuad(MaterialPtr material)
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
