#ifndef MATERIALX_GLSLPROGRAM_H
#define MATERIALX_GLSLPROGRAM_H

#include <MaterialXGenShader/HwShader.h>
#include <MaterialXGenShader/HwLightHandler.h>
#include <MaterialXRender/ShaderValidators/ExceptionShaderValidationError.h>
#include <MaterialXRender/Handlers/ViewHandler.h>
#include <MaterialXRender/Handlers/ImageHandler.h>
#include <MaterialXRender/Handlers/GeometryHandler.h>
#include <vector>
#include <string>
#include <unordered_map>

namespace MaterialX
{
// Shared pointer to a GlslProgram
using GlslProgramPtr = std::shared_ptr<class GlslProgram>;

/// @class @GlslProgram
/// GLSL program helper class to perform validation of GLSL source code. 
///
/// There are two main interfaces which can be used. One which takes in a HwShader and one which
/// allows for explicit setting of shader stage code.
///
/// The main services provided are:
///     - Validation: All shader stages are compiled and atteched to a GLSL shader program.
///     - Introspection: The compiled shader program is examined for uniforms and attributes.
///
class GlslProgram
{
  public:
    /// Create a GLSL program instance
    static GlslProgramPtr create();

    /// Destructor
    virtual ~GlslProgram();

    /// @name Shader code setup
    /// @{

    /// Set up code stages to validate based on an input hardware shader.
    /// @param shader Hardware shader to use
    void setStages(const HwShaderPtr shader);

    /// Set the code stages based on a list of stage strings.
    /// Refer to the ordering of stages as defined by a HwShader.
    /// @param stage Name of the shader stage.
    /// @param sourcCode Source code of the shader stage.
    void addStage(const string& stage, const string& sourcCode);

    /// Get source code string for a given stage.
    /// @return Shader stage string. String is empty if not found.
    const string& getStageSourceCode(const string& stage) const;

    /// Clear out any existing stages
    void clearStages();

    /// @}
    /// @name Program validation and introspection
    /// @{

    /// Create the shader program from stages specified
    /// An exception is thrown if the program cannot be created.
    /// The exception will contain a list of program creation errors.
    /// @return Program identifier. 
    unsigned int build();

    /// Structure to hold information about program inputs
    /// The structure is populated by directly scanning the program so may not contain
    /// some inputs listed on any associated HwShader as those inputs may have been
    /// optimized out if they are unused.
    struct Input
    {
        static int INVALID_OPENGL_TYPE;

        /// Program location. -1 means an invalid location
        int location;
        /// OpenGL type of the input. -1 means an invalid type
        int gltype;
        /// Size.
        int size;
        /// Input type string. Will only be non-empty if initialized stages with a HwShader
        std::string typeString;
        /// Input value. Will only be non-empty if initialized stages with a HwShader and a value was set during
        /// shader generation.
        MaterialX::ValuePtr value;
        /// Is this a constant
        bool isConstant;
        /// Element path (if any)
        string path;

        /// Program input constructor
        Input(int inputLocation, int inputType, int inputSize, string inputPath)
            : location(inputLocation)
            , gltype(inputType)
            , size(inputSize)
            , isConstant(false)
            , path(inputPath)
        { }
    };
    /// Program input structure shared pointer type
    using InputPtr = std::shared_ptr<Input>;
    /// Program input shaded pointer map type
    using InputMap = std::unordered_map<std::string, InputPtr>;

    /// Get list of program input uniforms. 
    /// The program must have been created successfully first.
    /// An exception is thrown if the parsing of the program for uniforms cannot be performed.
    /// @return Program uniforms list.
    const InputMap& getUniformsList();

    /// Get list of program input attributes. 
    /// The program must have been created successfully first.
    /// An exception is thrown if the parsing of the program for attribute cannot be performed.
    /// @return Program attributes list.
    const InputMap& getAttributesList();

    /// Find the locations in the program which starts with a given variable name
    /// @param variable Variable to search for
    /// @param variableList List of program inputs to search
    /// @param foundList Returned list of found program inputs. Empty if none found.
    /// @param exactMatch Search for exact variable name match.
    void findInputs(const std::string& variable,
                            const InputMap& variableList,
                            InputMap& foundList,
                            bool exactMatch);

    /// @}
    /// @name Program activation
    /// @{

    /// Bind the program.
    /// @return False if failed
    bool bind();

    /// Bind inputs
    void bindInputs(ViewHandlerPtr viewHandler,
                    GeometryHandler& geometryHandler,
                    ImageHandlerPtr imageHandler,
                    HwLightHandlerPtr lightHandler);

    /// Unbind inputs
    void unbindInputs(ImageHandlerPtr imageHandler);

    /// Return if there are any active inputs on the program
    bool haveActiveAttributes() const;

    /// Assign a parameter value to a uniform
    void bindUniform(int location, const Value& value);

    /// Bind attribute buffers to attribute inputs.
    /// A hardware buffer of the given attribute type is created and bound to the program locations
    /// for the input attribute.
    /// @param inputs Attribute inputs to bind to
    /// @param mesh Mesh containing streams to bind
    void bindAttribute(const GlslProgram::InputMap& inputs, MeshPtr mesh);

    /// Bind input geometry partition (indexing)
    void bindPartition(MeshPartitionPtr partition);

    /// Bind input geometry streams
    void bindStreams(MeshPtr mesh);

    /// Unbind any bound geometry
    void unbindGeometry();

    /// Bind any input textures
    void bindTextures(ImageHandlerPtr imageHandler);

    /// Unbind input textures
    void unbindTextures(ImageHandlerPtr imageHandler);

    /// Bind lighting
    void bindLighting(HwLightHandlerPtr lightHandler, ImageHandlerPtr imageHandler);

    /// Bind view information
    void bindViewInformation(ViewHandlerPtr viewHandler);

    /// Bind time and frame
    void bindTimeAndFrame();

    /// Unbind the program. Equivalent to binding no program
    void unbind() const;

    /// @}

    void printUniforms(std::ostream& outputStream);
    void printAttributes(std::ostream& outputStream);

    /// Constant for an undefined OpenGL resource identifier
    static unsigned int UNDEFINED_OPENGL_RESOURCE_ID;
    /// Constant for a undefined OpenGL program location
    static int UNDEFINED_OPENGL_PROGRAM_LOCATION;

  protected:
    /// Constructor
    GlslProgram();

    ///
    /// @name Program introspection
    /// @{

    /// Update a list of program input uniforms
    const InputMap& updateUniformsList();

    /// Update a list of program input attributes
    const InputMap& updateAttributesList();

    /// Clear out any cached input lists
    void clearInputLists();   

    /// Utility to map a MaterialX type to an OpenGL type
    /// @param type MaterialX type
    /// @return OpenGL type. INVALID_OPENGL_TYPE is returned if no mapping exists. For example strings have no OpenGL type.
    static int mapTypeToOpenGLType(const TypeDesc* type);

    /// Utility to find a uniform value in an uniform list.
    /// If uniform cannot be found a null pointer will be return.
    MaterialX::ValuePtr findUniformValue(const std::string& uniformName, const InputMap& uniformList);

    /// @}
    /// @name Utilities
    /// @{

    /// Bind an individual texture to a program uniform location
    bool bindTexture(unsigned int uniformType, int uniformLocation, const string& fileName,
                     ImageHandlerPtr imageHandler, bool generateMipMaps, const ImageSamplingProperties& imageProperties);

    /// Utility to check for OpenGL context errors.
    /// Will throw an ExceptionShaderValidationError exception which will list of the errors found
    /// if any errors encountered.
    void checkErrors();

    /// Delete any currently created shader program
    void deleteProgram();

    /// @}

  private:
    /// Stages used to create program
    /// Map of stage name and its source code
    std::unordered_map<string, string> _stages;

    /// Generated program. A non-zero number indicates a valid shader program.
    unsigned int _programId;

    /// List of program input uniforms
    InputMap _uniformList;
    /// List of program input attributes
    InputMap _attributeList;

    /// Hardware shader (if any) used for program creation
    HwShaderPtr _hwShader;

    /// Attribute buffer resource handles
    /// for each attribute identifier in the program
    std::unordered_map<std::string, unsigned int> _attributeBufferIds;

    /// Attribute indexing buffer handle
    unsigned int _indexBuffer;
    /// Size of index buffer
    size_t _indexBufferSize;

    /// Attribute vertex array handle
    unsigned int _vertexArray;

    /// Program texture map
    std::unordered_map<std::string, unsigned int> _programTextures;
};

} // namespace MaterialX
#endif
