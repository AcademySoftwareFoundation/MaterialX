//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/ShaderStage.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Value.h>

namespace MaterialX
{

namespace Stage
{
    const string PIXEL = "pixel";
}

//
// VariableBlock methods
//

ShaderPort* VariableBlock::operator[](const string& name)
{
    ShaderPort* v = find(name);
    if (!v)
    {
        throw ExceptionShaderGenError("No variable named '" + name + "' exists for block '" + getName() + "'");
    }
    return v;
}

const ShaderPort* VariableBlock::operator[](const string& name) const
{
    return const_cast<VariableBlock*>(this)->operator[](name);
}

ShaderPort* VariableBlock::find(const string& name)
{
    auto it = _variableMap.find(name);
    return it != _variableMap.end() ? it->second.get() : nullptr;
}

const ShaderPort* VariableBlock::find(const string& name) const
{
    return const_cast<VariableBlock*>(this)->find(name);
}

ShaderPort* VariableBlock::add(const TypeDesc* type, const string& name, ValuePtr value)
{
    auto it = _variableMap.find(name);
    if (it != _variableMap.end())
    {
        return it->second.get();
    }

    ShaderPortPtr port = std::make_shared<ShaderPort>(nullptr, type, name, value);
    _variableMap[name] = port;
    _variableOrder.push_back(port.get());

    return port.get();
}

void VariableBlock::add(ShaderPortPtr port)
{
    if (!_variableMap.count(port->getName()))
    {
        _variableMap[port->getName()] = port;
        _variableOrder.push_back(port.get());
    }
}

//
// ShaderStage methods
//

ShaderStage::ShaderStage(const string& name, ConstSyntaxPtr syntax) :
    _name(name),
    _syntax(syntax),
    _indentations(0),
    _constants("Constants", "cn")
{
}

VariableBlockPtr ShaderStage::createUniformBlock(const string& name, const string& instance)
{
    auto it = _uniforms.find(name);
    if (it == _uniforms.end())
    {
        VariableBlockPtr b = std::make_shared<VariableBlock>(name, instance);
        _uniforms[name] = b;
        return b;
    }
    return it->second;
}

VariableBlockPtr ShaderStage::createInputBlock(const string& name, const string& instance)
{
    auto it = _inputs.find(name);
    if (it == _inputs.end())
    {
        VariableBlockPtr b = std::make_shared<VariableBlock>(name, instance);
        _inputs[name] = b;
        return b;
    }
    return it->second;
}

VariableBlockPtr ShaderStage::createOutputBlock(const string& name, const string& instance)
{
    auto it = _outputs.find(name);
    if (it == _outputs.end())
    {
        VariableBlockPtr b = std::make_shared<VariableBlock>(name, instance);
        _outputs[name] = b;
        return b;
    }
    return it->second;
}

VariableBlock& ShaderStage::getUniformBlock(const string& name)
{
    auto it = _uniforms.find(name);
    if (it == _uniforms.end())
    {
        throw ExceptionShaderGenError("No uniform block named '" + name + "' exists for shader stage '" + getName() + "'");
    }
    return *it->second;
}

const VariableBlock& ShaderStage::getUniformBlock(const string& name) const
{
    return const_cast<ShaderStage*>(this)->getUniformBlock(name);
}

VariableBlock& ShaderStage::getInputBlock(const string& name)
{
    auto it = _inputs.find(name);
    if (it == _inputs.end())
    {
        throw ExceptionShaderGenError("No input block named '" + name + "' exists for shader stage '" + getName() + "'");
    }
    return *it->second;
}

const VariableBlock& ShaderStage::getInputBlock(const string& name) const
{
    return const_cast<ShaderStage*>(this)->getInputBlock(name);
}

VariableBlock& ShaderStage::getOutputBlock(const string& name)
{
    auto it = _outputs.find(name);
    if (it == _outputs.end())
    {
        throw ExceptionShaderGenError("No output block named '" + name + "' exists for shader stage '" + getName() + "'");
    }
    return *it->second;
}

const VariableBlock& ShaderStage::getOutputBlock(const string& name) const
{
    return const_cast<ShaderStage*>(this)->getOutputBlock(name);
}

VariableBlock& ShaderStage::getConstantBlock()
{
    return _constants;
}

const VariableBlock& ShaderStage::getConstantBlock() const
{
    return _constants;
}

void ShaderStage::beginScope(Syntax::Punctuation punc)
{
    switch (punc) {
    case Syntax::CURLY_BRACKETS:
        beginLine();
        _code += "{" + _syntax->getNewline();
        break;
    case Syntax::PARENTHESES:
        beginLine();
        _code += "(" + _syntax->getNewline();
        break;
    case Syntax::SQUARE_BRACKETS:
        beginLine();
        _code += "[" + _syntax->getNewline();
        break;
    }

    ++_indentations;
    _scopes.push(punc);
}

void ShaderStage::endScope(bool semicolon, bool newline)
{
    if (_scopes.empty())
    {
        throw ExceptionShaderGenError("End scope called with no scope active, please check your beginScope/endScope calls");
    }

    Syntax::Punctuation punc = _scopes.back();
    _scopes.pop();
    --_indentations;

    switch (punc) {
    case Syntax::CURLY_BRACKETS:
        beginLine();
        _code += "}";
        break;
    case Syntax::PARENTHESES:
        beginLine();
        _code += ")";
        break;
    case Syntax::SQUARE_BRACKETS:
        beginLine();
        _code += "]";
        break;
    }
    if (semicolon)
        _code += ";";
    if (newline)
        _code += _syntax->getNewline();
}

void ShaderStage::beginLine()
{
    for (int i = 0; i < _indentations; ++i)
    {
        _code += _syntax->getIndentation();
    }
}

void ShaderStage::endLine(bool semicolon)
{
    if (semicolon)
    {
        _code += ";";
    }
    newLine();
}

void ShaderStage::newLine()
{
    _code += _syntax->getNewline();
}

void ShaderStage::addString(const string& str)
{
    _code += str;
}

void ShaderStage::addLine(const string& str, bool semicolon)
{
    beginLine();
    addString(str);
    endLine(semicolon);
}

void ShaderStage::addComment(const string& str)
{
    beginLine();
    _code += _syntax->getSingleLineComment() + str;
    endLine(false);
}

void ShaderStage::addBlock(const string& str, GenContext& context)
{
    const string& INCLUDE = _syntax->getIncludeStatement();
    const string& QUOTE   = _syntax->getStringQuote();

    // Add each line in the block seperatelly
    // to get correct indentation
    std::stringstream stream(str);
    for (string line; std::getline(stream, line); )
    {
        size_t pos = line.find(INCLUDE);
        if (pos != string::npos)
        {
            size_t startQuote = line.find_first_of(QUOTE);
            size_t endQuote = line.find_last_of(QUOTE);
            if (startQuote != string::npos && endQuote != string::npos && endQuote > startQuote)
            {
                size_t length = (endQuote - startQuote) - 1;
                if (length)
                {
                    const string filename = line.substr(startQuote + 1, length);
                    addInclude(filename, context);
                }
            }
        }
        else
        {
            addLine(line, false);
        }
    }
}

void ShaderStage::addInclude(const string& file, GenContext& context)
{
    const string path = context.resolveSourceFile(file);

    if (!_includes.count(path))
    {
        string content;
        if (!readFile(path, content))
        {
            throw ExceptionShaderGenError("Could not find include file: '" + file + "'");
        }
        _includes.insert(path);
        addBlock(content, context);
    }
}

void ShaderStage::addFunctionDefinition(const ShaderNode& node, GenContext& context)
{
    const ShaderNodeImpl& impl = node.getImplementation();
    const size_t id = impl.getHash();

    if (!_definedFunctions.count(id))
    {
        _definedFunctions.insert(id);
        impl.emitFunctionDefinition(node, context, *this);
    }
}

}
