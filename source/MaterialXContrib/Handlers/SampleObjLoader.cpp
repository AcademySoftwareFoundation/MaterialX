//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXContrib/Handlers/SampleObjLoader.h>
#include <MaterialXCore/Util.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <numeric>
#include <iomanip>  
#include <limits>

namespace MaterialX
{ 
bool SampleObjLoader::load(const FilePath& filePath, MeshList& meshList)
{
    std::ifstream objfile;
    objfile.open(filePath);
    if (!objfile.is_open())
    {
        return false;
    }

    MeshPtr mesh = Mesh::create(filePath);
    mesh->setSourceUri(filePath);
    MeshStreamPtr positionStream = MeshStream::create("i_" + MeshStream::POSITION_ATTRIBUTE, MeshStream::POSITION_ATTRIBUTE, 0);
    MeshFloatBuffer& positionData = positionStream->getData();
    mesh->addStream(positionStream);

    MeshStreamPtr normalStream = MeshStream::create("i_" + MeshStream::NORMAL_ATTRIBUTE, MeshStream::NORMAL_ATTRIBUTE, 0);
    MeshFloatBuffer& normalData = normalStream->getData();
    mesh->addStream(normalStream);

    MeshStreamPtr texCoordStream = MeshStream::create("i_" + MeshStream::TEXCOORD_ATTRIBUTE + "_0", MeshStream::TEXCOORD_ATTRIBUTE, 0);
    texCoordStream->setStride(2);
    MeshFloatBuffer& texCoordData = texCoordStream->getData();
    mesh->addStream(texCoordStream);

    MeshStreamPtr texCoordStream2 = MeshStream::create("i_" + MeshStream::TEXCOORD_ATTRIBUTE + "_1", MeshStream::TEXCOORD_ATTRIBUTE, 1);
    texCoordStream2->setStride(2);
    MeshFloatBuffer& texCoordData2 = texCoordStream2->getData();
    mesh->addStream(texCoordStream2);

    // Extra color data
    MeshStreamPtr colorStream1 = MeshStream::create("i_" + MeshStream::COLOR_ATTRIBUTE + "_0", MeshStream::COLOR_ATTRIBUTE, 0);
    MeshFloatBuffer& colorData1 = colorStream1->getData();
    colorStream1->setStride(4);
    mesh->addStream(colorStream1);

    MeshStreamPtr colorStream2 = MeshStream::create("i_" + MeshStream::COLOR_ATTRIBUTE + "_1", MeshStream::COLOR_ATTRIBUTE, 1);
    MeshFloatBuffer& colorData2 = colorStream2->getData();
    colorStream2->setStride(4);
    mesh->addStream(colorStream2);

    MeshFloatBuffer pos;
    MeshFloatBuffer uv;
    MeshFloatBuffer norm;
    MeshIndexBuffer uvidx;
    MeshIndexBuffer nidx;

    std::vector<MeshPartitionPtr> partitions;
    MeshPartitionPtr currentPartition = nullptr;
    MeshIndexBuffer pidx;

    const float MAX_FLOAT = std::numeric_limits<float>::max();
    Vector3 minPos = { MAX_FLOAT , MAX_FLOAT , MAX_FLOAT };
    Vector3 maxPos = { -MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT };

    // Enable debugging of read by dumping to disk what was read in.
    // Disabled by default
    std::ofstream dump;
    if (_debugDump)
    {
        dump.open("dump.obj");
    }

    float val1, val2, val3;
    unsigned int ipos[4], iuv[4], inorm[4];
    std::string line;
    while (std::getline(objfile, line))
    {
        if (line.substr(0, 2) == "v ")
        {
            std::istringstream valstring(line.substr(2));
            valstring >> val1; valstring >> val2; valstring >> val3;

            if (_debugDump)
            {
                dump << "v " << val1 << " " << val2 << " " << val3 << std::endl;
            }

            // Y-flip
            val2 = -val2;

            // Update bounds
            if (val1 < minPos[0]) minPos[0] = val1;
            if (val2 < minPos[1]) minPos[1] = val2;
            if (val3 < minPos[2]) minPos[2] = val3;
            if (val1 > maxPos[0]) maxPos[0] = val1;
            if (val2 > maxPos[1]) maxPos[1] = val2;
            if (val3 > maxPos[2]) maxPos[2] = val3;

            pos.push_back(val1); pos.push_back(val2); pos.push_back(val3);
        }
        else if (line.substr(0, 3) == "vt ")
        {
            std::istringstream valstring(line.substr(3));
            valstring >> val1; valstring >> val2;
            uv.push_back(val1); uv.push_back(val2);

            if (_debugDump)
            {
                dump << "vt " << val1 << " " << val2 << std::endl;
            }
        }
        else if (line.substr(0, 3) == "vn ")
        {
            std::istringstream valstring(line.substr(3));
            valstring >> val1; valstring >> val2; valstring >> val3;
            norm.push_back(val1); norm.push_back(val2); norm.push_back(val3);

            if (_debugDump)
            {
                dump << "vn " << val1 << " " << val2 << " " << val3 << std::endl;
            }
        }
        else if (_readGroups && line.substr(0, 2) == "g ")
        {
            string name;
            string parent;
            std::istringstream valstring(line.substr(2));
            valstring >> name;

            currentPartition = MeshPartition::create();
            currentPartition->setName(name);
            partitions.push_back(currentPartition);
        }
        else if (line.substr(0, 2) == "f ")
        {
            // Extact out the component parts from face string
            //
            std::istringstream valstring(line.substr(2));
            std::string vertices[4];
            valstring >> vertices[0];
            valstring >> vertices[1];
            valstring >> vertices[2];
            valstring >> vertices[3];

            int vertexCount = 0;
            for (unsigned int i = 0; i < 4; i++)
            {
                if (vertices[i].size())
                {
                    StringVec vertexParts = MaterialX::splitString(vertices[i], "/");
                    if (vertexParts.size() == 3)
                    {
                        ipos[i] = std::stoi(vertexParts[0]);
                        iuv[i] = std::stoi(vertexParts[1]);
                        inorm[i] = std::stoi(vertexParts[2]);
                        vertexCount++;
                    }
                }
            }

            if (!currentPartition)
            {
                currentPartition = MeshPartition::create();
                currentPartition->setName("Partition" + std::to_string(mesh->getPartitionCount()));
                partitions.push_back(currentPartition);
            }
            
            if (vertexCount >= 3)
            {
                size_t facesAdded = 1;

                pidx.push_back(ipos[0] - 1);
                pidx.push_back(ipos[1] - 1);
                pidx.push_back(ipos[2] - 1);

                uvidx.push_back(iuv[0] - 1);
                uvidx.push_back(iuv[1] - 1);
                uvidx.push_back(iuv[2] - 1);

                nidx.push_back(inorm[0] - 1);
                nidx.push_back(inorm[1] - 1);
                nidx.push_back(inorm[2] - 1);

                if (_debugDump)
                {
                    dump << "f "
                        << ipos[0] << "/" << iuv[0] << "/" << inorm[0] << " "
                        << ipos[1] << "/" << iuv[1] << "/" << inorm[1] << " "
                        << ipos[2] << "/" << iuv[2] << "/" << inorm[2] << std::endl;
                }

                if (vertexCount >= 4)
                {
                    facesAdded++;

                    pidx.push_back(ipos[0] - 1);
                    pidx.push_back(ipos[2] - 1);
                    pidx.push_back(ipos[3] - 1);

                    uvidx.push_back(iuv[0] - 1);
                    uvidx.push_back(iuv[2] - 1);
                    uvidx.push_back(iuv[3] - 1);

                    nidx.push_back(inorm[0] - 1);
                    nidx.push_back(inorm[2] - 1);
                    nidx.push_back(inorm[3] - 1);

                    if (_debugDump)
                    {
                        dump << "f "
                            << ipos[0] << "/" << iuv[0] << "/" << inorm[0] << " "
                            << ipos[2] << "/" << iuv[2] << "/" << inorm[2] << " "
                            << ipos[3] << "/" << iuv[3] << "/" << inorm[3] << std::endl;
                    }
                }

                currentPartition->setFaceCount(currentPartition->getFaceCount() + facesAdded);
            }
        }
    }

    if (_debugDump)
    {
        dump.close();
    }
    objfile.close();

    // Set bounds
    mesh->setMinimumBounds(minPos);
    mesh->setMaximumBounds(maxPos);
    Vector3 sphereCenter = (maxPos + minPos) / 2.0;
    mesh->setSphereCenter(sphereCenter);
    mesh->setSphereRadius((sphereCenter - minPos).getMagnitude());

    // Organize data to get triangles for positions 
    for (unsigned int i = 0; i < pidx.size(); i++)
    {
        unsigned int vertexIndex = 3 * pidx[i];
        positionData.push_back(pos[vertexIndex]);
        positionData.push_back(pos[vertexIndex + 1]);
        positionData.push_back(pos[vertexIndex + 2]);
    }

    // Organize data to get triangles for texture coordinates 
    for (unsigned int i = 0; i < uvidx.size(); i++)
    {
        unsigned int vertexIndex = 2 * uvidx[i];
        texCoordData.push_back(uv[vertexIndex]);
        texCoordData.push_back(uv[vertexIndex + 1]);

        // Fake second set of texture coordinates
        texCoordData2.push_back(uv[vertexIndex + 1]);
        texCoordData2.push_back(uv[vertexIndex]);

        // Fake some colors
        colorData1.push_back(uv[vertexIndex]);
        colorData1.push_back(uv[vertexIndex] + 1);
        colorData1.push_back(1.0f);
        colorData1.push_back(1.0f);

        colorData2.push_back(1.0f);
        colorData2.push_back(uv[vertexIndex] + 1);
        colorData2.push_back(uv[vertexIndex]);
        colorData2.push_back(1.0f);
    }

    // Organize data to get triangles for normals 
    for (unsigned int i = 0; i < nidx.size(); i++)
    {
        unsigned int vertexIndex = 3 * nidx[i];
        normalData.push_back(norm[vertexIndex]);
        normalData.push_back(norm[vertexIndex + 1]);
        normalData.push_back(norm[vertexIndex + 2]);
    }

    // Set up flattened contiguous indexing for all partitions
    unsigned int startIndex = 0;
    for (const auto& partition : partitions)
    {
        MeshIndexBuffer& indexing = partition->getIndices();
        size_t indexCount = partition->getFaceCount() * 3;
        if (indexCount > 0)
        {
            indexing.resize(indexCount);
            std::iota(indexing.begin(), indexing.end(), startIndex);
            startIndex += (unsigned int)indexing.size();

            mesh->addPartition(partition);
        }
    }

    // Add tangent basis
    //
    MeshStreamPtr tangentStream = mesh->generateTangents(positionStream, normalStream, texCoordStream);
    mesh->addStream(tangentStream);

    mesh->setVertexCount(positionData.size() / 3);

    // Add in new mesh
    meshList.push_back(mesh);
    return true;
}

}