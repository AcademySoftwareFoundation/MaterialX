//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_SYNTAX_H
#define MATERIALX_SYNTAX_H

/// @file
/// Base class for syntax handling for shader generators

#include <MaterialXCore/Library.h>
#include <MaterialXCore/Value.h>
#include <MaterialXCore/Definition.h>

#include <utility>

namespace MaterialX
{

class Syntax;
class TypeSyntax;
class TypeDesc;

/// Shared pointer to a Syntax
using SyntaxPtr = shared_ptr<Syntax>;
/// Shared pointer to a constant Syntax
using ConstSyntaxPtr = shared_ptr<const Syntax>;
/// Shared pointer to a TypeSyntax
using TypeSyntaxPtr = shared_ptr<TypeSyntax>;

/// @class Syntax
/// Base class for syntax objects used by shader generators
/// to emit code with correct syntax for each language.
class Syntax
{
public:
    using UniqueNameMap = std::unordered_map<string, size_t>;

    /// Punctuation types
    enum Punctuation
    {
        PARENTHESES,
        CURLY_BRACKETS,
        SQUARE_BRACKETS
    };

public:
    virtual ~Syntax() {}

    /// Register syntax handling for a data type.
    /// Required to be set for all supported data types.
    void registerTypeSyntax(const TypeDesc* type, TypeSyntaxPtr syntax);

    /// Register names that are restricted to use by a code generator when naming 
    /// variables and functions. Keywords, types, built-in functions etc. should be 
    /// added to this set. Multiple calls will add to the internal set of names.
    void registerRestrictedNames(const StringSet& names);

    /// Register a set string replacements for disallowed tokens 
    /// for a code generator when naming variables and functions. 
    /// Multiple calls will add to the internal set of tokens.
    void registerInvalidTokens(const StringMap& tokens);

    /// Returns the type syntax object for a named type.
    /// Throws an exception if a type syntax is not defined for the given type.
    const TypeSyntax& getTypeSyntax(const TypeDesc* type) const;

    /// Returns an array of all registered type syntax objects
    const vector<TypeSyntaxPtr>& getTypeSyntaxes() const { return _typeSyntaxes; }

    /// Returns the name syntax of the given type
    const string& getTypeName(const TypeDesc* type) const;

    /// Returns the type name in an output context
    string getOutputTypeName(const TypeDesc* type) const;

    /// Returns a type alias for the given data type.
    /// If not used returns an empty string.
    const string& getTypeAlias(const TypeDesc* type) const;

    /// Returns a custom type definition if needed for the given data type.
    /// If not used returns an empty string.
    const string& getTypeDefinition(const TypeDesc* type) const;

    /// Returns the default value string for the given type
    const string& getDefaultValue(const TypeDesc* type, bool uniform = false) const;

    /// Returns the value string for a given type and value object
    string getValue(const TypeDesc* type, const Value& value, bool uniform = false) const;

    /// Get syntax for a swizzled variable
    string getSwizzledVariable(const string& srcName, const TypeDesc* srcType, const string& channels, const TypeDesc* dstType) const;

    /// Returns a set of names that are restricted to use for this language syntax.
    const StringSet& getRestrictedNames() const { return _restrictedNames; }

    /// Returns a mapping from disallowed tokens to replacement strings for this language syntax.
    const StringMap& getInvalidTokens() const { return _invalidTokens; }

    /// Returns a type qualifier to be used when declaring types for output variables.
    /// Default implementation returns empty string and derived syntax classes should
    /// override this method.
    virtual const string& getInputQualifier() const { return EMPTY_STRING; };

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

    /// Return the characters used for a newline.
    virtual const string& getNewline() const { return NEWLINE; };

    /// Return the characters used for a single indentation level.
    virtual const string& getIndentation() const { return INDENTATION; };

    /// Return the characters used to begin/end a string definition.
    virtual const string& getStringQuote() const { return STRING_QUOTE; };

    /// Return the string pattern used for a file include statement.
    virtual const string& getIncludeStatement() const { return INCLUDE_STATEMENT; };

    /// Return the characters used for single line comment.
    virtual const string& getSingleLineComment() const { return SINGLE_LINE_COMMENT; };

    /// Return the characters used to begin a multi line comments block.
    virtual const string& getBeginMultiLineComment() const { return BEGIN_MULTI_LINE_COMMENT; };

    /// Return the characters used to end a multi line comments block.
    virtual const string& getEndMultiLineComment() const { return END_MULTI_LINE_COMMENT; };

    /// Return the array suffix to use for declaring an array variable.
    virtual string getArraySuffix(const TypeDesc* type, const Value& value) const;

    /// Query if given type is suppored in the syntax.
    /// By default all types are assumed to be supported.
    virtual bool typeSupported(const TypeDesc* type) const;

    /// Modify the given name string to make it unique according to the given uniqueName record 
    /// and according to restricted names registered for this syntax class.
    /// The method is used for naming variables (inputs and outputs) in generated code.
    /// Derived classes can override this method to have a custom naming strategy.
    /// Default implementation adds a number suffix, or increases an existing number suffix, 
    /// on the name string if there is a name collision.
    virtual void makeUnique(string& name, UniqueNameMap& uniqueNames) const;

    /// Modify the given name string to remove any invalid characters or tokens.
    virtual void makeValidName(string& name) const;

protected:
    /// Protected constructor
    Syntax();

private:
    vector<TypeSyntaxPtr> _typeSyntaxes;
    std::unordered_map<const TypeDesc*, size_t> _typeSyntaxByType;

    StringSet _restrictedNames;
    StringMap _invalidTokens;

    static const string NEWLINE;
    static const string INDENTATION;
    static const string STRING_QUOTE;
    static const string INCLUDE_STATEMENT;
    static const string SINGLE_LINE_COMMENT;
    static const string BEGIN_MULTI_LINE_COMMENT;
    static const string END_MULTI_LINE_COMMENT;
};

/// @class TypeSyntax
/// Base class for syntax handling of types.
class TypeSyntax
{
public:
    virtual ~TypeSyntax() {}

    /// Returns the type name.
    const string& getName() const { return _name; }

    /// Returns a type alias if needed to define the type in the target language.
    const string& getTypeAlias() const { return _typeAlias; }

    /// Returns a type definition if needed to define the type in the target language.
    const string& getTypeDefinition() const { return _typeDefinition; }

    /// Returns the default value for this type.
    const string& getDefaultValue(bool uniform) const { return uniform ? _uniformDefaultValue : _defaultValue; }

    /// Returns the syntax for accessing type members if the type 
    /// can be swizzled.
    const StringVec& getMembers() const { return _members; }

    /// Returns a value formatted according to this type syntax.
    /// The value is constructed from the given value object.
    virtual string getValue(const Value& value, bool uniform) const = 0;

    /// Returns a value formatted according to this type syntax.
    /// The value is constructed from the given list of value entries
    /// with one entry for each member of the type.
    virtual string getValue(const StringVec& values, bool uniform) const = 0;

protected:
    /// Protected constructor
    TypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue, 
        const string& typeAlias, const string& typeDefinition, const StringVec& members);

    string _name;                // type name
    string _defaultValue;        // default value syntax
    string _uniformDefaultValue; // default value syntax when assigned to uniforms
    string _typeAlias;           // type alias if needed in source code
    string _typeDefinition;      // custom type definition if needed in source code
    StringVec _members;          // syntax for member access

    static const StringVec EMPTY_MEMBERS;
};

/// Syntax class for scalar types.
class ScalarTypeSyntax : public TypeSyntax
{
public:
    ScalarTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue, 
        const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING);

    string getValue(const Value& value, bool uniform) const override;
    string getValue(const StringVec& values, bool uniform) const override;
};

/// Syntax class for string types.
class StringTypeSyntax : public ScalarTypeSyntax
{
public:
    StringTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue,
        const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING);

    string getValue(const Value& value, bool uniform) const override;
};

/// Syntax class for aggregate types.
class AggregateTypeSyntax : public TypeSyntax
{
public:
    AggregateTypeSyntax(const string& name, const string& defaultValue, const string& uniformDefaultValue,
        const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING, 
        const StringVec& members = EMPTY_MEMBERS);

    string getValue(const Value& value, bool uniform) const override;
    string getValue(const StringVec& values, bool uniform) const override;
};

} // namespace MaterialX

#endif
