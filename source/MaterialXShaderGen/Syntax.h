#ifndef MATERIALX_SYNTAX_H
#define MATERIALX_SYNTAX_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Value.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Node.h>

#include <utility>

namespace MaterialX
{

using SyntaxPtr = shared_ptr<class Syntax>;

/// Base class for syntax objects used by code generators
/// to emit code with correcy syntax for each language
class Syntax
{
public:
    /// Struct holding information on syntax to be used for data types
    /// Required to be set for each data type
    struct TypeSyntax
    {
        string name;                // type name
        string defaultValue;        // default value
        string paramDefaultValue;   // default value in a shader param initialization context
        string typeDef;             // custom typedef if needed in source code
        string outputName;          // type name in output context

        TypeSyntax() {}
        TypeSyntax(const string& n, const string& dv, const string& pdv, const string& dn, const string& on)
            : name(n)
            , defaultValue(dv)
            , paramDefaultValue(pdv)
            , typeDef(dn)
            , outputName(on)
        {}
    };

    /// Struct holding information on syntax needed to construct values and swizzle patterns for data types
    /// Optional and only used if a custom constructor and/or swizzling is needed for a data type
    struct ValueConstructSyntax
    {
        std::pair<string, string> valueConstructor;      // value constructor prefix/post fix
        std::pair<string, string> paramValueConstructor; // value constructor prefix/post fix in a shader param initialization context
        vector<string> vectorComponents;            // syntax for each vector component if swizzling is supported

        ValueConstructSyntax() {}
        ValueConstructSyntax(const string& pre, const string& post, const string& paramPre, const string& paramPost, const vector<string>& vecComponents)
            : valueConstructor(pre,post)
            , paramValueConstructor(paramPre, paramPost)
            , vectorComponents(vecComponents)
        {}
    };

public:
    virtual ~Syntax() {}

    /// Add syntax information for a data type
    /// Required to be set for each data type
    void addTypeSyntax(const string& type, const TypeSyntax& syntax);

    /// Add value constructor syntax for a data type
    /// Optional and only used if a custom constructor is needed for a value
    void addValueConstructSyntax(const string& type, const ValueConstructSyntax& syntax);

    /// Returns the value string for a given value object
    virtual string getValue(const Value& value, bool paramInit = false) const;

    /// Returns the name syntax of the given type
    virtual const string& getTypeName(const string& type) const;

    /// Returns the default value of the given type
    virtual const string& getTypeDefault(const string& type, bool paramInit = false) const;

    /// Returns the custom typedef syntax for the given data type
    /// If not used returns an empty string
    virtual const string& getTypeDef(const string& type) const;

    /// Returns the type name in an output context
    virtual const string& getOutputTypeName(const string& type) const;

    /// Get variable name for an input, output or node
    virtual string getVariableName(const Element& elem, bool includeParentName = false) const;

    /// Get syntax for a swizzled variable
    virtual string getSwizzledVariable(const string& name, const string& type, const string& fromType, const string& channels) const;

    /// Returnes an array of all registered type syntax objects
    const vector<TypeSyntax>& getTypeSyntax() const { return _typeSyntax; }

protected:
    /// Protected constructor
    Syntax();

    vector<TypeSyntax> _typeSyntax;
    std::unordered_map<string, size_t> _typeSyntaxByName;

    vector<ValueConstructSyntax> _valueConstructSyntax;
    std::unordered_map<string, size_t> _valueConstructSyntaxByName;
};

/// Built in data types
static const string kBoolean("boolean");
static const string kInteger("integer");
static const string kFloat("float");
static const string kVector2("vector2");
static const string kVector3("vector3");
static const string kVector4("vector4");
static const string kColor2("color2");
static const string kColor3("color3");
static const string kColor4("color4");
static const string kString("string");
static const string kFilename("filename");
static const string kBSDF("BSDF");
static const string kEDF("EDF");
static const string kVDF("VDF");
static const string kSURFACE("surfaceshader");
static const string kVOLUME("volumeshader");
static const string kDISPLACEMENT("displacementshader");

} // namespace MaterialX

#endif
