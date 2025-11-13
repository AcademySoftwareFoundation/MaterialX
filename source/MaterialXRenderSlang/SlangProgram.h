//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SLANGPROGRAM_H
#define MATERIALX_SLANGPROGRAM_H

/// @file
/// Slang code program

#include <MaterialXRenderSlang/Export.h>
#include <MaterialXRenderSlang/SlangContext.h>

#include <MaterialXRender/ImageHandler.h>
#include <MaterialXRender/ShaderRenderer.h>

#include <vector>
#include <unordered_map>

MATERIALX_NAMESPACE_BEGIN

using SlangFramebufferPtr = std::shared_ptr<class SlangFramebuffer>;
using SlangProgramPtr = std::shared_ptr<class SlangProgram>;

class SlangProgram
{
  public:
    static SlangProgramPtr create(SlangContextPtr context);

    ~SlangProgram();

    /// @name Shader code setup
    /// @{

    /// Set up code stages to validate based on an input hardware shader.
    /// @param shader Hardware shader to use
    void setStages(ShaderPtr shader);

    /// Set the code stages based on a list of stage strings.
    /// Refer to the ordering of stages as defined by a HwShader.
    /// Does not clear stages.
    /// @param stage Name of the shader stage.
    /// @param sourceCode Source code of the shader stage.
    void addStage(const string& stage, const string& sourceCode);

    /// Get source code string for a given stage.
    /// @return Shader stage string. String is empty if not found.
    const string& getStageSourceCode(const string& stage) const;

    /// Clear out any existing stages
    void clearStages();

    /// Return the shader, if any, used to generate this program.
    ShaderPtr getShader() const
    {
        return _shader;
    }

    /// @brief Builds the program defined by vertex and pixel stages.
    /// Will throw on failure compilation failure. Upon success,
    /// rootObject is created and uniform values can be bound to it.
    /// @return true when rootObject has been created successfully.
    bool build();

    struct UniformInput
    {
        /// Variable name, with `parent.child` name pattern for individual struct members.
        std::string name;
        /// All cursors to which to write the value (could be in both fragment and vertex shader cbuffers).
        std::vector<rhi::ShaderCursor> slangCursors;
        /// Byte size of the slang type, cached from slangCursors.
        size_t slangByteSize;
        /// Default value to set onto SlangRHI descriptor. If not valid, all memory will be set to 0.
        ConstValuePtr slangDefaultValue;

        /// Only valid when shader is attached and used for printing.
        /// Element path (if any)
        std::string mxElementPath;
        /// Unit
        std::string mxUnit;
        /// Colorspace
        std::string mxColorspace;
    };
    /// Program input structure shared pointer type
    using UniformInputPtr = std::shared_ptr<UniformInput>;
    /// Program input shaded pointer map type
    using UniformInputMap = std::unordered_map<string, UniformInputPtr>;

    struct VertexInput
    {
        /// Index of the buffer to bind input to
        unsigned bufferIndex;
        /// Semantic name used to bind the input
        std::string valueSemanticName;
        /// TODO: The type does not match between Slang's getSemanticIndex and SlangRHI's InputElementDesc::semanticIndex.
        /// Opting for the SlangRHI type, but in the future this will probably turn into slang::SlangIndex
        uint32_t valueSemanticIndex = 0;
        /// Slang reflection type
        slang::TypeLayoutReflection* slangTypeLayout = nullptr;
        /// Input type string. Will only be non-empty if initialized stages with a HwShader
        string typeString;
    };
    /// Program input structure shared pointer type
    using VertexInputPtr = std::shared_ptr<VertexInput>;
    /// Program input shaded pointer map type
    using VertexInputMap = std::unordered_map<string, VertexInputPtr>;

    /// Get list of program input uniforms.
    /// The program must have been built first.
    const UniformInputMap& getUniformsList();

    /// Get list of program vertex stage inputs.
    /// The program must have been built first.
    const VertexInputMap& getVertexInputsList();

    /// Set the pass encoder and render state to bind further mesh/partition
    void bind() { }
    void bind(SlangRenderPassEncoder* passEncoder, SlangRenderState* renderState)
    {
        _passEncoder = passEncoder;
        _renderState = renderState;
    }

    void unbind()
    {
        _passEncoder = nullptr;
        _renderState = nullptr;
    }

    /// Return true if a uniform with the given name is present.
    bool hasUniform(const string& name);

    /// Bind a value to the uniform with the given name.
    void bindUniform(const string& name, ConstValuePtr value, bool errorIfMissing = true);

  protected:
    /// Binds all vertex inputs for a given mesh and caches them for the next call
    void bindVertexInputs(MeshPtr mesh, rhi::RenderState& renderState);

  public:
    /// Bind the partition (create index buffer), return the buffer and cache it.
    /// Binds it to the RenderState, if that is set.
    rhi::ComPtr<rhi::IBuffer> bindPartition(MeshPartitionPtr part);

    /// Draw a previously bound partition
    void drawPartition(MeshPartitionPtr partition);

    /// Bind the mesh (creating input buffers if necessary) to the renderState.
    void bindMesh(MeshPtr mesh, rhi::RenderState& renderState);

    void bindMesh(MeshPtr mesh)
    {
        bindMesh(mesh, *_renderState);
    }

    /// Must be preceded by bindPartition() call for the same partition.
    /// TODO: Evaluate if this is actually necessary or should be a single call.
    rhi::ComPtr<rhi::IBuffer> getIndexBuffer(MeshPartitionPtr partition);

    /// Clears all the bindMesh and bindPartition results.
    void unbindGeometry();

    /// Bind all default input textures. Not env maps and not those already bound by bindTexture calls.
    void bindTextures(ImageHandlerPtr imageHandler);

    /// Bind a specific texture explicitly, will remove binding in `bindTextures` and will not be overriden by it.
    void bindTexture(ImageHandlerPtr imageHandler,
                     const std::string& shaderTextureName,
                     ImagePtr imagePtr,
                     const ImageSamplingProperties& samplingProperties);

    /// Removes the explicit binding of a texture, the following call to `bindTextures` will bind the default one.
    void unbindTexture(const std::string& shaderTextureName)
    {
        _explicitBoundImages.erase(shaderTextureName);
    }

    void bindLighting(LightHandlerPtr lightHandler, ImageHandlerPtr imageHandler);

    void bindViewInformation(CameraPtr camera);

    void bindTimeAndFrame(float time = 0.0f, float frame = 1.0f);

    enum class PipelineKind2
    {
        Default = 0,         // automatic alpha blending (based on isTransparent) and no culling
        AlphaOn = (1 << 0),  // alpha blending turned on
        AlphaOff = (1 << 1), // alpha turned on/off based on the transparency in the shader
        CullFront = (1 << 2),
        CullBack = (1 << 3),
        Wireframe = (1 << 4)
    };

    rhi::ComPtr<rhi::IRenderPipeline> getPipeline(PipelineKind2 kind, rhi::Format colorFormat);
    rhi::ComPtr<rhi::IRenderPipeline> getPipeline(PipelineKind2 kind, const SlangFramebufferPtr& framebuffer);

    /// Binds the pipeline and the rootObject to it. Does not deep copy the rootObject (uniforms),
    /// until after
    void bindPipeline(rhi::IRenderPassEncoder* passEncoder, rhi::ComPtr<rhi::IRenderPipeline> pipeline)
    {
        passEncoder->bindPipeline(pipeline, _rootObject);
    }

    SlangShaderObject* getRootObject()
    {
        return _rootObject;
    }

  public:
    void clearBuiltData();

    void printUniforms(std::ostream& outputStream);
    void printVertexInputs(std::ostream& outputStream);

    /// Returns if alpha blending is enabled.
    bool isTransparent() const { return _alphaBlendingEnabled; }

  public:
    static constexpr unsigned int UNDEFINED_SLANG_RESOURCE_ID = 0;

  protected:
    SlangProgram(SlangContextPtr context);

    void bindUniformDefaults();
    bool isBuilt() const
    {
        return (bool) _rootObject;
    }
    void bindUniformGeomprops(MeshPtr mesh);

    const VertexInputMap& updateVertexInputList();
    const UniformInputMap& updateUniformsList();

    struct PipelineKey
    {
        PipelineKind2 kind;
        rhi::Format format;

        bool operator<(const PipelineKey& rhs) const
        {
            return std::tie(kind, format) < std::tie(rhs.kind, rhs.format);
        }
    };

  public:
    std::string _name;
    SlangContextPtr _context;

    rhi::ComPtr<rhi::IShaderObject> _rootObject = nullptr;
    std::map<PipelineKey, rhi::ComPtr<rhi::IRenderPipeline>> _pipelines;

    StringMap _stages;
    ShaderPtr _shader;
    slang::ProgramLayout* _slangReflection;
    rhi::ComPtr<rhi::IShaderProgram> _shaderProgram;
    VertexInputMap _vertexInputsList;
    UniformInputMap _uniformList;

    std::map<std::string, ImagePtr> _explicitBoundImages;

    std::map<MeshPartitionPtr, rhi::ComPtr<rhi::IBuffer>> _indexBuffers;
    std::map<std::string, rhi::ComPtr<rhi::IBuffer>> _vertexInputBuffers;
    rhi::ComPtr<rhi::IInputLayout> _inputLayout;
    MeshPtr _boundMesh;

    bool _alphaBlendingEnabled = false;

    SlangRenderPassEncoder* _passEncoder = nullptr;
    SlangRenderState* _renderState = nullptr;
};

SLANG_RHI_ENUM_CLASS_OPERATORS(SlangProgram::PipelineKind2)

MATERIALX_NAMESPACE_END

#endif
