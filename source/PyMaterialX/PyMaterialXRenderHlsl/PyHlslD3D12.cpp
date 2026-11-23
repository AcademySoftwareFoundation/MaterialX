//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderHlsl/HlslD3D12Renderer.h>
#include <MaterialXRenderHlsl/HlslRootSignature.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyHlslD3D12(py::module& mod)
{
    py::enum_<mx::HlslShaderVisibility>(mod, "HlslShaderVisibility")
        .value("All",    mx::HlslShaderVisibility::All)
        .value("Vertex", mx::HlslShaderVisibility::Vertex)
        .value("Pixel",  mx::HlslShaderVisibility::Pixel);

    py::enum_<mx::HlslRootParameterType>(mod, "HlslRootParameterType")
        .value("ConstantBuffer", mx::HlslRootParameterType::ConstantBuffer)
        .value("TextureTable",   mx::HlslRootParameterType::TextureTable)
        .value("SamplerTable",   mx::HlslRootParameterType::SamplerTable);

    py::class_<mx::HlslRootParameter>(mod, "HlslRootParameter")
        .def_readonly("type",          &mx::HlslRootParameter::type)
        .def_readonly("visibility",    &mx::HlslRootParameter::visibility)
        .def_readonly("registerBase",  &mx::HlslRootParameter::registerBase)
        .def_readonly("registerCount", &mx::HlslRootParameter::registerCount)
        .def_readonly("space",         &mx::HlslRootParameter::space)
        .def_readonly("name",          &mx::HlslRootParameter::name);

    py::class_<mx::HlslRootSignatureDesc>(mod, "HlslRootSignatureDesc")
        .def_static("create", &mx::HlslRootSignatureDesc::create)
        .def("getParameters",      &mx::HlslRootSignatureDesc::getParameters)
        .def("findConstantBuffer", &mx::HlslRootSignatureDesc::findConstantBuffer,
             py::arg("visibility"), py::arg("reg"), py::arg("space") = 0)
        .def("findTable",          &mx::HlslRootSignatureDesc::findTable,
             py::arg("type"), py::arg("visibility"), py::arg("space") = 0)
        .def("getRootCost",        &mx::HlslRootSignatureDesc::getRootCost);

    py::class_<mx::HlslD3D12Context, mx::HlslD3D12ContextPtr>(mod, "HlslD3D12Context")
        .def_static("create", &mx::HlslD3D12Context::create, py::arg("preferWarp") = false)
        .def("isHardware", &mx::HlslD3D12Context::isHardware);

    py::class_<mx::HlslD3D12Framebuffer, mx::HlslD3D12FramebufferPtr>(mod, "HlslD3D12Framebuffer")
        .def_static("create", &mx::HlslD3D12Framebuffer::create,
                    py::arg("context"), py::arg("width"), py::arg("height"),
                    py::arg("baseType") = mx::Image::BaseType::UINT8)
        .def("getWidth",      &mx::HlslD3D12Framebuffer::getWidth)
        .def("getHeight",     &mx::HlslD3D12Framebuffer::getHeight)
        .def("getBaseType",   &mx::HlslD3D12Framebuffer::getBaseType)
        .def("setEncodeSrgb", &mx::HlslD3D12Framebuffer::setEncodeSrgb)
        .def("getEncodeSrgb", &mx::HlslD3D12Framebuffer::getEncodeSrgb)
        .def("readColor",     &mx::HlslD3D12Framebuffer::readColor, py::arg("image") = nullptr);

    py::class_<mx::HlslD3D12TextureHandler, mx::ImageHandler, mx::HlslD3D12TextureHandlerPtr>(mod, "HlslD3D12TextureHandler")
        .def_static("create", &mx::HlslD3D12TextureHandler::create);

    py::class_<mx::HlslD3D12RenderState>(mod, "HlslD3D12RenderState")
        .def(py::init<>())
        .def_readwrite("blend",          &mx::HlslD3D12RenderState::blend)
        .def_readwrite("depthTest",      &mx::HlslD3D12RenderState::depthTest)
        .def_readwrite("depthWrite",     &mx::HlslD3D12RenderState::depthWrite)
        .def_readwrite("depthLessEqual", &mx::HlslD3D12RenderState::depthLessEqual)
        .def_readwrite("cullBackFaces",  &mx::HlslD3D12RenderState::cullBackFaces)
        .def_readwrite("wireframe",      &mx::HlslD3D12RenderState::wireframe);

    py::class_<mx::HlslD3D12Material, mx::HlslD3D12MaterialPtr> material(mod, "HlslD3D12Material");

    py::enum_<mx::HlslD3D12Material::Stage>(material, "Stage")
        .value("Vertex", mx::HlslD3D12Material::Stage::Vertex)
        .value("Pixel",  mx::HlslD3D12Material::Stage::Pixel);

    material.def_static("create", &mx::HlslD3D12Material::create)
        .def("getPixelBindings",     &mx::HlslD3D12Material::getPixelBindings)
        .def("getRootSignatureDesc", &mx::HlslD3D12Material::getRootSignatureDesc)
        .def("getVertexStride",      &mx::HlslD3D12Material::getVertexStride)
        .def("lookupVariableOffset", &mx::HlslD3D12Material::lookupVariableOffset)
        .def("setCbufferRange",
             [](mx::HlslD3D12Material& self, mx::HlslD3D12Material::Stage stage,
                const std::string& name, std::size_t offset, py::bytes data) {
                 const std::string s = data;
                 return self.setCbufferRange(stage, name, offset, s.data(), s.size());
             });

    py::class_<mx::HlslD3D12Renderer, mx::ShaderRenderer, mx::HlslD3D12RendererPtr>(mod, "HlslD3D12Renderer")
        .def_static("create", &mx::HlslD3D12Renderer::create,
                    py::arg("width") = 512,
                    py::arg("height") = 512,
                    py::arg("baseType") = mx::Image::BaseType::UINT8)
        .def("initialize",         &mx::HlslD3D12Renderer::initialize, py::arg("renderContextHandle") = nullptr)
        .def("setPreferWarp",      &mx::HlslD3D12Renderer::setPreferWarp)
        .def("getPreferWarp",      &mx::HlslD3D12Renderer::getPreferWarp)
        .def("setCompilerBackend", &mx::HlslD3D12Renderer::setCompilerBackend)
        .def("getCompilerBackend", &mx::HlslD3D12Renderer::getCompilerBackend)
        .def("getShaderModel",     &mx::HlslD3D12Renderer::getShaderModel)
        .def("createImageHandler", &mx::HlslD3D12Renderer::createImageHandler)
        .def("createProgram",
             static_cast<void (mx::HlslD3D12Renderer::*)(mx::ShaderPtr)>(&mx::HlslD3D12Renderer::createProgram))
        .def("createProgram",
             static_cast<void (mx::HlslD3D12Renderer::*)(const mx::HlslD3D12Renderer::StageMap&)>(&mx::HlslD3D12Renderer::createProgram))
        .def("validateInputs",     &mx::HlslD3D12Renderer::validateInputs)
        .def("setSize",            &mx::HlslD3D12Renderer::setSize)
        .def("render",             &mx::HlslD3D12Renderer::render)
        .def("renderTextureSpace", &mx::HlslD3D12Renderer::renderTextureSpace)
        .def("captureImage",       &mx::HlslD3D12Renderer::captureImage, py::arg("image") = nullptr)
        .def("setScreenColor",     &mx::HlslD3D12Renderer::setScreenColor)
        .def("getScreenColor",     &mx::HlslD3D12Renderer::getScreenColor)
        .def("setActiveMeshes",    &mx::HlslD3D12Renderer::setActiveMeshes)
        .def("setClearOnRender",   &mx::HlslD3D12Renderer::setClearOnRender)
        .def("setRenderState",     &mx::HlslD3D12Renderer::setRenderState)
        .def("getRenderState",     &mx::HlslD3D12Renderer::getRenderState)
        .def("bindImage",          &mx::HlslD3D12Renderer::bindImage,
             py::arg("uniformName"), py::arg("image"), py::arg("properties") = nullptr)
        .def("getContext",         &mx::HlslD3D12Renderer::getContext)
        .def("getFramebuffer",     &mx::HlslD3D12Renderer::getFramebuffer)
        .def("getProgram",         &mx::HlslD3D12Renderer::getProgram)
        .def("getMaterial",        &mx::HlslD3D12Renderer::getMaterial)
        .def("getTextureHandler",  &mx::HlslD3D12Renderer::getTextureHandler);
}
