//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>
#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenOsl/ArnoldShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

class PyShaderGenerator : public mx::ShaderGenerator
{
  public:
    PyShaderGenerator(mx::SyntaxPtr syntax) :
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

class PyGlslShaderGenerator : public mx::GlslShaderGenerator
{
  public:
    using GlslShaderGenerator::GlslShaderGenerator;

    mx::ShaderPtr generate(const std::string& shaderName, mx::ElementPtr element, const mx::GenOptions& options) override
    {
        PYBIND11_OVERLOAD(
            mx::ShaderPtr,
            mx::GlslShaderGenerator,
            generate,
            shaderName,
            element,
            options
        );
    }

    const std::string& getLanguage() const override
    {
        PYBIND11_OVERLOAD(
            const std::string&,
            mx::GlslShaderGenerator,
            getLanguage
        );
    }

    const std::string& getTarget() const override
    {
        PYBIND11_OVERLOAD(
            const std::string&,
            mx::GlslShaderGenerator,
            getTarget
        );
    }

    const std::string& getVersion() const override
    {
        PYBIND11_OVERLOAD(
            const std::string&,
            mx::GlslShaderGenerator,
            getVersion
        );
    }
};

class PyOgsFxShaderGenerator : public mx::OgsFxShaderGenerator
{
  public:
    using OgsFxShaderGenerator::OgsFxShaderGenerator;

    mx::ShaderPtr generate(const std::string& shaderName, mx::ElementPtr element, const mx::GenOptions& options) override
    {
        PYBIND11_OVERLOAD(
            mx::ShaderPtr,
            mx::OgsFxShaderGenerator,
            generate,
            shaderName,
            element,
            options
        );
    }

    const std::string& getTarget() const override
    {
        PYBIND11_OVERLOAD(
            const std::string&,
            mx::OgsFxShaderGenerator,
            getTarget
        );
    }
};

class PyOslShaderGenerator : public mx::OslShaderGenerator
{
  public:
    PyOslShaderGenerator() :
        mx::OslShaderGenerator()
    {
    }

    mx::ShaderPtr generate(const std::string& shaderName, mx::ElementPtr element, const mx::GenOptions& options) override
    {
        PYBIND11_OVERLOAD(
            mx::ShaderPtr,
            mx::OslShaderGenerator,
            generate,
            shaderName,
            element,
            options
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

    const std::string& getLanguage() const override
    {
        PYBIND11_OVERLOAD(
            const std::string&,
            mx::OslShaderGenerator,
            getLanguage
        );
    }
};

class PyArnoldShaderGenerator : public mx::ArnoldShaderGenerator
{
  public:
    using ArnoldShaderGenerator::ArnoldShaderGenerator;

    const std::string& getTarget() const override
    {
        PYBIND11_OVERLOAD_PURE(
            const std::string&,
            mx::ArnoldShaderGenerator,
            getTarget
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

    py::class_<mx::GlslShaderGenerator, mx::ShaderGenerator, PyGlslShaderGenerator, mx::GlslShaderGeneratorPtr>(mod, "GlslShaderGenerator")
        .def_static("create", &mx::GlslShaderGenerator::create)
        .def(py::init([](){ return new PyGlslShaderGenerator(); }))
        .def("generate", &mx::GlslShaderGenerator::generate)
        .def("getLanguage", &mx::GlslShaderGenerator::getLanguage)
        .def("getTarget", &mx::GlslShaderGenerator::getTarget)
        .def("getVersion", &mx::GlslShaderGenerator::getVersion);

    py::class_<mx::OgsFxShaderGenerator, mx::ShaderGenerator, PyOgsFxShaderGenerator, mx::OgsFxShaderGeneratorPtr>(mod, "OgsFxShaderGenerator")
        .def_static("create", &mx::OgsFxShaderGenerator::create)
        .def(py::init([](){ return new PyOgsFxShaderGenerator(); }))
        .def("generate", &mx::OgsFxShaderGenerator::generate)
        .def("getTarget", &mx::OgsFxShaderGenerator::getTarget);

    py::class_<mx::OslShaderGenerator, mx::ShaderGenerator, PyOslShaderGenerator, mx::OslShaderGeneratorPtr>(mod, "OslShaderGenerator")
        .def(py::init([](){ return new PyOslShaderGenerator(); }))
        .def("generate", &mx::OslShaderGenerator::generate)
        .def("getLanguage", &mx::OslShaderGenerator::getLanguage);

    py::class_<mx::ArnoldShaderGenerator, mx::OslShaderGenerator, PyArnoldShaderGenerator, mx::ArnoldShaderGeneratorPtr>(mod, "ArnoldShaderGenerator")
        .def_static("create", &mx::ArnoldShaderGenerator::create)
        .def(py::init([](){ return new PyArnoldShaderGenerator(); }))
        .def("getTarget", &mx::ArnoldShaderGenerator::getTarget);
}
