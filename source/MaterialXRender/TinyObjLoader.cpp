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
#include <map>

namespace MaterialX
{

namespace {

const float MAX_FLOAT = std::numeric_limits<float>::max();
const size_t FACE_VERTEX_COUNT = 3;

} // anonymous namespace

//
// TinyObjLoader methods
//

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
    if (!attrib.vertices.size())
    {
        return false;
    }

    MeshPtr mesh = Mesh::create(filePath);
    meshList.push_back(mesh);
    mesh->setSourceUri(filePath);

    MeshStreamPtr positionStream = MeshStream::create("i_" + MeshStream::POSITION_ATTRIBUTE, MeshStream::POSITION_ATTRIBUTE, 0);
    MeshStreamPtr normalStream = MeshStream::create("i_" + MeshStream::NORMAL_ATTRIBUTE, MeshStream::NORMAL_ATTRIBUTE, 0);
    MeshStreamPtr texcoordStream = MeshStream::create("i_" + MeshStream::TEXCOORD_ATTRIBUTE + "_0", MeshStream::TEXCOORD_ATTRIBUTE, 0);
    texcoordStream->setStride(MeshStream::STRIDE_2D);

    using VertexTuple = std::tuple<Vector3, Vector3, Vector2>;
    using VertexIndexMap = std::map<VertexTuple, uint32_t>;
    VertexIndexMap vertexIndexMap;

    Vector3 boxMin = { MAX_FLOAT, MAX_FLOAT, MAX_FLOAT };
    Vector3 boxMax = { -MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT };

    uint32_t nextVertexIndex = 0;
    bool normalsFound = false;
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
        part->setFaceCount(faceCount);
        mesh->addPartition(part);

        MeshIndexBuffer& indices = part->getIndices();
        indices.resize(indexCount);

        for (size_t i = 0; i < shape.mesh.indices.size(); i++)
        {
            const tinyobj::index_t& indexObj = shape.mesh.indices[i];

            // Read vertex components.
            Vector3 position, normal;
            Vector2 texcoord;
            for (unsigned int k = 0; k < MeshStream::STRIDE_3D; k++)
            {
                position[k] = attrib.vertices[indexObj.vertex_index * MeshStream::STRIDE_3D + k];
                if (indexObj.normal_index >= 0)
                {
                    normal[k] = attrib.normals[indexObj.normal_index * MeshStream::STRIDE_3D + k];
                    normalsFound = true;
                }
                if (indexObj.texcoord_index >= 0 && k < MeshStream::STRIDE_2D)
                {
                    texcoord[k] = attrib.texcoords[indexObj.texcoord_index * MeshStream::STRIDE_2D + k];
                }
            }

            // Check for duplicate vertices.
            VertexTuple tuple(position, normal, texcoord);
            VertexIndexMap::iterator it = vertexIndexMap.find(tuple);
            if (it != vertexIndexMap.end())
            {
                indices[i] = it->second;
                continue;
            }
            vertexIndexMap[tuple] = nextVertexIndex;

            // Store vertex components.
            for (unsigned int k = 0; k < MeshStream::STRIDE_3D; k++)
            {
                positionStream->getData().push_back(position[k]);
                normalStream->getData().push_back(normal[k]);
                if (k < MeshStream::STRIDE_2D)
                {
                    texcoordStream->getData().push_back(texcoord[k]);
                }

                // Update bounds.
                boxMin[k] = std::min(position[k], boxMin[k]);
                boxMax[k] = std::max(position[k], boxMax[k]);
            }

            // Store index data.
            indices[i] = nextVertexIndex++;
        }
    }

    // Generate normals if needed.
    if (!normalsFound)
    {
        normalStream = mesh->generateNormals(positionStream);
    }

    // Generate tangents.
    MeshStreamPtr tangentStream = mesh->generateTangents(positionStream, normalStream, texcoordStream);

    // Assign streams to mesh.
    mesh->addStream(positionStream);
    mesh->addStream(normalStream);
    mesh->addStream(texcoordStream);
    if (tangentStream)
    {
        mesh->addStream(tangentStream);
    }

    // Assign properties to mesh.
    mesh->setVertexCount(positionStream->getData().size() / MeshStream::STRIDE_3D);
    mesh->setMinimumBounds(boxMin);
    mesh->setMaximumBounds(boxMax);
    Vector3 sphereCenter = (boxMax + boxMin) * 0.5;
    mesh->setSphereCenter(sphereCenter);
    mesh->setSphereRadius((sphereCenter - boxMin).getMagnitude());

    return true;
}

} // namespace MaterialX
