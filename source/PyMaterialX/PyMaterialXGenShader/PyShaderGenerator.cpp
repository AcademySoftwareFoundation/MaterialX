//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

class PyShaderGenerator : public mx::ShaderGenerator
{
  public:
    explicit PyShaderGenerator(mx::SyntaxPtr syntax) :
        mx::ShaderGenerator(syntax)
    {
    }

    const std::string& getLanguage() const override
    {
        PYBIND11_OVERLOAD_PURE(
            const std::string&,
            mx::ShaderGenerator,
            getLanguage
        );
    }

    const std::string& getTarget() const override
    {
        PYBIND11_OVERLOAD_PURE(
            const std::string&,
            mx::ShaderGenerator,
            getTarget
        );
    }

    mx::ShaderPtr generate(const std::string& shaderName, mx::ElementPtr element, const mx::GenOptions& options) override
    {
        PYBIND11_OVERLOAD_PURE(
            mx::ShaderPtr,
            mx::ShaderGenerator,
            generate,
            shaderName,
            element,
            options
        );
    }

    void emitTypeDefinitions(mx::Shader& shader) override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitTypeDefinitions,
            shader
        );
    }

    void emitFunctionDefinitions(mx::Shader& shader) override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitFunctionDefinitions,
            shader
        );
    }

    void emitFunctionCalls(const mx::GenContext& context, mx::Shader& shader) override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitFunctionCalls,
            context,
            shader
        );
    }

    void emitFinalOutput(mx::Shader& shader) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitFinalOutput,
            shader
        );
    }

    void emitInput(const mx::GenContext& context, const mx::ShaderInput* input, mx::Shader& shader) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitInput,
            context,
            input,
            shader
        );
    }

    void getInput(const mx::GenContext& context, const mx::ShaderInput* input, std::string& result) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            getInput,
            context,
            input,
            result
        );
    }

    void emitOutput(const mx::GenContext& context, const mx::ShaderOutput* output, bool includeType, bool assignDefault, mx::Shader& shader) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitOutput,
            context,
            output,
            includeType,
            assignDefault,
            shader
        );
    }

    void emitVariableBlock(const mx::Shader::VariableBlock& block, const std::string& qualifier, const std::string& separator, mx::Shader& shader) override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitVariableBlock,
            block,
            qualifier,
            separator,
            shader
        );
    }

    void emitVariable(const mx::Shader::Variable& variable, const std::string& qualifier, mx::Shader& shader) override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            emitVariable,
            variable,
            qualifier,
            shader
        );
    }

    void addContextIDs(mx::ShaderNode* node) const override
    {
        PYBIND11_OVERLOAD(
            void,
            mx::ShaderGenerator,
            addContextIDs,
            node
        );
    }

    mx::ValuePtr remapEnumeration(const mx::ValueElementPtr& input, const mx::InterfaceElement& mappingElement, const mx::TypeDesc*& enumerationType) override
    {
        PYBIND11_OVERLOAD(
            mx::ValuePtr,
            mx::ShaderGenerator,
            remapEnumeration,
            input,
            mappingElement,
            enumerationType
        );
    }

    mx::ValuePtr remapEnumeration(const std::string& inputName, const std::string& inputValue, const std::string& inputType,
                                  const mx::InterfaceElement& mappingElement, const mx::TypeDesc*& enumerationType) override
    {
        PYBIND11_OVERLOAD(
            mx::ValuePtr,
            mx::ShaderGenerator,
            remapEnumeration,
            inputName,
            inputValue,
            inputType,
            mappingElement,
            enumerationType
        );
    }
};

void bindPyShaderGenerator(py::module& mod)
{
    py::class_<mx::ShaderGenerator, PyShaderGenerator, mx::ShaderGeneratorPtr>(mod, "ShaderGenerator")
        .def(py::init([](mx::SyntaxPtr syntax) { return new PyShaderGenerator(syntax); }))
        .def("getLanguage", &mx::ShaderGenerator::getLanguage)
        .def("getTarget", &mx::ShaderGenerator::getTarget)
        .def("generate", &mx::ShaderGenerator::generate)
        .def("setColorManagementSystem", &mx::ShaderGenerator::setColorManagementSystem)
        .def("getColorManagementSystem", &mx::ShaderGenerator::getColorManagementSystem)
        .def("registerSourceCodeSearchPath", &mx::ShaderGenerator::registerSourceCodeSearchPath);
}
