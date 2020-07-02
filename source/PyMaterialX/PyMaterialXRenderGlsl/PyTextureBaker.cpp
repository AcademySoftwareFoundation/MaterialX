//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderGlsl/TextureBaker.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyTextureBaker(py::module& mod)
{
    py::class_<mx::TextureBaker, mx::GlslRenderer, mx::TextureBakerPtr>(mod, "TextureBaker")
        .def_static("create", &mx::TextureBaker::create);
        //.def(py::init<>());
        //.def_static("create", &mx::TextureBaker::create);
        //.def_property("_extension", &mx::TextureBaker::getExtension, &mx::TextureBaker::setExtension)
        //.def("bakeShaderInputs", (void (mx::TextureBaker::*)(mx::ShaderRefPtr, mx::GenContext&, const mx::FilePath&)) &mx::TextureBaker::bakeShaderInputs)
        //.def("bakeShaderInputs", (void (mx::TextureBaker::*)(mx::NodePtr, mx::GenContext&, const mx::FilePath&)) &mx::TextureBaker::bakeShaderInputs)
        //.def("bakeGraphOutput", &mx::TextureBaker::bakeGraphOutput);
        //.def("writeBakedDocument", (void(mx::TextureBaker::*)(mx::ShaderRefPtr, const mx::FilePath&)) &mx::TextureBaker::writeBakedDocument)
        //.def("writeBakedDocument", (void(mx::TextureBaker::*)(mx::NodePtr, const mx::FilePath&)) &mx::TextureBaker::writeBakedDocument);
}