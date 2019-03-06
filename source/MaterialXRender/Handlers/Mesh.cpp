//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/Handlers/Mesh.h>

namespace MaterialX
{
const string MeshStream::POSITION_ATTRIBUTE("position");
const string MeshStream::NORMAL_ATTRIBUTE("normal");
const string MeshStream::TEXCOORD_ATTRIBUTE("texcoord");
const string MeshStream::TANGENT_ATTRIBUTE("tangent");
const string MeshStream::BITANGENT_ATTRIBUTE("bitangent");
const string MeshStream::COLOR_ATTRIBUTE("color");
const string MeshStream::GEOMETRY_PROPERTY_ATTRIBUTE("geomprop");

const float MAX_FLOAT = std::numeric_limits<float>::max();

Mesh::Mesh(const string& identifier) :
    _identifier(identifier),
    _minimumBounds(MAX_FLOAT, MAX_FLOAT, MAX_FLOAT),
    _maximumBounds(-MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT),
    _sphereCenter(0.0f, 0.0f, 0.0f),
    _sphereRadius(0.0f),
    _vertexCount(0)
{
}

bool Mesh::generateTangents(MeshStreamPtr positionStream, MeshStreamPtr texcoordStream, MeshStreamPtr normalStream,
                            MeshStreamPtr tangentStream, MeshStreamPtr bitangentStream)
{
    MeshFloatBuffer& positions = positionStream->getData();
    unsigned int positionStride = positionStream->getStride();
    MeshFloatBuffer& texcoords = texcoordStream->getData();
    unsigned int texcoordStride = texcoordStream->getStride();
    MeshFloatBuffer& normals = normalStream->getData();
    unsigned int normalStride = normalStream->getStride();

    size_t vertexCount = positions.size() / positionStride;
    size_t uvCount = texcoords.size() / texcoordStride;
    size_t normalCount = normals.size() / normalStride;
    if (vertexCount != uvCount ||
        vertexCount != normalCount)
    {
        return false;
    }

    // Prepare tangent stream data
    MeshFloatBuffer& tangents = tangentStream->getData();
    tangents.resize(positions.size());
    std::fill(tangents.begin(), tangents.end(), 0.0f);
    const unsigned int tangentStride = 3;
    tangentStream->setStride(tangentStride);

    for (size_t i = 0; i < getPartitionCount(); i++)
    {
        auto part = getPartition(i);

        // Based on Eric Lengyel at http://www.terathon.com/code/tangent.html

        const MeshIndexBuffer& indicies = part->getIndices();
        for (size_t faceIndex = 0; faceIndex < part->getFaceCount(); faceIndex++)
        {
            int i1 = indicies[faceIndex * 3 + 0];
            int i2 = indicies[faceIndex * 3 + 1];
            int i3 = indicies[faceIndex * 3 + 2];

            Vector3& v1 = *reinterpret_cast<Vector3*>(&(positions[i1 * positionStride]));
            Vector3& v2 = *reinterpret_cast<Vector3*>(&(positions[i2 * positionStride]));
            Vector3& v3 = *reinterpret_cast<Vector3*>(&(positions[i3 * positionStride]));

            Vector2& w1 = *reinterpret_cast<Vector2*>(&(texcoords[i1 * texcoordStride]));
            Vector2& w2 = *reinterpret_cast<Vector2*>(&(texcoords[i2 * texcoordStride]));
            Vector2& w3 = *reinterpret_cast<Vector2*>(&(texcoords[i3 * texcoordStride]));

            float x1 = v2[0] - v1[0];
            float x2 = v3[0] - v1[0];
            float y1 = v2[1] - v1[1];
            float y2 = v3[1] - v1[1];
            float z1 = v2[2] - v1[2];
            float z2 = v3[2] - v1[2];

            float s1 = w2[0] - w1[0];
            float s2 = w3[0] - w1[0];
            float t1 = w2[1] - w1[1];
            float t2 = w3[1] - w1[1];

            float denom = s1 * t2 - s2 * t1;
            float r = denom ? 1.0f / denom : 0.0f;
            Vector3 dir((t2 * x1 - t1 * x2) * r,
                (t2 * y1 - t1 * y2) * r,
                (t2 * z1 - t1 * z2) * r);

            Vector3& tan1 = *reinterpret_cast<Vector3*>(&(tangents[i1 * tangentStride]));
            Vector3& tan2 = *reinterpret_cast<Vector3*>(&(tangents[i2 * tangentStride]));
            Vector3& tan3 = *reinterpret_cast<Vector3*>(&(tangents[i3 * tangentStride]));
            tan1 += dir;
            tan2 += dir;
            tan3 += dir;
        }
    }

    // Prepare bitangent stream data
    MeshFloatBuffer* bitangents = nullptr;
    unsigned int bitangentStride = 0;
    if (bitangentStream)
    {
        bitangents = &(bitangentStream->getData());
        bitangents->resize(positions.size());
        std::fill(bitangents->begin(), bitangents->end(), 0.0f);
        bitangentStride = bitangentStream->getStride();
    }

    for (size_t v = 0; v < vertexCount; v++)
    {
        Vector3& n = *reinterpret_cast<Vector3*>(&(normals[v * normalStride]));
        Vector3& t = *reinterpret_cast<Vector3*>(&(tangents[v * tangentStride]));
        Vector3* b = bitangents ? reinterpret_cast<Vector3*>(&((*bitangents)[v * bitangentStride])) : nullptr;

        // Gram-Schmidt orthogonalize
        if (t != Vector3(0.0f))
        {
            t = (t - n * n.dot(t)).getNormalized();
            if (b)
            {
                *b = n.cross(t);
            }
        }
    }
    return true;
}

}