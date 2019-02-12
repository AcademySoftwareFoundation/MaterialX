//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//
#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ColorManagementSystem.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

class PyColorManagementSystem : public mx::ColorManagementSystem
{
  public:
    explicit PyColorManagementSystem(const std::string& configFile) :
        mx::ColorManagementSystem(configFile)
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
    std::string getImplementationName(const mx::ColorSpaceTransform& transform) override
    {
        PYBIND11_OVERLOAD_PURE(
            std::string,
            mx::ColorManagementSystem,
            getImplementationName,
            transform
        );
    }
};

class PyDefaultColorManagementSystem : public mx::DefaultColorManagementSystem
{
  public:
    PyDefaultColorManagementSystem(const std::string& configFile) :
        mx::DefaultColorManagementSystem(configFile)
    {
    }

    const std::string& getName() const override
    {
        PYBIND11_OVERLOAD(
            const std::string&,
            mx::DefaultColorManagementSystem,
            getName
        );
    }
};

void bindPyColorManagement(py::module& mod)
{
    py::class_<mx::ColorSpaceTransform>(mod, "ColorSpaceTransform")
        .def(py::init([](const std::string& ss, const std::string& ts, const mx::TypeDesc* t) { return new mx::ColorSpaceTransform(ss, ts, t); }))
        .def_readwrite("sourceSpace", &mx::ColorSpaceTransform::sourceSpace)
        .def_readwrite("targetSpace", &mx::ColorSpaceTransform::targetSpace)
        .def_readwrite("type", &mx::ColorSpaceTransform::type);

    py::class_<mx::ColorManagementSystem, PyColorManagementSystem, mx::ColorManagementSystemPtr>(mod, "ColorManagementSystem")
        .def(py::init([](const std::string& configFile) { return new PyColorManagementSystem(configFile); }))
        .def("getName", &mx::ColorManagementSystem::getName)
        .def("getConfigFile", &mx::ColorManagementSystem::getConfigFile)
        .def("setConfigFile", &mx::ColorManagementSystem::setConfigFile)
        .def("loadLibrary", &mx::ColorManagementSystem::loadLibrary)
        .def("supportsTransform", &mx::ColorManagementSystem::supportsTransform)
        .def("createNode", &mx::ColorManagementSystem::createNode);

    py::class_<mx::DefaultColorManagementSystem, PyDefaultColorManagementSystem, mx::DefaultColorManagementSystemPtr>(mod, "DefaultColorManagementSystem")
        .def_static("create", &mx::DefaultColorManagementSystem::create)
        .def(py::init([](const std::string& configFile) { return new PyDefaultColorManagementSystem(configFile); }))
        .def("getName", &mx::DefaultColorManagementSystem::getName);
}
