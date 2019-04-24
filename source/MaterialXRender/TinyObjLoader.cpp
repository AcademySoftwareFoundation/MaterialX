//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/TinyObjLoader.h>
#include <MaterialXCore/Util.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#define TINYOBJLOADER_IMPLEMENTATION
#include <MaterialXRender/External/TinyObjLoader/tiny_obj_loader.h>
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <iostream>

namespace MaterialX
{
bool TinyObjLoader::load(const FilePath& filePath, MeshList& meshList)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    bool load = tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, &err,
                                 filePath.asString().c_str(), nullptr, true, false);
    if (!load)
    {
        std::cerr << err << std::endl;
        return false;
    }

    size_t vertComponentCount = std::max(attrib.vertices.size(), attrib.normals.size());
    size_t vertexCount = vertComponentCount / 3;
    if (!vertexCount)
    {
        return false;
    }

    MeshPtr mesh = Mesh::create(filePath);
    meshList.push_back(mesh);
    mesh->setVertexCount(vertexCount);
    mesh->setSourceUri(filePath);
    MeshStreamPtr positionStream = MeshStream::create("i_" + MeshStream::POSITION_ATTRIBUTE, MeshStream::POSITION_ATTRIBUTE, 0);
    MeshFloatBuffer& positions = positionStream->getData();
    mesh->addStream(positionStream);

    MeshStreamPtr normalStream = MeshStream::create("i_" + MeshStream::NORMAL_ATTRIBUTE, MeshStream::NORMAL_ATTRIBUTE, 0);
    MeshFloatBuffer& normals = normalStream->getData();
    mesh->addStream(normalStream);

    MeshStreamPtr texCoordStream = MeshStream::create("i_" + MeshStream::TEXCOORD_ATTRIBUTE + "_0", MeshStream::TEXCOORD_ATTRIBUTE, 0);
    texCoordStream->setStride(2);
    MeshFloatBuffer& texcoords = texCoordStream->getData();
    mesh->addStream(texCoordStream);

    MeshStreamPtr tangentStream = MeshStream::create("i_" + MeshStream::TANGENT_ATTRIBUTE, MeshStream::TANGENT_ATTRIBUTE, 0);
    tangentStream->setStride(3);
    MeshFloatBuffer& tangents = tangentStream->getData();
    mesh->addStream(tangentStream);

    positions.resize(vertexCount * 3);
    normals.resize(vertexCount * 3);
    texcoords.resize(vertexCount * 2);
    tangents.resize(vertexCount * 3);

    const float MAX_FLOAT = std::numeric_limits<float>::max();
    Vector3 boxMin = { MAX_FLOAT, MAX_FLOAT, MAX_FLOAT };
    Vector3 boxMax = { -MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT };

    for (const tinyobj::shape_t& shape : shapes)
    {
        size_t faceCount = shape.mesh.indices.size();
        if (faceCount == 0)
        {
            continue;
        }
        faceCount /= 3;

        MeshPartitionPtr part = MeshPartition::create();
        part->setIdentifier(shape.name);
        MeshIndexBuffer& indices = part->getIndices();
        indices.resize(shape.mesh.indices.size());
        part->setFaceCount(faceCount);
        mesh->addPartition(part);

        for (size_t faceIndex = 0; faceIndex < faceCount; faceIndex++)
        {
            const tinyobj::index_t& indexObj0 = shape.mesh.indices[faceIndex * 3 + 0];
            const tinyobj::index_t& indexObj1 = shape.mesh.indices[faceIndex * 3 + 1];
            const tinyobj::index_t& indexObj2 = shape.mesh.indices[faceIndex * 3 + 2];

            int writeIndex0, writeIndex1, writeIndex2;
            if (vertComponentCount == attrib.vertices.size())
            {
                writeIndex0 = indexObj0.vertex_index;
                writeIndex1 = indexObj1.vertex_index;
                writeIndex2 = indexObj2.vertex_index;
            }
            else
            {
                writeIndex0 = indexObj0.normal_index;
                writeIndex1 = indexObj1.normal_index;
                writeIndex2 = indexObj2.normal_index;
            }
  
            // Copy indices.
            indices[faceIndex * 3 + 0] = writeIndex0;
            indices[faceIndex * 3 + 1] = writeIndex1;
            indices[faceIndex * 3 + 2] = writeIndex2;

            // Copy positions and compute bounding box.
            Vector3 v[3];
            for (int k = 0; k < 3; k++)
            {
                v[0][k] = attrib.vertices[indexObj0.vertex_index * 3 + k];
                v[1][k] = attrib.vertices[indexObj1.vertex_index * 3 + k];
                v[2][k] = attrib.vertices[indexObj2.vertex_index * 3 + k];

                boxMin[k] = std::min(v[0][k], boxMin[k]);
                boxMin[k] = std::min(v[1][k], boxMin[k]);
                boxMin[k] = std::min(v[2][k], boxMin[k]);

                boxMax[k] = std::max(v[0][k], boxMax[k]);
                boxMax[k] = std::max(v[1][k], boxMax[k]);
                boxMax[k] = std::max(v[2][k], boxMax[k]);
            }

            // Copy or compute normals
            Vector3 n[3];
            if (indexObj0.normal_index >= 0 &&
                indexObj1.normal_index >= 0 &&
                indexObj2.normal_index >= 0)
            {
                for (int k = 0; k < 3; k++)
                {
                    n[0][k] = attrib.normals[indexObj0.normal_index * 3 + k];
                    n[1][k] = attrib.normals[indexObj1.normal_index * 3 + k];
                    n[2][k] = attrib.normals[indexObj2.normal_index * 3 + k];
                }
            }
            else
            {
                Vector3 faceNorm = (v[1] - v[0]).cross(v[2] - v[0]).getNormalized();
                n[0] = faceNorm;
                n[1] = faceNorm;
                n[2] = faceNorm;
            }

            // Copy texture coordinates.
            Vector2 t[3];
            if (indexObj0.texcoord_index >= 0 &&
                indexObj1.texcoord_index >= 0 &&
                indexObj2.texcoord_index >= 0)
            {
                for (int k = 0; k < 2; k++)
                {
                    t[0][k] = attrib.texcoords[indexObj0.texcoord_index * 2 + k];
                    t[1][k] = attrib.texcoords[indexObj1.texcoord_index * 2 + k];
                    t[2][k] = attrib.texcoords[indexObj2.texcoord_index * 2 + k];
                }
            }

            Vector3* pos = reinterpret_cast<Vector3*>(&(positions[writeIndex0 * 3]));
            *pos = v[0];
            pos = reinterpret_cast<Vector3*>(&(positions[writeIndex1 * 3]));
            *pos = v[1];
            pos = reinterpret_cast<Vector3*>(&(positions[writeIndex2 * 3]));
            *pos = v[2];

            Vector3* norm = reinterpret_cast<Vector3*>(&(normals[writeIndex0 * 3]));
            *norm = n[0];
            norm = reinterpret_cast<Vector3*>(&(normals[writeIndex1 * 3]));
            *norm = n[1];
            norm = reinterpret_cast<Vector3*>(&(normals[writeIndex2 * 3]));
            *norm = n[2];

            Vector2* texcoord = reinterpret_cast<Vector2*>(&(texcoords[writeIndex0 * 2]));
            *texcoord = t[0];
            texcoord = reinterpret_cast<Vector2*>(&(texcoords[writeIndex1 * 2]));
            *texcoord = t[1];
            texcoord = reinterpret_cast<Vector2*>(&(texcoords[writeIndex2 * 2]));
            *texcoord = t[2];
        }
    }

    mesh->setMinimumBounds(boxMin);
    mesh->setMaximumBounds(boxMax);
    Vector3 sphereCenter = (boxMax + boxMin) / 2.0;
    mesh->setSphereCenter(sphereCenter);
    mesh->setSphereRadius((sphereCenter - boxMin).getMagnitude());

    MeshStreamPtr bitangentStream = MeshStream::create("i_" + MeshStream::BITANGENT_ATTRIBUTE, MeshStream::BITANGENT_ATTRIBUTE, 0);
    mesh->generateTangents(positionStream, texCoordStream, normalStream, tangentStream, bitangentStream);
    mesh->addStream(bitangentStream);

    return true;
}

} // namespace MaterialX
