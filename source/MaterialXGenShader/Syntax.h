//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_SYNTAX_H
#define MATERIALX_SYNTAX_H

/// @file
/// Base class for syntax handling for shader generators

#include <MaterialXGenShader/Export.h>
#include <MaterialXGenShader/TypeDesc.h>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Library.h>
#include <MaterialXCore/Value.h>

MATERIALX_NAMESPACE_BEGIN

class Syntax;
class TypeSyntax;
class StructTypeSyntax;
class TypeDesc;
class ShaderPort;

/// Shared pointer to a Syntax
using SyntaxPtr = shared_ptr<Syntax>;
/// Shared pointer to a constant Syntax
using ConstSyntaxPtr = shared_ptr<const Syntax>;
/// Shared pointer to a TypeSyntax
using TypeSyntaxPtr = shared_ptr<TypeSyntax>;
/// Shared pointer to a StructTypeSyntax
using StructTypeSyntaxPtr = shared_ptr<StructTypeSyntax>;

/// Map holding identifier names and a counter for
/// creating unique names from them.
using IdentifierMap = std::unordered_map<string, size_t>;

/// @class Syntax
/// Base class for syntax objects used by shader generators
/// to emit code with correct syntax for each language.
class MX_GENSHADER_API Syntax
{
  public:
    /// Punctuation types
    enum Punctuation
    {
        PARENTHESES,
        CURLY_BRACKETS,
        SQUARE_BRACKETS,
        DOUBLE_SQUARE_BRACKETS
    };

  public:
    virtual ~Syntax() { }

    /// Register syntax handling for a data type.
    /// Required to be set for all supported data types.
    void registerTypeSyntax(TypeDesc type, TypeSyntaxPtr syntax);
    [[deprecated]] void registerTypeSyntax(const TypeDesc* type, TypeSyntaxPtr syntax) { registerTypeSyntax(*type, syntax); }

    /// Register names that are reserved words not to be used by a code generator when naming
    /// variables and functions. Keywords, types, built-in functions etc. should be
    /// added to this set. Multiple calls will add to the internal set of names.
    void registerReservedWords(const StringSet& names);

    /// Register a set string replacements for disallowed tokens
    /// for a code generator when naming variables and functions.
    /// Multiple calls will add to the internal set of tokens.
    void registerInvalidTokens(const StringMap& tokens);

    /// Returns a set of names that are reserved words for this language syntax.
    const StringSet& getReservedWords() const { return _reservedWords; }

    /// Returns a mapping from disallowed tokens to replacement strings for this language syntax.
    const StringMap& getInvalidTokens() const { return _invalidTokens; }

    /// Returns the type syntax object for a named type.
    /// Throws an exception if a type syntax is not defined for the given type.
    const TypeSyntax& getTypeSyntax(TypeDesc type) const;
    [[deprecated]] const TypeSyntax& getTypeSyntax(const TypeDesc* type) const { return getTypeSyntax(*type); }

    /// Returns an array of all registered type syntax objects
    const vector<TypeSyntaxPtr>& getTypeSyntaxes() const { return _typeSyntaxes; }

    /// Return a type description for the given type name.
    TypeDesc getType(const string& name) const { return _typeSystem->getType(name); }

    /// Returns the name syntax of the given type
    const string& getTypeName(TypeDesc type) const;
    [[deprecated]] const string& getTypeName(const TypeDesc* type) const { return getTypeName(*type); }

    /// Returns the type name in an output context
    virtual string getOutputTypeName(TypeDesc type) const;
    [[deprecated]] string getOutputTypeName(const TypeDesc* type) const { return getOutputTypeName(*type); }

    /// Returns a type alias for the given data type.
    /// If not used returns an empty string.
    const string& getTypeAlias(TypeDesc type) const;
    [[deprecated]] const string& getTypeAlias(const TypeDesc* type) const { return getTypeAlias(*type); }

    /// Returns a custom type definition if needed for the given data type.
    /// If not used returns an empty string.
    const string& getTypeDefinition(TypeDesc type) const;
    [[deprecated]] const string& getTypeDefinition(const TypeDesc* type) const { return getTypeDefinition(*type); }

    /// Returns the default value string for the given type
    const string& getDefaultValue(TypeDesc type, bool uniform = false) const;
    [[deprecated]] const string& getDefaultValue(const TypeDesc* type, bool uniform = false) const { return getDefaultValue(*type, uniform); }

    /// Returns the value string for a given type and value object
    virtual string getValue(TypeDesc type, const Value& value, bool uniform = false) const;
    [[deprecated]] string getValue(const TypeDesc* type, const Value& value, bool uniform = false) const { return getValue(*type, value, uniform); }

    /// Returns the value string for a given shader port object
    virtual string getValue(const ShaderPort* port, bool uniform = false) const;

    /// Returns a type qualifier to be used when declaring types for input variables.
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

    /// Return the file extension used for source code files in this language.
    virtual const string& getSourceFileExtension() const = 0;

    /// Return the array suffix to use for declaring an array type.
    virtual string getArrayTypeSuffix(TypeDesc, const Value&) const { return EMPTY_STRING; };
    [[deprecated]] string getArrayTypeSuffix(const TypeDesc* type, const Value& value) const { return getArrayTypeSuffix(*type, value); }

    /// Return the array suffix to use for declaring an array variable.
    virtual string getArrayVariableSuffix(TypeDesc type, const Value& value) const;
    [[deprecated]] string getArrayVariableSuffix(const TypeDesc* type, const Value& value) const { return getArrayVariableSuffix(*type, value); }

    /// Query if given type is supported in the syntax.
    /// By default all types are assumed to be supported.
    [[deprecated]] virtual bool typeSupported(const TypeDesc* type) const;

    /// Modify the given name string to remove any invalid characters or tokens.
    virtual void makeValidName(string& name) const;

    /// Make sure the given name is a unique identifier,
    /// updating it if needed to make it unique.
    virtual void makeIdentifier(string& name, IdentifierMap& identifiers) const;

    /// Create a unique identifier for the given variable name and type.
    /// The method is used for naming variables (inputs and outputs) in generated code.
    /// Derived classes can override this method to have a custom naming strategy.
    /// Default implementation adds a number suffix, or increases an existing number suffix,
    /// on the name string if there is a name collision.
    virtual string getVariableName(const string& name, TypeDesc type, IdentifierMap& identifiers) const;
    [[deprecated]] string getVariableName(const string& name, const TypeDesc* type, IdentifierMap& identifiers) const { return getVariableName(name, *type, identifiers); }

    /// Given an input specification attempt to remap this to an enumeration which is accepted by
    /// the shader generator. The enumeration may be converted to a different type than the input.
    /// @param value The value string to remap.
    /// @param type The type of the value to remap,
    /// @param enumNames Type enumeration names
    /// @param result Enumeration type and value (returned).
    /// @return Return true if the remapping was successful.
    virtual bool remapEnumeration(const string& value, TypeDesc type, const string& enumNames,
                                  std::pair<TypeDesc, ValuePtr>& result) const;

    // Create and return a type syntax for a struct type.
    virtual StructTypeSyntaxPtr createStructSyntax(const string& structTypeName, const string& defaultValue,
                                                   const string& uniformDefaultValue, const string& typeAlias,
                                                   const string& typeDefinition) const;

    /// Constants with commonly used strings.
    static const string NEWLINE;
    static const string SEMICOLON;
    static const string COMMA;

  protected:
    /// Protected constructor
    Syntax(TypeSystemPtr typeSystem);

    TypeSystemPtr _typeSystem;
    vector<TypeSyntaxPtr> _typeSyntaxes;
    std::unordered_map<TypeDesc, size_t, TypeDesc::Hasher> _typeSyntaxIndexByType;

    StringSet _reservedWords;
    StringMap _invalidTokens;

    static const string INDENTATION;
    static const string STRING_QUOTE;
    static const string INCLUDE_STATEMENT;
    static const string SINGLE_LINE_COMMENT;
    static const string BEGIN_MULTI_LINE_COMMENT;
    static const string END_MULTI_LINE_COMMENT;

    static const std::unordered_map<char, size_t> CHANNELS_MAPPING;
};

/// @class TypeSyntax
/// Base class for syntax handling of types.
class MX_GENSHADER_API TypeSyntax
{
  public:
    virtual ~TypeSyntax() { }

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
    /// The value is constructed from the given shader port object.
    virtual string getValue(const ShaderPort* port, bool uniform) const;

    /// Returns a value formatted according to this type syntax.
    /// The value is constructed from the given value object.
    virtual string getValue(const Value& value, bool uniform) const = 0;

  protected:
    /// Protected constructor
    TypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
               const string& typeAlias, const string& typeDefinition, const StringVec& members);

    const Syntax* _parent;       // parent syntax class
    string _name;                // type name
    string _defaultValue;        // default value syntax
    string _uniformDefaultValue; // default value syntax when assigned to uniforms
    string _typeAlias;           // type alias if needed in source code
    string _typeDefinition;      // custom type definition if needed in source code
    StringVec _members;          // syntax for member access

    static const StringVec EMPTY_MEMBERS;
};

/// Specialization of TypeSyntax for scalar types.
class MX_GENSHADER_API ScalarTypeSyntax : public TypeSyntax
{
  public:
    ScalarTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                     const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING);

    string getValue(const Value& value, bool uniform) const override;
};

/// Specialization of TypeSyntax for string types.
class MX_GENSHADER_API StringTypeSyntax : public ScalarTypeSyntax
{
  public:
    StringTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                     const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING);

    string getValue(const Value& value, bool uniform) const override;
};

/// Specialization of TypeSyntax for aggregate types.
class MX_GENSHADER_API AggregateTypeSyntax : public TypeSyntax
{
  public:
    AggregateTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                        const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING,
                        const StringVec& members = EMPTY_MEMBERS);

    string getValue(const Value& value, bool uniform) const override;
};

/// Specialization of TypeSyntax for struct types.
class MX_GENSHADER_API StructTypeSyntax : public TypeSyntax
{
  public:
    StructTypeSyntax(const Syntax* parent, const string& name, const string& defaultValue, const string& uniformDefaultValue,
                     const string& typeAlias = EMPTY_STRING, const string& typeDefinition = EMPTY_STRING,
                     const StringVec& members = EMPTY_MEMBERS);

    string getValue(const Value& value, bool uniform) const override;
};

MATERIALX_NAMESPACE_END

#endif
