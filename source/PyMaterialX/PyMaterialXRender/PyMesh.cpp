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
    py::class_<mx::MeshStream, mx::MeshStreamPtr>(mod, "MeshStream", "Class to represent a mesh data stream.")
        .def_readonly_static("POSITION_ATTRIBUTE", &mx::MeshStream::POSITION_ATTRIBUTE)
        .def_readonly_static("NORMAL_ATTRIBUTE", &mx::MeshStream::NORMAL_ATTRIBUTE)
        .def_readonly_static("TEXCOORD_ATTRIBUTE", &mx::MeshStream::TEXCOORD_ATTRIBUTE)
        .def_readonly_static("TANGENT_ATTRIBUTE", &mx::MeshStream::TANGENT_ATTRIBUTE)
        .def_readonly_static("BITANGENT_ATTRIBUTE", &mx::MeshStream::BITANGENT_ATTRIBUTE)
        .def_readonly_static("COLOR_ATTRIBUTE", &mx::MeshStream::COLOR_ATTRIBUTE)
        .def_readonly_static("GEOMETRY_PROPERTY_ATTRIBUTE", &mx::MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE)
        .def_static("create", &mx::MeshStream::create)
        .def(py::init<const std::string&, const std::string&, unsigned int>())
        .def("reserve", &mx::MeshStream::reserve, "Reserve memory for a given number of elements.")
        .def("resize", &mx::MeshStream::resize, "Resize data to the given number of indices.")
        .def("getName", &mx::MeshStream::getName, "Return the ColorManagementSystem name.")
        .def("getType", &mx::MeshStream::getType, "Get stream attribute name.")
        .def("getIndex", &mx::MeshStream::getIndex, "Return the index string of this element.")
        .def("getData", static_cast<mx::MeshFloatBuffer& (mx::MeshStream::*)()>(&mx::MeshStream::getData), py::return_value_policy::reference, "Return the raw float vector.")
        .def("getStride", &mx::MeshStream::getStride, "Get stride between elements.")
        .def("setStride", &mx::MeshStream::setStride, "Set stride between elements.")
        .def("getSize", &mx::MeshStream::getSize, "Get the number of elements.")
        .def("transform", &mx::MeshStream::transform, "Transform elements by a matrix.");

    py::class_<mx::MeshPartition, mx::MeshPartitionPtr>(mod, "MeshPartition", "Class that describes a sub-region of a mesh using vertex indexing.\n\nNote that a face is considered to be a triangle.")
        .def_static("create", &mx::MeshPartition::create)
        .def(py::init<>())
        .def("resize", &mx::MeshPartition::resize, "Resize data to the given number of indices.")
        .def("setName", &mx::MeshPartition::setName, "Set the element's name string.")
        .def("getName", &mx::MeshPartition::getName, "Return the ColorManagementSystem name.")
        .def("addSourceName", &mx::MeshPartition::addSourceName, "Add a source name, representing a partition that was processed to generate this one.")
        .def("getSourceNames", &mx::MeshPartition::getSourceNames, "Return the vector of source names, representing all partitions that were processed to generate this one.")
        .def("getIndices", static_cast<mx::MeshIndexBuffer& (mx::MeshPartition::*)()>(&mx::MeshPartition::getIndices), py::return_value_policy::reference, "Return indexing.")
        .def("getFaceCount", &mx::MeshPartition::getFaceCount, "Return number of faces.")
        .def("setFaceCount", &mx::MeshPartition::setFaceCount, "Set face count.");

    py::class_<mx::Mesh, mx::MeshPtr>(mod, "Mesh", "Container for mesh data.")
        .def_static("create", &mx::Mesh::create)
        .def(py::init<const std::string&>())
        .def("getName", &mx::Mesh::getName, "Return the ColorManagementSystem name.")
        .def("setSourceUri", &mx::Mesh::setSourceUri, "Set the element's source URI.\n\nArgs:\n    sourceUri: A URI string representing the resource from which this element originates. This string may be used by serialization and deserialization routines to maintain hierarchies of include references.")
        .def("hasSourceUri", &mx::Mesh::hasSourceUri, "Return true if this element has a source URI.")
        .def("getSourceUri", &mx::Mesh::getSourceUri, "Return the element's source URI.")
        .def("getStream", static_cast<mx::MeshStreamPtr (mx::Mesh::*)(const std::string&) const>(&mx::Mesh::getStream), "Get a mesh stream by type and index.\n\nArgs:\n    type: Type of stream\n    index: Index of stream\n\nReturns:\n    Reference to a mesh stream if found")
        .def("getStream", static_cast<mx::MeshStreamPtr (mx::Mesh::*)(const std::string&, unsigned int) const> (&mx::Mesh::getStream), "Get a mesh stream by type and index.\n\nArgs:\n    type: Type of stream\n    index: Index of stream\n\nReturns:\n    Reference to a mesh stream if found")
        .def("addStream", &mx::Mesh::addStream, "Add a mesh stream.")
        .def("setVertexCount", &mx::Mesh::setVertexCount, "Set vertex count.")
        .def("getVertexCount", &mx::Mesh::getVertexCount, "Get vertex count.")
        .def("setMinimumBounds", &mx::Mesh::setMinimumBounds, "Set the minimum bounds for the geometry.")
        .def("getMinimumBounds", &mx::Mesh::getMinimumBounds, "Return the minimum bounds for all meshes.")
        .def("setMaximumBounds", &mx::Mesh::setMaximumBounds, "Set the minimum bounds for the geometry.")
        .def("getMaximumBounds", &mx::Mesh::getMaximumBounds, "Return the minimum bounds for all meshes.")
        .def("setSphereCenter", &mx::Mesh::setSphereCenter, "Set center of the bounding sphere.")
        .def("getSphereCenter", &mx::Mesh::getSphereCenter, "Return center of the bounding sphere.")
        .def("setSphereRadius", &mx::Mesh::setSphereRadius, "Set radius of the bounding sphere.")
        .def("getSphereRadius", &mx::Mesh::getSphereRadius, "Return radius of the bounding sphere.")
        .def("getPartitionCount", &mx::Mesh::getPartitionCount, "Return the number of mesh partitions.")
        .def("addPartition", &mx::Mesh::addPartition, "Add a partition.")
        .def("getPartition", &mx::Mesh::getPartition, "Return a reference to a mesh partition.")
        .def("generateTextureCoordinates", &mx::Mesh::generateTextureCoordinates, "Create texture coordinates from the given positions.\n\nArgs:\n    positionStream: Input position stream\n\nReturns:\n    The generated texture coordinate stream")
        .def("generateNormals", &mx::Mesh::generateNormals, "Generate face normals from the given positions.\n\nArgs:\n    positionStream: Input position stream\n\nReturns:\n    The generated normal stream")
        .def("generateTangents", &mx::Mesh::generateTangents, "Generate tangents from the given positions, normals, and texture coordinates.\n\nArgs:\n    positionStream: Input position stream\n    normalStream: Input normal stream\n    texcoordStream: Input texcoord stream\n\nReturns:\n    The generated tangent stream, on success; otherwise, a null pointer.")
        .def("generateBitangents", &mx::Mesh::generateBitangents, "Generate bitangents from the given normals and tangents.\n\nArgs:\n    normalStream: Input normal stream\n    tangentStream: Input tangent stream\n\nReturns:\n    The generated bitangent stream, on success; otherwise, a null pointer.")
        .def("mergePartitions", &mx::Mesh::mergePartitions, "Merge all mesh partitions into one.")
        .def("splitByUdims", &mx::Mesh::splitByUdims, "Split the mesh into a single partition per UDIM.");
}
