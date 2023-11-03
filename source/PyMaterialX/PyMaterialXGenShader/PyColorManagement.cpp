//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ColorManagementSystem.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/ShaderGraph.h>

namespace py = pybind11;
namespace mx = MaterialX;

class PyColorManagementSystem : public mx::ColorManagementSystem
{
  public:
    PyColorManagementSystem()
    {
    }

    const std::string& getName() const override
    {
        PYBIND11_OVERLOAD_PURE(
            const std::string&,
            mx::ColorManagementSystem,
            getName
        );
    }

  protected:
    mx::NodeDefPtr getNodeDef(const mx::ColorSpaceTransform& transform) const override
    {
        PYBIND11_OVERLOAD_PURE(
            mx::NodeDefPtr,
            mx::ColorManagementSystem,
            getNodeDef,
            transform
        );
    }
};

void bindPyColorManagement(py::module& mod)
{
    py::class_<mx::ColorSpaceTransform>(mod, "ColorSpaceTransform")

        .def(py::init<const std::string&, const std::string&, const mx::TypeDesc*>(),
             py::arg("sourceSpace"),
             py::arg("targetSpace"),
             py::arg("typeDesc"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class using the given source space, target
    space, and type descriptor.
)docstring"))

        .def_readwrite("sourceSpace", &mx::ColorSpaceTransform::sourceSpace,
                       PYMATERIALX_DOCSTRING(R"docstring(
    The source color space of the color space transform.
)docstring"))

        .def_readwrite("targetSpace", &mx::ColorSpaceTransform::targetSpace,
                       PYMATERIALX_DOCSTRING(R"docstring(
    The target color space of the color space transform.
)docstring"))

        .def_readwrite("typeDesc", &mx::ColorSpaceTransform::type,
                       PYMATERIALX_DOCSTRING(R"docstring(
    The type descriptor of the color space transform.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Structure that represents color space transform information.

    :see: https://materialx.org/docs/api/struct_color_space_transform.html
)docstring");

    py::class_<mx::ColorManagementSystem, PyColorManagementSystem, mx::ColorManagementSystemPtr>(mod, "ColorManagementSystem")

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .def("getName", &mx::ColorManagementSystem::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the name of the color management system.
)docstring"))

        .def("loadLibrary", &mx::ColorManagementSystem::loadLibrary,
             py::arg("document"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Load a library of implementations from the provided `document`,
    replacing any previously loaded content.
)docstring"))

        .def("supportsTransform", &mx::ColorManagementSystem::supportsTransform,
             py::arg("transform"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Returns whether this color management system supports the provided
    `transform`.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Abstract base class for color management systems.

    :see: https://materialx.org/docs/api/class_color_management_system.html
)docstring");

    py::class_<mx::DefaultColorManagementSystem, mx::DefaultColorManagementSystemPtr, mx::ColorManagementSystem>(mod, "DefaultColorManagementSystem")

        .def_static("create", &mx::DefaultColorManagementSystem::create,
                    py::arg("target"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a new `DefaultColorManagementSystem` instance.
)docstring"))

        .def("getName", &mx::DefaultColorManagementSystem::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the name of the default color management system.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class for a default color management system.

    :see: https://materialx.org/docs/api/class_default_color_management_system.html
)docstring");
}
