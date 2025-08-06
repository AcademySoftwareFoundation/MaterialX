//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/DocumentHandler.h>

namespace py = pybind11;
namespace mx = MaterialX;

// Python wrapper class for document loader
class PyDocumentLoader : public mx::DocumentLoader
{
  public:
    using mx::DocumentLoader::DocumentLoader;

    mx::StringSet supportedExtensions() const override
    {
        PYBIND11_OVERRIDE(mx::StringSet, mx::DocumentLoader, supportedExtensions);
    }

    mx::DocumentPtr importDocument(const std::string& uri) override
    {
        PYBIND11_OVERRIDE(mx::DocumentPtr, mx::DocumentLoader, importDocument, uri);
    }

    bool exportDocument(mx::ConstDocumentPtr document, const std::string& uri) override
    {
        PYBIND11_OVERRIDE(bool, mx::DocumentLoader, exportDocument, document, uri);
    }
};

void bindPyDocumentHandler(py::module& mod)
{
    py::class_<mx::DocumentLoader, PyDocumentLoader, std::shared_ptr<mx::DocumentLoader>>(mod, "DocumentLoader")
        .def(py::init<const std::string&, const std::string&, const std::string&>(),
             py::arg("identifier"), py::arg("name"), py::arg("description"))
        .def("getIdentifier", &mx::DocumentLoader::getIdentifier)
        .def("getName", &mx::DocumentLoader::getName)
        .def("getDescription", &mx::DocumentLoader::getDescription)
        .def("supportedExtensions", &mx::DocumentLoader::supportedExtensions,
             py::return_value_policy::reference)
        .def("importDocument", &mx::DocumentLoader::importDocument,
             py::arg("uri"))
        .def("exportDocument", &mx::DocumentLoader::exportDocument,
             py::arg("document"), py::arg("uri"));

    py::class_<mx::DocumentHandler, std::shared_ptr<mx::DocumentHandler>>(mod, "DocumentHandler")
        .def_static("create", &mx::DocumentHandler::create)
        .def("registerLoader", &mx::DocumentHandler::registerLoader,
             py::arg("loader"))
        .def("unregisterLoader", &mx::DocumentHandler::unregisterLoader,
             py::arg("identifier"))
        .def("getLoader", &mx::DocumentHandler::getLoader,
             py::arg("identifier"))
        .def("importDocument", &mx::DocumentHandler::importDocument,
             py::arg("uri"))
        .def("exportDocument", &mx::DocumentHandler::exportDocument,
             py::arg("document"), py::arg("uri"))        .def("supportedExtensions", &mx::DocumentHandler::supportedExtensions);
}
