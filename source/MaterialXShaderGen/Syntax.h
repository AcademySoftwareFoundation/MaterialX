#ifndef MATERIALX_SYNTAX_H
#define MATERIALX_SYNTAX_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Value.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Node.h>

#include <utility>
#include <set>

namespace MaterialX
{

class SgInput;
class SgOutput;

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

    using UniqueNameMap = std::unordered_map<string, size_t>;

public:
    virtual ~Syntax() {}

    /// Add syntax information for a data type
    /// Required to be set for each data type
    void addTypeSyntax(const string& type, const TypeSyntax& syntax);

    /// Add value constructor syntax for a data type
    /// Optional and only used if a custom constructor is needed for a value
    void addValueConstructSyntax(const string& type, const ValueConstructSyntax& syntax);

    /// Register names that are restricted to use by a code generator when naming 
    /// variables and functions. Keywords, types, built-in functions etc. should be 
    /// added to this set. Multiple calls will add to the internal set of names.
    void addRestrictedNames(const StringSet& names);

    /// Returns the value string for a given value object
    virtual string getValue(const Value& value, const string& type, bool paramInit = false) const;

    /// Returns the name syntax of the given type
    virtual const string& getTypeName(const string& type) const;

    /// Returns the default value of the given type
    virtual const string& getTypeDefault(const string& type, bool paramInit = false) const;

    /// Returns the custom typedef syntax for the given data type
    /// If not used returns an empty string
    virtual const string& getTypeDef(const string& type) const;

    /// Returns the type name in an output context
    virtual const string& getOutputTypeName(const string& type) const;

    /// Get syntax for a swizzled variable
    virtual string getSwizzledVariable(const string& name, const string& type, const string& fromType, const string& channels) const;

    /// Returns an array of all registered type syntax objects
    const vector<TypeSyntax>& getTypeSyntax() const { return _typeSyntax; }

    /// Returns a set of names that are restricted to use for this language syntax.
    const StringSet& getRestrictedNames() const { return _restrictedNames; }

    /// Modify the given name string to make it unique according to the given uniqueName record 
    /// and according to restricted names registered for this syntax class.
    /// The method is used for naming variables (inputs and outputs) in generated code.
    /// Derived classes can override this method to have a custom naming strategy.
    /// Default implementation adds a number suffix, or increases an existing number suffix, 
    /// on the name string if there is a name collision.
    virtual void makeUnique(string& name, UniqueNameMap& uniqueNames) const;

    /// Modify the give name string to make it appropriate as a public shader uniform.
    /// The shader generator will call this function on all public shader uniforms
    /// to give the syntax class a chance to modify public names.
    /// Derived classes can override this method if needed. The default implementation 
    /// will leave the name unchanged.
    virtual void renamePublicUniform(string& name, const string& type) const;

protected:
    /// Protected constructor
    Syntax();

private:
    vector<TypeSyntax> _typeSyntax;
    std::unordered_map<string, size_t> _typeSyntaxByName;

    vector<ValueConstructSyntax> _valueConstructSyntax;
    std::unordered_map<string, size_t> _valueConstructSyntaxByName;

    StringSet _restrictedNames;
};


/// Built in data types
class DataType
{
public:
    static const string BOOLEAN;
    static const string INTEGER;
    static const string FLOAT;
    static const string VECTOR2;
    static const string VECTOR3;
    static const string VECTOR4;
    static const string COLOR2;
    static const string COLOR3;
    static const string COLOR4;
    static const string MATRIX3;
    static const string MATRIX4;
    static const string STRING;
    static const string FILENAME;
    static const string BSDF;
    static const string EDF;
    static const string VDF;
    static const string SURFACE;
    static const string VOLUME;
    static const string DISPLACEMENT;
    static const string LIGHT;

    static const std::set<string> SCALARS;
    static const std::set<string> TUPLES;
    static const std::set<string> TRIPLES;
    static const std::set<string> QUADRUPLES;

    static bool isScalar(const string& type) { return SCALARS.count(type) > 0; }
    static bool isTuple(const string& type) { return TUPLES.count(type) > 0; }
    static bool isTriple(const string& type) { return TRIPLES.count(type) > 0; }
    static bool isQuadruple(const string& type) { return QUADRUPLES.count(type) > 0; }
};

} // namespace MaterialX

#endif
