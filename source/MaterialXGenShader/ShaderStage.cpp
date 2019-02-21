#include <MaterialXGenShader/ShaderStage.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Value.h>

namespace MaterialX
{

/// Identifier for main shader stage.
const string MAIN_STAGE = "main";

Variable::Variable(const VariableBlock* block, const TypeDesc* type, const string& name, const string& semantic,
    ValuePtr value, const string& path)
    : _type(type)
    , _name(name)
    , _fullName(block && !block->getInstance().empty() ? block->getInstance() + "." + name : name)
    , _semantic(semantic)
    , _value(value)
    , _path(path)
    , _calculated(false)
{
}

Variable& VariableBlock::operator[](const string& name)
{
    auto it = _variableMap.find(name);
    if (it == _variableMap.end())
    {
        throw ExceptionShaderGenError("No variable named '" + name + "' exists for block '" + getName() + "'");
    }
    return *it->second;
}

const Variable& VariableBlock::operator[](const string& name) const
{
    return const_cast<VariableBlock*>(this)->operator[](name);
}

void VariableBlock::add(const TypeDesc* type, const string& name, const string& semantic,
                        ValuePtr value, const string& path)
{
    if (!_variableMap.count(name))
    {
        VariablePtr v = std::make_shared<Variable>(this, type, name, semantic, value, path);
        _variableMap[name] = v;
        _variableOrder.push_back(v.get());
    }
}


ShaderStage::ShaderStage(const string& name, ConstSyntaxPtr syntax)
    : _name(name)
    , _syntax(syntax)
    , _indentations(0)
    , _constants("Constants", "cn")
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

void ShaderStage::beginScope(ShaderStage::Brackets brackets)
{
    switch (brackets) {
    case Brackets::BRACES:
        beginLine();
        _code += "{\n";
        break;
    case Brackets::PARENTHESES:
        beginLine();
        _code += "(\n";
        break;
    case Brackets::SQUARES:
        beginLine();
        _code += "[\n";
        break;
    case Brackets::NONE:
        break;
    }

    ++_indentations;
    _scopes.push(brackets);
}

void ShaderStage::endScope(bool semicolon, bool newline)
{
    if (_scopes.empty())
    {
        throw ExceptionShaderGenError("End scope called with no scope active, please check your beginScope/endScope calls");
    }

    Brackets brackets = _scopes.back();
    _scopes.pop();
    --_indentations;

    switch (brackets) {
    case Brackets::BRACES:
        beginLine();
        _code += "}";
        break;
    case Brackets::PARENTHESES:
        beginLine();
        _code += ")";
        break;
    case Brackets::SQUARES:
        beginLine();
        _code += "]";
        break;
    case Brackets::NONE:
        break;
    }
    if (semicolon)
        _code += ";";
    if (newline)
        _code += "\n";
}

void ShaderStage::beginLine()
{
    // TODO: Call syntax class to get indentation string
    // Use 4 spaces as default indentation
    static const string& INDENTATION = "    ";
    for (int i = 0; i < _indentations; ++i)
    {
        _code += INDENTATION;
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
    _code += "\n";
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
    _code += "// " + str;
    endLine(false);
}

void ShaderStage::addBlock(const string& str, GenContext& context)
{
    static const string INCLUDE_PATTERN = "#include";
    static const string QUOTATION_MARK = "\"";

    // Add each line in the block seperatelly
    // to get correct indentation
    std::stringstream stream(str);
    for (string line; std::getline(stream, line); )
    {
        size_t pos = line.find(INCLUDE_PATTERN);
        if (pos != string::npos)
        {
            size_t startQuote = line.find_first_of(QUOTATION_MARK);
            size_t endQuote = line.find_last_of(QUOTATION_MARK);
            if (startQuote != string::npos && endQuote != string::npos)
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
    const string path = context.findSourceCode(file);

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

void ShaderStage::addFunctionDefinition(const ShaderNode& node, const ShaderGenerator& shadergen, GenContext& context)
{
    const ShaderNodeImpl& impl = node.getImplementation();
    const size_t id = impl.getHash();
    if (!_definedFunctions.count(id))
    {
        _definedFunctions.insert(id);
        impl.emitFunctionDefinition(*this, node, shadergen, context);
    }
}

}
