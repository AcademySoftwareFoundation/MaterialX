#ifndef MATERIALX_SHADERGENERATOR_H
#define MATERIALX_SHADERGENERATOR_H

#include <MaterialXShaderGen/Shader.h>
#include <MaterialXShaderGen/Syntax.h>
#include <MaterialXShaderGen/Factory.h>

#include <MaterialXCore/Util.h>

#include <MaterialXFormat/File.h>

namespace MaterialX
{

using ShaderGeneratorPtr = shared_ptr<class ShaderGenerator>;

/// Base class for shader generators
/// All 3rd party shader generators should derive from this class.
/// Derived classes should use DECLARE_SHADER_GENERATOR / DEFINE_SHADER_GENERATOR
/// in it's declaration / definition, and register with the Registry class.
class ShaderGenerator
{
public:
    /// Destructor
    virtual ~ShaderGenerator() {}

    /// Return a unique identifyer for the language used by this generator
    virtual const string& getLanguage() const = 0;

    /// Return a unique identifyer for the target this generator is for
    virtual const string& getTarget() const = 0;

    /// Generate a shader starting from the given element, translating 
    /// the element and all dependencies upstream into shader code.
    virtual ShaderPtr generate(const string& shaderName, ElementPtr element) = 0;

    /// Emit typedefs for all data types that needs it
    virtual void emitTypeDefs(Shader& shader);

    /// Emit function definitions for all nodes
    virtual void emitFunctions(Shader& shader);

    /// Emit the shader body
    virtual void emitShaderBody(Shader& shader);

    /// Emit the final output expression
    virtual void emitFinalOutput(Shader& shader) const;

    /// Emit a shader uniform input variable
    virtual void emitUniform(const string& name, const string& type, const ValuePtr& value, Shader& shader);

    /// Emit the connected variable name for an input,
    /// or constant value if the port is not connected
    virtual void emitInput(const SgInput* input, Shader& shader);

    /// Emit the output variable name for an output, optionally including it's type
    virtual void emitOutput(const SgOutput* output, bool includeType, Shader& shader);

    ///
    virtual bool shouldPublish(const ValueElement* port, string& publicName) const;

    using Argument = std::pair<string, string>;
    virtual const vector<Argument>* getExtraArguments(const SgNode& node) const;

    /// Return the v-direction used by the target system
    virtual Shader::VDirection getTargetVDirection() const;

    /// Return the syntax object for the language used by the code generator
    SyntaxPtr getSyntax() const { return _syntax; }

    template<class T>
    using CreatorFunc = shared_ptr<T>(*)();

    /// Register a shader gen implementation for a given implementation element name
    void registerImplementation(const string& name, CreatorFunc<SgImplementation> creator);

    /// Return a registered shader gen implementation given an implementation element.
    /// If no registered implementaion is found a default source code implementation 
    /// instance will be returned.
    SgImplementationPtr getImplementation(ElementPtr element);

    /// Add to the search path used for finding source code.
    static void registerSourceCodeSearchPath(const FilePath& path);

    /// Resolve a source code file using the registered search paths.
    static FilePath findSourceCode(const FilePath& filename);

protected:
    /// Protected constructor
    ShaderGenerator(SyntaxPtr syntax);

    SyntaxPtr _syntax;
    Factory<SgImplementation> _implFactory;
    unordered_map<string, SgImplementationPtr> _cachedImpls;

    static FileSearchPath _sourceCodeSearchPath;
};

} // namespace MaterialX

  /// Macro declaring required members and methods for a shader generator
#define DECLARE_SHADER_GENERATOR(T)                                           \
    public:                                                                   \
        static const string kLanguage;                                        \
        static const string kTarget;                                          \
        static ShaderGeneratorPtr creator() { return std::make_shared<T>(); } \
        const string& getLanguage() const override { return kLanguage; }      \
        const string& getTarget() const override { return kTarget; }          \

  /// Macro defining required members and methods for a shader generator
  /// LNG sets the language string identifier
  /// TRG sets the target string identifier
#define DEFINE_SHADER_GENERATOR(T, LNG, TRG) \
    const string T::kLanguage = LNG;         \
    const string T::kTarget = TRG;           \

#endif
