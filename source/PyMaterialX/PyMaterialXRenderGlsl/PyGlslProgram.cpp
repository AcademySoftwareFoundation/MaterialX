//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderGlsl/GlslProgram.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGlslProgram(py::module& mod)
{
    py::class_<mx::GlslProgram, mx::GlslProgramPtr>(mod, "GlslProgram")
        .def_readwrite_static("UNDEFINED_OPENGL_RESOURCE_ID", &mx::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
        .def_readwrite_static("UNDEFINED_OPENGL_PROGRAM_LOCATION", &mx::GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION)
        .def_static("create", &mx::GlslProgram::create)
        .def("setStages", &mx::GlslProgram::setStages)
        .def("addStage", &mx::GlslProgram::addStage)
        .def("getStageSourceCode", &mx::GlslProgram::getStageSourceCode)
        .def("clearStages", &mx::GlslProgram::clearStages)
        .def("build", &mx::GlslProgram::build)
        .def("getUniformsList", &mx::GlslProgram::getUniformsList)
        .def("getAttributesList", &mx::GlslProgram::getAttributesList)
        .def("findInputs", &mx::GlslProgram::findInputs)
        .def("bind", &mx::GlslProgram::bind)
        .def("bindInputs", &mx::GlslProgram::bindInputs)
        .def("unbindInputs", &mx::GlslProgram::unbindInputs)
        .def("haveActiveAttributes", &mx::GlslProgram::haveActiveAttributes)
        .def("bindUniform", &mx::GlslProgram::bindUniform)
        .def("bindAttribute", &mx::GlslProgram::bindAttribute)
        .def("bindPartition", &mx::GlslProgram::bindPartition)
        .def("bindStreams", &mx::GlslProgram::bindStreams)
        .def("unbindGeometry", &mx::GlslProgram::unbindGeometry)
        .def("bindTextures", &mx::GlslProgram::bindTextures)
        .def("unbindTextures", &mx::GlslProgram::unbindTextures)
        .def("bindLighting", &mx::GlslProgram::bindLighting)
        .def("bindViewInformation", &mx::GlslProgram::bindViewInformation)
        .def("bindTimeAndFrame", &mx::GlslProgram::bindTimeAndFrame)
        .def("unbind", &mx::GlslProgram::unbind);

    py::class_<mx::GlslProgram::Input>(mod, "Input")
        .def_readwrite_static("INVALID_OPENGL_TYPE", &mx::GlslProgram::Input::INVALID_OPENGL_TYPE)
        .def_readwrite("location", &mx::GlslProgram::Input::location)
        .def_readwrite("gltype", &mx::GlslProgram::Input::gltype)
        .def_readwrite("size", &mx::GlslProgram::Input::size)
        .def_readwrite("typeString", &mx::GlslProgram::Input::typeString)
        .def_readwrite("value", &mx::GlslProgram::Input::value)
        .def_readwrite("isConstant", &mx::GlslProgram::Input::isConstant)
        .def_readwrite("path", &mx::GlslProgram::Input::path)
        .def(py::init<int, int, int, std::string>());
}
