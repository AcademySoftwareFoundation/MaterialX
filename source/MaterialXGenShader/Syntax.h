#ifndef MATERIALX_SYNTAX_H
#define MATERIALX_SYNTAX_H

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Value.h>
#include <MaterialXCore/Definition.h>

#include <utility>
#include <set>

namespace MaterialX
{

class TypeDesc;

using SyntaxPtr = shared_ptr<class Syntax>;
using TypeSyntaxPtr = shared_ptr<class TypeSyntax>;

/// Base class for syntax objects used by shader generators
/// to emit code with correcy syntax for each language.
class Syntax
{
public:
    using UniqueNameMap = std::unordered_map<string, size_t>;

public:
    virtual ~Syntax() {}

    /// Register syntax handling for a data type.
    /// Required to be set for all supported data types.
    void registerTypeSyntax(const TypeDesc* type, TypeSyntaxPtr syntax);

    /// Register names that are restricted to use by a code generator when naming 
    /// variables and functions. Keywords, types, built-in functions etc. should be 
    /// added to this set. Multiple calls will add to the internal set of names.
    void registerRestrictedNames(const StringSet& names);

    /// Returns the type syntax object for a named type.
    /// Throws an exception if a type syntax is not defined for the given type.
    const TypeSyntax& getTypeSyntax(const TypeDesc* type) const;

    /// Returns an array of all registered type syntax objects
    const vector<TypeSyntaxPtr>& getTypeSyntaxs() const { return _typeSyntaxs; }

    /// Returns the name syntax of the given type
    const string& getTypeName(const TypeDesc* type) const;

    /// Returns the type name in an output context
    string getOutputTypeName(const TypeDesc* type) const;

    /// Returns the custom typedef syntax for the given data type
    /// If not used returns an empty string
    const string& getTypeDefStatement(const TypeDesc* type) const;

    /// Returns the default value string for the given type
    const string& getDefaultValue(const TypeDesc* type, bool uniform = false) const;

    /// Returns the value string for a given type and value object
    string getValue(const TypeDesc* type, const Value& value, bool uniform = false) const;

    /// Get syntax for a swizzled variable
    string getSwizzledVariable(const string& srcName, const TypeDesc* srcType, const string& channels, const TypeDesc* dstType) const;

    /// Returns a set of names that are restricted to use for this language syntax.
    const StringSet& getRestrictedNames() const { return _restrictedNames; }

    /// Returns a type qualifier to be used when declaring types for output variables.
    /// Default implementation returns empty string and derived syntax classes should
    /// override this method.
    virtual const string& getOutputQualifier() const { return EMPTY_STRING; };

    /// Get the qualifier used when declaring constant variables.
    /// Derived classes must define this method. 
    virtual const string& getConstantQualifier() const = 0;

    /// Get the qualifier used when declaring uniform variables.
    /// Default implementation returns empty string and derived syntax classes should
    /// override this method.
    virtual const string& getUniformQualifier() const { return EMPTY_STRING; };

    /// Query if given type is suppored in the syntax
    /// By default all types are assumed to be supported
    virtual bool typeSupported(const TypeDesc* /*type*/) const { return true; }

    /// Modify the given name string to make it unique according to the given uniqueName record 
    /// and according to restricted names registered for this syntax class.
    /// The method is used for naming variables (inputs and outputs) in generated code.
    /// Derived classes can override this method to have a custom naming strategy.
    /// Default implementation adds a number suffix, or increases an existing number suffix, 
    /// on the name string if there is a name collision.
    virtual void makeUnique(string& name, UniqueNameMap& uniqueNames) const;

protected:
    /// Protected constructor
    Syntax();

private:
    vector<TypeSyntaxPtr> _typeSyntaxs;
    std::unordered_map<const TypeDesc*, size_t> _typeSyntaxByType;

    StringSet _restrictedNames;
};

/// Base class for syntax handling of types.
class TypeSyntax
{
public:
    virtual ~TypeSyntax() {}

    /// Returns the type name.
    const string& getName() const { return _name; }

    /// Returns a typedef string if needed to define the type in the target language.
    const string& getTypeDefStatement() const { return _typeDefStatement; }

    /// Returns the default value for this type.
    const string& getDefaultValue(bool uniform) const { return uniform ? _uniformDefaultValue : _defaultValue; }

    /// Returns the syntax for accessing type members if the type 
    /// can be swizzled.
    const vector<string>& getMembers() const { return _members; }

    /// Returns a value formatted according to this type syntax.
    /// The value is constructed from the given value object.
    virtual string getValue(const Value& value, bool uniform) const = 0;

    /// Returns a value formatted according to this type syntax.
    /// The value is constructed from the given list of value entries
    /// with one entry for each member of the type.
    virtual string getValue(const vector<string>& values, bool uniform) const = 0;

protected:
    /// Protected constructor
    TypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue, 
        const string& typeDefStatement, const vector<string>& members);

    string _name;                // type name
    string _defaultValue;        // default value syntax
    string _uniformDefaultValue; // default value syntax when assigned to uniforms
    string _typeDefStatement;    // custom typedef statement if needed in source code
    vector<string> _members;     // syntax for member access

    static const vector<string> EMPTY_MEMBERS;
};

/// Syntax class for scalar types.
class ScalarTypeSyntax : public TypeSyntax
{
public:
    ScalarTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue, 
        const string& typeDefStatement = EMPTY_STRING);

    string getValue(const Value& value, bool uniform) const override;
    string getValue(const vector<string>& values, bool uniform) const override;
};

/// Syntax class for string types.
class StringTypeSyntax : public ScalarTypeSyntax
{
public:
    StringTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue,
        const string& typeDefStatement = EMPTY_STRING);

    string getValue(const Value& value, bool uniform) const override;
};

/// Syntax class for aggregate types.
class AggregateTypeSyntax : public TypeSyntax
{
public:
    AggregateTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue,
        const string& typeDefStatement = EMPTY_STRING, const vector<string>& members = EMPTY_MEMBERS);

    string getValue(const Value& value, bool uniform) const override;
    string getValue(const vector<string>& values, bool uniform) const override;
};

} // namespace MaterialX

#endif
