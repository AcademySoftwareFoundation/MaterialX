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
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;
    string err;
    bool load = tinyobj::LoadObj(&attrib, &shapes, &materials, nullptr, &err,
                                 filePath.asString().c_str(), nullptr, true, false);
    if (!load)
    {
        std::cerr << err << std::endl;
        return false;
    }

    size_t vertComponentCount = std::max(attrib.vertices.size(), attrib.normals.size());
    size_t vertexCount = vertComponentCount / MeshStream::STRIDE_3D;
    if (!vertexCount)
    {
        return false;
    }

    MeshPtr mesh = Mesh::create(filePath);
    meshList.push_back(mesh);
    mesh->setSourceUri(filePath);
    MeshStreamPtr positionStream = MeshStream::create("i_" + MeshStream::POSITION_ATTRIBUTE, MeshStream::POSITION_ATTRIBUTE, 0);
    MeshFloatBuffer& positions = positionStream->getData();
    mesh->addStream(positionStream);

    MeshStreamPtr normalStream = MeshStream::create("i_" + MeshStream::NORMAL_ATTRIBUTE, MeshStream::NORMAL_ATTRIBUTE, 0);
    MeshFloatBuffer& normals = normalStream->getData();
    mesh->addStream(normalStream);

    MeshStreamPtr texCoordStream = MeshStream::create("i_" + MeshStream::TEXCOORD_ATTRIBUTE + "_0", MeshStream::TEXCOORD_ATTRIBUTE, 0);
    texCoordStream->setStride(MeshStream::STRIDE_2D);
    MeshFloatBuffer& texcoords = texCoordStream->getData();
    mesh->addStream(texCoordStream);

    MeshStreamPtr tangentStream = MeshStream::create("i_" + MeshStream::TANGENT_ATTRIBUTE, MeshStream::TANGENT_ATTRIBUTE, 0);
    tangentStream->setStride(MeshStream::STRIDE_3D);
    MeshFloatBuffer& tangents = tangentStream->getData();
    mesh->addStream(tangentStream);

    // Explode the geometry, since we may have unshared geometry
    // in position, normal or uv
    size_t totalIndexCount = 0;
    for (const tinyobj::shape_t& shape : shapes)
    {
        totalIndexCount += shape.mesh.indices.size();
    }
    positions.resize(totalIndexCount * MeshStream::STRIDE_3D);
    normals.resize(totalIndexCount * MeshStream::STRIDE_3D);
    texcoords.resize(totalIndexCount * MeshStream::STRIDE_2D);
    tangents.resize(totalIndexCount * MeshStream::STRIDE_3D);
    mesh->setVertexCount(totalIndexCount);

    const float MAX_FLOAT = std::numeric_limits<float>::max();
    Vector3 boxMin = { MAX_FLOAT, MAX_FLOAT, MAX_FLOAT };
    Vector3 boxMax = { -MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT };

    int writeIndex0 = 0;
    int writeIndex1 = 1;
    int writeIndex2 = 2;

    const size_t FACE_VERTEX_COUNT = 3;
    for (const tinyobj::shape_t& shape : shapes)
    {
        size_t indexCount = shape.mesh.indices.size();
        if (indexCount == 0)
        {
            continue;
        }
        size_t faceCount = indexCount / FACE_VERTEX_COUNT;

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
 
            // Copy indices.
            indices[faceIndex * MeshStream::STRIDE_3D + 0] = writeIndex0;
            indices[faceIndex * MeshStream::STRIDE_3D + 1] = writeIndex1;
            indices[faceIndex * MeshStream::STRIDE_3D + 2] = writeIndex2;

            // Copy positions and compute bounding box.
            Vector3 v[MeshStream::STRIDE_3D];
            for (unsigned int k = 0; k < MeshStream::STRIDE_3D; k++)
            {
                v[0][k] = attrib.vertices[indexObj0.vertex_index * MeshStream::STRIDE_3D + k];
                v[1][k] = attrib.vertices[indexObj1.vertex_index * MeshStream::STRIDE_3D + k];
                v[2][k] = attrib.vertices[indexObj2.vertex_index * MeshStream::STRIDE_3D + k];

                boxMin[k] = std::min(v[0][k], boxMin[k]);
                boxMin[k] = std::min(v[1][k], boxMin[k]);
                boxMin[k] = std::min(v[2][k], boxMin[k]);

                boxMax[k] = std::max(v[0][k], boxMax[k]);
                boxMax[k] = std::max(v[1][k], boxMax[k]);
                boxMax[k] = std::max(v[2][k], boxMax[k]);
            }

            Vector3* pos = reinterpret_cast<Vector3*>(&(positions[writeIndex0 * MeshStream::STRIDE_3D]));
            *pos = v[0];
            pos = reinterpret_cast<Vector3*>(&(positions[writeIndex1 * MeshStream::STRIDE_3D]));
            *pos = v[1];
            pos = reinterpret_cast<Vector3*>(&(positions[writeIndex2 * MeshStream::STRIDE_3D]));
            *pos = v[2];

            // Copy or compute normals
            Vector3 n[3];
            if (indexObj0.normal_index >= 0 &&
                indexObj1.normal_index >= 0 &&
                indexObj2.normal_index >= 0)
            {
                for (int k = 0; k < 3; k++)
                {
                    n[0][k] = attrib.normals[indexObj0.normal_index * MeshStream::STRIDE_3D + k];
                    n[1][k] = attrib.normals[indexObj1.normal_index * MeshStream::STRIDE_3D + k];
                    n[2][k] = attrib.normals[indexObj2.normal_index * MeshStream::STRIDE_3D + k];
                }
            }
            else
            {
                Vector3 faceNorm = (v[1] - v[0]).cross(v[2] - v[0]).getNormalized();
                n[0] = faceNorm;
                n[1] = faceNorm;
                n[2] = faceNorm;
            }

            Vector3* norm = reinterpret_cast<Vector3*>(&(normals[writeIndex0 * MeshStream::STRIDE_3D]));
            *norm = n[0];
            norm = reinterpret_cast<Vector3*>(&(normals[writeIndex1 * MeshStream::STRIDE_3D]));
            *norm = n[1];
            norm = reinterpret_cast<Vector3*>(&(normals[writeIndex2 * MeshStream::STRIDE_3D]));
            *norm = n[2];

            // Copy texture coordinates.
            Vector2 t[3];
            if (indexObj0.texcoord_index >= 0 &&
                indexObj1.texcoord_index >= 0 &&
                indexObj2.texcoord_index >= 0)
            {
                for (int k = 0; k < 2; k++)
                {
                    t[0][k] = attrib.texcoords[indexObj0.texcoord_index * MeshStream::STRIDE_2D + k];
                    t[1][k] = attrib.texcoords[indexObj1.texcoord_index * MeshStream::STRIDE_2D + k];
                    t[2][k] = attrib.texcoords[indexObj2.texcoord_index * MeshStream::STRIDE_2D + k];
                }
            }

            Vector2* texcoord = reinterpret_cast<Vector2*>(&(texcoords[writeIndex0 * MeshStream::STRIDE_2D]));
            *texcoord = t[0];
            texcoord = reinterpret_cast<Vector2*>(&(texcoords[writeIndex1 * MeshStream::STRIDE_2D]));
            *texcoord = t[1];
            texcoord = reinterpret_cast<Vector2*>(&(texcoords[writeIndex2 * MeshStream::STRIDE_2D]));
            *texcoord = t[2];

            writeIndex0 += MeshStream::STRIDE_3D;
            writeIndex1 += MeshStream::STRIDE_3D;
            writeIndex2 += MeshStream::STRIDE_3D;
        }
    }

    mesh->setMinimumBounds(boxMin);
    mesh->setMaximumBounds(boxMax);
    Vector3 sphereCenter = (boxMax + boxMin) * 0.5;
    mesh->setSphereCenter(sphereCenter);
    mesh->setSphereRadius((sphereCenter - boxMin).getMagnitude());

    MeshStreamPtr bitangentStream = MeshStream::create("i_" + MeshStream::BITANGENT_ATTRIBUTE, MeshStream::BITANGENT_ATTRIBUTE, 0);
    mesh->generateTangents(positionStream, texCoordStream, normalStream, tangentStream, bitangentStream);
    mesh->addStream(bitangentStream);

    return true;
}

} // namespace MaterialX
