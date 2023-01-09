//
// TM & (c) 2022 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXFormat/GraphIo.h>

namespace py = pybind11;
namespace mx = MaterialX;

class PyGraphIo : public mx::GraphIo
{
  public:
    std::string write(mx::GraphElementPtr graph, const std::vector<mx::OutputPtr> roots) override
    {
        PYBIND11_OVERLOAD_PURE(
            std::string,
            mx::GraphIo,
            write,
            graph,
            roots
        );
    }
};

void bindPyGraphIo(py::module& mod)
{
    py::enum_<mx::NodeIO::NodeShape>(mod, "NodeShape")
        .value("BOX", mx::NodeIO::NodeShape::BOX)
        .value("ROUNDEDBOX", mx::NodeIO::NodeShape::ROUNDEDBOX)
        .value("DIAMOND", mx::NodeIO::NodeShape::DIAMOND)
        .export_values();

    py::enum_<mx::GraphIoGenOptions::Orientation>(mod, "GraphOrientation")
        .value("TOP_DOWN", mx::GraphIoGenOptions::Orientation::TOP_DOWN)
        .value("BOTTOM_UP", mx::GraphIoGenOptions::Orientation::BOTTOM_UP)
        .value("LEFT_RIGHT", mx::GraphIoGenOptions::Orientation::LEFT_RIGHT)
        .value("RIGHT_LEFT", mx::GraphIoGenOptions::Orientation::RIGHT_LEFT)
        .export_values();

    py::class_<mx::GraphIoGenOptions>(mod, "GraphIoGenOptions")
        .def("setWriteCategories", &mx::GraphIoGenOptions::setWriteCategories)
        .def("getWriteCategories", &mx::GraphIoGenOptions::getWriteCategories)
        .def("setWriteSubgraphs", &mx::GraphIoGenOptions::setWriteSubgraphs)
        .def("getWriteSubgraphs", &mx::GraphIoGenOptions::getWriteSubgraphs)
        .def("setOrientation", &mx::GraphIoGenOptions::setOrientation)
        .def("getOrientation", &mx::GraphIoGenOptions::getOrientation)
        .def(py::init<>());

    py::class_<mx::NodeIO>(mod, "NodeIO")
        .def_readwrite("identifier", &mx::NodeIO::identifier)
        .def_readwrite("uilabel", &mx::NodeIO::uilabel)
        .def_readwrite("category", &mx::NodeIO::category)
        .def_readwrite("group", &mx::NodeIO::group)
        .def_readwrite("uishape", &mx::NodeIO::uishape)
        .def(py::init<>());

    py::class_<mx::GraphIo, PyGraphIo, mx::GraphIoPtr>(mod, "GraphIo")
        .def("write", (std::string(mx::GraphIo::*)(mx::GraphElementPtr, const std::vector<mx::OutputPtr>)) & mx::GraphIo::write)
        .def("supportsFormats", &mx::GraphIo::supportsFormats)
        .def("setGenOptions", &mx::GraphIo::setGenOptions)
        .def("getGenOptions", &mx::GraphIo::getGenOptions);

    py::class_<mx::DotGraphIo, mx::GraphIo, mx::DotGraphIoPtr>(mod, "DotGraphIo")
        .def_static("create", &mx::DotGraphIo::create)
        .def("write", (std::string (mx::DotGraphIo::*)(mx::GraphElementPtr, const std::vector<mx::OutputPtr>)) &mx::DotGraphIo::write);

    py::class_<mx::MermaidGraphIo, mx::GraphIo, mx::MermaidGraphIoPtr>(mod, "MermaidGraphIo")
        .def_static("create", &mx::MermaidGraphIo::create)
        .def("write", (std::string (mx::MermaidGraphIo::*)(mx::GraphElementPtr, const std::vector<mx::OutputPtr>)) &mx::MermaidGraphIo::write);
}

