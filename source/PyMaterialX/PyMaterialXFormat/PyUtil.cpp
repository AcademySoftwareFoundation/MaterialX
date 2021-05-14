//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXCore/Node.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyUtil(py::module& mod)
{
    mod.def("readFile", &mx::readFile);
    mod.def("getSubdirectories", &mx::getSubdirectories);
    mod.def("loadDocuments", &mx::loadDocuments,
        py::arg("rootPath"), py::arg("searchPath"), py::arg("skipFiles"), py::arg("includeFiles"), py::arg("documents"), py::arg("documentsPaths"),
        py::arg("readOptions") = (mx::XmlReadOptions*) nullptr, py::arg("errors") = (mx::StringVec*) nullptr);
    mod.def("loadLibrary", &mx::loadLibrary,
        py::arg("file"), py::arg("doc"), py::arg("searchPath") = mx::FileSearchPath(), py::arg("readOptions") = (mx::XmlReadOptions*) nullptr);
    mod.def("loadLibraries", &mx::loadLibraries,
        py::arg("libraryFolders"), py::arg("searchPath"), py::arg("doc"), py::arg("excludeFiles") = mx::StringSet(), py::arg("readOptions") = (mx::XmlReadOptions*) nullptr);
    mod.def("loadCoreLibraries", &mx::loadCoreLibraries,
        py::arg("libraryFolders"), py::arg("searchPath"), py::arg("doc"), py::arg("excludeFiles") = mx::StringSet(), py::arg("readOptions") = (mx::XmlReadOptions*) nullptr);
    mod.def("flattenFilenames", &mx::flattenFilenames,
        py::arg("doc"), py::arg("searchPath") = mx::FileSearchPath(), py::arg("customResolver") = (mx::StringResolverPtr) nullptr);

    mod.def("getResolvedDefinitionPath", &mx::getResolvedDefinitionPath,
        py::arg("userDefinitionPath"), py::arg("includeSubFolders") = true);
    mod.def("getResolvedTexturePath", &mx::getResolvedTexturePath,
        py::arg("userTexturePath"), py::arg("userDefinitionPath"), py::arg("includeSubFolders") = true);

    mod.def("getEnvironmentPath", &mx::getEnvironmentPath,
        py::arg("sep") = mx::PATH_LIST_SEPARATOR);
    mod.def("getAssetDefinitionPath", &mx::getAssetDefinitionPath,
        py::arg("sep") = mx::PATH_LIST_SEPARATOR);
    mod.def("getAssetTexturePath", &mx::getAssetTexturePath,
        py::arg("sep") = mx::PATH_LIST_SEPARATOR);
    mod.def("getCoreDefinitionPath", &mx::getCoreDefinitionPath);
    mod.def("setEnvironmentPath", &mx::setEnvironmentPath);
    mod.def("setAssetDefinitionPath", &mx::setAssetDefinitionPath);
    mod.def("setAssetTexturePath", &mx::setAssetTexturePath);
    mod.def("setCoreDefinitionPath", &mx::setCoreDefinitionPath);

    mod.attr("PATH_LIST_SEPARATOR") = mx::PATH_LIST_SEPARATOR;
    mod.attr("MATERIALX_SEARCH_PATH_ENV_VAR") = mx::MATERIALX_SEARCH_PATH_ENV_VAR;
    mod.attr("MATERIALX_ASSET_DEFINITION_PATH_ENV_VAR") = mx::MATERIALX_ASSET_DEFINITION_PATH_ENV_VAR;
    mod.attr("MATERIALX_ASSET_TEXTURE_PATH_ENV_VAR") = mx::MATERIALX_ASSET_TEXTURE_PATH_ENV_VAR;
}
