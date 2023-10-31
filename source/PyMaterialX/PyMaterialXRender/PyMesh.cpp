//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/Mesh.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyMesh(py::module& mod)
{
    py::class_<mx::MeshStream, mx::MeshStreamPtr>(mod, "MeshStream")

        .def_readonly_static("POSITION_ATTRIBUTE",
                             &mx::MeshStream::POSITION_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The stream attribute name for storing positions.

    :see: `getType()`
)docstring"))

        .def_readonly_static("NORMAL_ATTRIBUTE",
                             &mx::MeshStream::NORMAL_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The stream attribute name for storing normals.

    :see: `getType()`
)docstring"))

        .def_readonly_static("TEXCOORD_ATTRIBUTE",
                             &mx::MeshStream::TEXCOORD_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The stream attribute name for storing texture coordinates.

    :see: `getType()`
)docstring"))

        .def_readonly_static("TANGENT_ATTRIBUTE",
                             &mx::MeshStream::TANGENT_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The stream attribute name for storing tangents.

    :see: `getType()`
)docstring"))

        .def_readonly_static("BITANGENT_ATTRIBUTE",
                             &mx::MeshStream::BITANGENT_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The stream attribute name for storing bitangents.

    :see: `getType()`
)docstring"))

        .def_readonly_static("COLOR_ATTRIBUTE",
                             &mx::MeshStream::COLOR_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The stream attribute name for storing colors.

    :see: `getType()`
)docstring"))

        .def_readonly_static("GEOMETRY_PROPERTY_ATTRIBUTE",
                             &mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE,
                             PYMATERIALX_DOCSTRING(R"docstring(
    The stream attribute name for storing geometry properties.

    :see: `getType()`
)docstring"))

        .def_static("create", &mx::MeshStream::create,
                    py::arg("name"),
                    py::arg("attributeName"),
                    py::arg("index") = 0,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class, initialized using the given name, stream
    attribute name, and stream index.
)docstring"))

        .def(py::init<const std::string&, const std::string&, unsigned int>(),
             py::arg("name"),
             py::arg("attributeName"),
             py::arg("index"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class using the given name, stream attribute
    name, and stream index.
)docstring"))

        .def("reserve", &mx::MeshStream::reserve,
             py::arg("elementCount"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Reserve memory for a given number of elements.
)docstring"))

        .def("resize", &mx::MeshStream::resize,
             py::arg("elementCount"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Resize data to a given number of elements.
)docstring"))

        .def("getName", &mx::MeshStream::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return stream name.
)docstring"))

        .def("getType", &mx::MeshStream::getType,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return stream attribute name.
)docstring"))

        .def("getIndex", &mx::MeshStream::getIndex,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return stream index.
)docstring"))

        .def("getData",
             static_cast<mx::MeshFloatBuffer& (mx::MeshStream::*)()>(&mx::MeshStream::getData),
             py::return_value_policy::reference,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the raw float data.
)docstring"))

        .def("getStride", &mx::MeshStream::getStride,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return stride between elements.
)docstring"))

        .def("setStride", &mx::MeshStream::setStride,
             py::arg("stride"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set stride between elements.
)docstring"))

        .def("getSize", &mx::MeshStream::getSize,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the number of elements.
)docstring"))

        .def("transform", &mx::MeshStream::transform,
             py::arg("matrix"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Transform elements by a matrix.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class to represent a mesh data stream.

    :see: https://materialx.org/docs/api/class_mesh_stream.html
)docstring");

    py::class_<mx::MeshPartition, mx::MeshPartitionPtr>(mod, "MeshPartition")
        .def_static("create", &mx::MeshPartition::create,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create a new mesh partition.
)docstring"))

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .def("resize", &mx::MeshPartition::resize,
             py::arg("indexCount"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Resize data to the given number of indices.
)docstring"))

        .def("setName", &mx::MeshPartition::setName,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the name of this partition.
)docstring"))

        .def("getName", &mx::MeshPartition::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the name of this partition.
)docstring"))

        .def("addSourceName", &mx::MeshPartition::addSourceName,
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a source name, representing a partition that was processed to generate
    this one.
)docstring"))

        .def("getSourceNames", &mx::MeshPartition::getSourceNames,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the set of source names, representing all partitions that were
    processed to generate this one.
)docstring"))

        .def("getIndices",
             static_cast<mx::MeshIndexBuffer& (mx::MeshPartition::*)()>(&mx::MeshPartition::getIndices),
             py::return_value_policy::reference,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return indexing.
)docstring"))

        .def("getFaceCount", &mx::MeshPartition::getFaceCount,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return number of faces.
)docstring"))

        .def("setFaceCount", &mx::MeshPartition::setFaceCount,
             py::arg("faceCount"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set number of faces.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class that describes a sub-region of a mesh using vertex indexing.

    Note that a face is considered to be a triangle.

    :see: https://materialx.org/docs/api/class_mesh_partition.html
)docstring");

    py::class_<mx::Mesh, mx::MeshPtr>(mod, "Mesh")

        .def_static("create", &mx::Mesh::create,
                    py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create a new mesh using the given `name`.
)docstring"))

        .def(py::init<const std::string&>(),
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class using the given `name`.
)docstring"))

        .def("getName", &mx::Mesh::getName,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the name of this mesh.
)docstring"))

        .def("setSourceUri", &mx::Mesh::setSourceUri,
             py::arg("sourceUri"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the mesh's source URI.
)docstring"))

        .def("hasSourceUri", &mx::Mesh::hasSourceUri,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if this mesh has a source URI.
)docstring"))

        .def("getSourceUri", &mx::Mesh::getSourceUri,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the mesh's source URI.
)docstring"))

        .def("getStream",
             static_cast<mx::MeshStreamPtr (mx::Mesh::*)(const std::string&) const>(&mx::Mesh::getStream),
             py::arg("name"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a mesh stream by name.

    :param name: Name of stream.
    :type name: str
    :returns: A mesh stream if found, otherwise `None`.
)docstring"))

        .def("getStream",
             static_cast<mx::MeshStreamPtr (mx::Mesh::*)(const std::string&, unsigned int) const> (&mx::Mesh::getStream),
             py::arg("streamType"),
             py::arg("index"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a mesh stream by type and index.

    :param streamType: Type of stream.
    :type streamType: str
    :param index: Index of stream.
    :type index: int
    :returns: A mesh stream if found, otherwise `None`.
)docstring"))

        .def("addStream", &mx::Mesh::addStream,
             py::arg("stream"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a mesh stream.
)docstring"))

        .def("setVertexCount", &mx::Mesh::setVertexCount,
             py::arg("vertexCount"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set vertex count.
)docstring"))

        .def("getVertexCount", &mx::Mesh::getVertexCount,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return vertex count.
)docstring"))

        .def("setMinimumBounds", &mx::Mesh::setMinimumBounds,
             py::arg("minimumBounds"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the minimum bounds for the geometry.
)docstring"))

        .def("getMinimumBounds", &mx::Mesh::getMinimumBounds,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the minimum bounds for the geometry.
)docstring"))

        .def("setMaximumBounds", &mx::Mesh::setMaximumBounds,
             py::arg("maximumBounds"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the maximum bounds for the geometry.
)docstring"))

        .def("getMaximumBounds", &mx::Mesh::getMaximumBounds,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the maximum bounds for the geometry.
)docstring"))

        .def("setSphereCenter", &mx::Mesh::setSphereCenter,
             py::arg("sphereCenter"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set center of the bounding sphere.
)docstring"))

        .def("getSphereCenter", &mx::Mesh::getSphereCenter,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return center of the bounding sphere.
)docstring"))

        .def("setSphereRadius", &mx::Mesh::setSphereRadius,
             py::arg("sphereRadius"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set radius of the bounding sphere.
)docstring"))

        .def("getSphereRadius", &mx::Mesh::getSphereRadius,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return radius of the bounding sphere.
)docstring"))

        .def("getPartitionCount", &mx::Mesh::getPartitionCount,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the number of mesh partitions.
)docstring"))

        .def("addPartition", &mx::Mesh::addPartition,
             py::arg("partition"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Add a partition.
)docstring"))

        .def("getPartition", &mx::Mesh::getPartition,
             py::arg("partIndex"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a mesh partition by index.
)docstring"))

        .def("generateTextureCoordinates",
             &mx::Mesh::generateTextureCoordinates,
             py::arg("positionStream"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create texture coordinates from the given positions.
    The texture coordinates are all initialize to a zero value.

    :param positionStream: Input position stream.
    :type positionStream: MeshStream
    :returns: The generated texture coordinate stream.
)docstring"))

        .def("generateNormals", &mx::Mesh::generateNormals,
             py::arg("positionStream"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Generate face normals from the given positions.

    :param positionStream: Input position stream.
    :type positionStream: MeshStream
    :returns: The generated normal stream
)docstring"))

        .def("generateTangents", &mx::Mesh::generateTangents,
             py::arg("positionStream"),
             py::arg("normalStream"),
             py::arg("texcoordStream"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Generate tangents from the given positions, normals, and texture coordinates.

    :param positionStream: Input position stream.
    :type positionStream: MeshStream
    :param normalStream: Input normal stream.
    :type normalStream: MeshStream
    :param texcoordStream: Input texcoord stream.
    :type texcoordStream: MeshStream
    :returns: The generated tangent stream, on success; otherwise, `None`.
)docstring"))

        .def("generateBitangents", &mx::Mesh::generateBitangents,
             py::arg("normalStream"),
             py::arg("tangentStream"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Generate bitangents from the given normals and tangents.

    :param normalStream: Input normal stream.
    :type normalStream: MeshStream
    :param tangentStream: Input tangent stream.
    :type tangentStream: MeshStream
    :returns: The generated bitangent stream, on success; otherwise, `None`.
)docstring"))

        .def("mergePartitions", &mx::Mesh::mergePartitions,
             PYMATERIALX_DOCSTRING(R"docstring(
    Merge all mesh partitions into one.
)docstring"))

        .def("splitByUdims", &mx::Mesh::splitByUdims,
             PYMATERIALX_DOCSTRING(R"docstring(
    Split the mesh into a single partition per UDIM.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a container for mesh data.

    :see: https://materialx.org/docs/api/class_mesh.html
)docstring");
}
