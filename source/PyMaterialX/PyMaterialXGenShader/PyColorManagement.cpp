//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ColorManagementSystem.h>
#include <MaterialXGenShader/DefaultColorManagementSystem.h>
#if defined(MATERIALX_BUILD_OCIO)
#include <MaterialXGenShader/OCIOColorManagementSystem.h>
#endif
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
    mx::ImplementationPtr getImplementation(const mx::ColorSpaceTransform& transform) const override
    {
        PYBIND11_OVERLOAD_PURE(
            mx::ImplementationPtr,
            mx::ColorManagementSystem,
            getImplementation,
            transform
        );
    }

    void getPortConnections(mx::ShaderGraph* graph, mx::ShaderNode* colorTransformNode,
                            const mx::TypeDesc* targetType, mx::GenContext& context,
                            mx::ShaderInput*& inputToConnect, mx::ShaderOutput*& outputToConnect) override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ColorManagementSystem,
            getPortConnections,
            graph, colorTransformNode, targetType, context, inputToConnect, outputToConnect
        );
    }

};

void bindPyColorManagement(py::module& mod)
{
    py::class_<mx::ColorSpaceTransform>(mod, "ColorSpaceTransform")
        .def(py::init<const std::string&, const std::string&, const mx::TypeDesc*>())
        .def_readwrite("sourceSpace", &mx::ColorSpaceTransform::sourceSpace)
        .def_readwrite("targetSpace", &mx::ColorSpaceTransform::targetSpace)
        .def_readwrite("type", &mx::ColorSpaceTransform::type);

    py::enum_<mx::ColorManagementSystem::ResourceType>(mod, "ResourceType")
        .value("UNIFORM", mx::ColorManagementSystem::ResourceType::UNIFORM)
        .value("TEXTURE1D", mx::ColorManagementSystem::ResourceType::TEXTURE1D)
        .value("TEXTURE2D", mx::ColorManagementSystem::ResourceType::TEXTURE2D)
        .value("TEXTURE3D", mx::ColorManagementSystem::ResourceType::TEXTURE3D)
        .export_values();

    py::class_<mx::ColorManagementSystem, PyColorManagementSystem, mx::ColorManagementSystemPtr>(mod, "ColorManagementSystem")
        .def(py::init<>())
        .def("getName", &mx::ColorManagementSystem::getName)
        .def("loadLibrary", &mx::ColorManagementSystem::loadLibrary)
        .def("supportsTransform", &mx::ColorManagementSystem::supportsTransform)
        .def("createNode", &mx::ColorManagementSystem::supportsTransform)
        .def("connectNodeToShaderInput", &mx::ColorManagementSystem::supportsTransform)
        .def("connectNodeToShaderOutput", &mx::ColorManagementSystem::supportsTransform)
        .def("getResource", &mx::ColorManagementSystem::supportsTransform)
        .def("clearResources", &mx::ColorManagementSystem::clearResources);

    py::class_<mx::DefaultColorManagementSystem, mx::DefaultColorManagementSystemPtr, mx::ColorManagementSystem>(mod, "DefaultColorManagementSystem")
        .def_static("create", &mx::DefaultColorManagementSystem::create)
        .def("getName", &mx::DefaultColorManagementSystem::getName);

#if defined(MATERIALX_BUILD_OCIO)
    py::class_<mx::OCIOColorManagementSystem, mx::OCIOColorManagementSystemPtr, mx::ColorManagementSystem>(mod, "OCIOColorManagementSystem")
        .def_static("create", &mx::OCIOColorManagementSystem::create)
        .def("getName", &mx::OCIOColorManagementSystem::getName)
        .def("loadLibrary", &mx::OCIOColorManagementSystem::loadLibrary)
        .def("supportsTransform", &mx::OCIOColorManagementSystem::supportsTransform)
        .def("readConfigFile", &mx::OCIOColorManagementSystem::readConfigFile)
        .def("getConfigFile", &mx::OCIOColorManagementSystem::getConfigFile)
        .def("getResource", &mx::OCIOColorManagementSystem::getResource)
        .def("clearResources", &mx::OCIOColorManagementSystem::clearResources);
#endif
}
