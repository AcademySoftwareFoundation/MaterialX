#include <MaterialXRender/Handlers/ObjGeometryHandler.h>
#include <MaterialXCore/Util.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <numeric>
#include <iomanip>  
#include <limits>

namespace MaterialX
{ 
ObjGeometryHandler::ObjGeometryHandler() :
    GeometryHandler()
{
}

ObjGeometryHandler::~ObjGeometryHandler()
{
}

void ObjGeometryHandler::clearData()
{
    _indexing.clear();
    _positionData.clear();
    _normalData.clear();
    _texcoordData[0].clear();
    _tangentData[0].clear();
    _bitangentData[0].clear();
    _colorData[0].clear();
    _texcoordData[1].clear();
    _tangentData[1].clear();
    _bitangentData[1].clear();
    _colorData[1].clear();
}


void ObjGeometryHandler::setQuadData()
{
    GeometryHandler::setIdentifier(UNIT_QUAD);

    // 2x2 quad centered around the origin in the X/Y plane which is
    // assumed to be the screen plane.
    _minimumBounds[0] = -0.5f;
    _minimumBounds[1] = -0.5f;
    _minimumBounds[2] = 0.0f;
    _maximumBounds[0] = 0.5f;
    _maximumBounds[1] = 0.5f;
    _maximumBounds[2] = 0.0f;

    _indexing = {
        0, 1, 2, 0, 2, 3
    };

    _positionData =
    {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
    };
    _normalData = {
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f
    };
    _texcoordData[0] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };
    _texcoordData[1] = {
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };
    _tangentData[0] = {
        .0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f
    };
    _tangentData[1] = {
        0.0f, -1.0f, 0.0f,
        .0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f
    };
    _bitangentData[0] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f
    };
    _bitangentData[1] = {
        0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        -1.0f, 0.0f, 0.0f
    };
    _colorData[0] = {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f
    };
    _colorData[1] = {
        1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.5f, 1.0f
    };
}

void ObjGeometryHandler::readData()
{
    // If data already exists dont' re-read
    // Data is cleared when a new identfier is set
    if (!_positionData.empty())
    {
        return;
    }

    FloatBuffer pos;
    FloatBuffer uv;
    FloatBuffer norm;
    IndexBuffer pidx;
    IndexBuffer uvidx;
    IndexBuffer nidx;

    const float MAX_FLOAT = std::numeric_limits<float>::max();
    const float MIN_FLOAT = std::numeric_limits<float>::min();
    float minPos[3] = { MAX_FLOAT , MAX_FLOAT , MAX_FLOAT };
    float maxPos[3] = { MIN_FLOAT, MIN_FLOAT, MIN_FLOAT };

    std::ifstream objfile;
    objfile.open(_identifier);
    if (!objfile.is_open())
    {
        // Set to default if can't read file
        setQuadData();
        return;
    }

    std::string line;

    // Enable debugging of read by dumping to disk what was read in.
    // Disabled by default
    std::ofstream dump;
    bool debugDump = false;
    if (debugDump)
    {
        dump.open("dump.obj");
    }

    float val1, val2, val3;
    unsigned int ipos[4], iuv[4], inorm[4];
    while (std::getline(objfile, line))
    {
        if (line.substr(0, 2) == "v ")
        {
            std::istringstream valstring(line.substr(2));
            valstring >> val1; valstring >> val2; valstring >> val3;

            if (debugDump)
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

            if (debugDump)
            {
                dump << "vt " << val1 << " " << val2 << std::endl;
            }
        }
        else if (line.substr(0, 3) == "vn ")
        {
            std::istringstream valstring(line.substr(3));
            valstring >> val1; valstring >> val2; valstring >> val3;
            norm.push_back(val1); norm.push_back(val2); norm.push_back(val3);

            if (debugDump)
            {
                dump << "vn " << val1 << " " << val2 << " " << val3 << std::endl;
            }
        }
        else if (line.substr(0, 2) == "f ")
        {
            // Extact out the compont parts from face string
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
                    std::vector<string> vertexParts = MaterialX::splitString(vertices[i], "/");
                    if (vertexParts.size() == 3)
                    {
                        ipos[i] = std::stoi(vertexParts[0]);
                        iuv[i] = std::stoi(vertexParts[1]);
                        inorm[i] = std::stoi(vertexParts[2]);
                        vertexCount++;
                    }
                }
            }

            if (vertexCount >= 3)
            {
                pidx.push_back(ipos[0] - 1);
                pidx.push_back(ipos[1] - 1);
                pidx.push_back(ipos[2] - 1);

                uvidx.push_back(iuv[0] - 1);
                uvidx.push_back(iuv[1] - 1);
                uvidx.push_back(iuv[2] - 1);

                nidx.push_back(inorm[0] - 1);
                nidx.push_back(inorm[1] - 1);
                nidx.push_back(inorm[2] - 1);

                if (debugDump)
                {
                    dump << "f "
                        << ipos[0] << "/" << iuv[0] << "/" << inorm[0] << " "
                        << ipos[1] << "/" << iuv[1] << "/" << inorm[1] << " "
                        << ipos[2] << "/" << iuv[2] << "/" << inorm[2] << std::endl;
                }

                if (vertexCount >= 4)
                {
                    pidx.push_back(ipos[0] - 1);
                    pidx.push_back(ipos[2] - 1);
                    pidx.push_back(ipos[3] - 1);

                    uvidx.push_back(iuv[0] - 1);
                    uvidx.push_back(iuv[2] - 1);
                    uvidx.push_back(iuv[3] - 1);

                    nidx.push_back(inorm[0] - 1);
                    nidx.push_back(inorm[2] - 1);
                    nidx.push_back(inorm[3] - 1);

                    if (debugDump)
                    {
                        dump << "f "
                            << ipos[0] << "/" << iuv[0] << "/" << inorm[0] << " "
                            << ipos[2] << "/" << iuv[2] << "/" << inorm[2] << " "
                            << ipos[3] << "/" << iuv[3] << "/" << inorm[3] << std::endl;
                    }
                }
            }
        }
    }

    dump.close();
    objfile.close();

    // Set bounds
    _minimumBounds[0] = minPos[0];
    _minimumBounds[1] = minPos[1];
    _minimumBounds[2] = minPos[2];
    _maximumBounds[0] = maxPos[0];
    _maximumBounds[1] = maxPos[1];
    _maximumBounds[2] = maxPos[2];

    // Organize data to get triangles for positions 
    for (unsigned int i = 0; i < pidx.size(); i++)
    {
        unsigned int vertexIndex = 3 * pidx[i];
        _positionData.push_back(pos[vertexIndex]);
        _positionData.push_back(pos[vertexIndex + 1]);
        _positionData.push_back(pos[vertexIndex + 2]);
    }

    // Organize data to get triangles for texture coordinates 
    for (unsigned int i = 0; i < uvidx.size(); i++)
    {
        unsigned int vertexIndex = 2 * uvidx[i];
        _texcoordData[0].push_back(uv[vertexIndex]);
        _texcoordData[0].push_back(uv[vertexIndex + 1]);

        _texcoordData[1].push_back(uv[vertexIndex + 1]);
        _texcoordData[1].push_back(uv[vertexIndex]);

        // Fake some colors
        _colorData[0].push_back(uv[vertexIndex]);
        _colorData[0].push_back(uv[vertexIndex] + 1);
        _colorData[0].push_back(1.0f);
        _colorData[0].push_back(1.0f);

        _colorData[1].push_back(1.0f);
        _colorData[1].push_back(uv[vertexIndex] + 1);
        _colorData[1].push_back(uv[vertexIndex]);
        _colorData[1].push_back(1.0f);
    }

    // Organize data to get triangles for normals 
    for (unsigned int i = 0; i < nidx.size(); i++)
    {
        unsigned int vertexIndex = 3 * nidx[i];
        _normalData.push_back(norm[vertexIndex]);
        _normalData.push_back(norm[vertexIndex + 1]);
        _normalData.push_back(norm[vertexIndex + 2]);

        // Fake some tangent, bitangent data
        _tangentData[0].push_back(norm[vertexIndex + 2]);
        _tangentData[0].push_back(norm[vertexIndex + 1]);
        _tangentData[0].push_back(norm[vertexIndex]);

        _tangentData[1].push_back(norm[vertexIndex + 1]);
        _tangentData[1].push_back(norm[vertexIndex + 2]);
        _tangentData[1].push_back(norm[vertexIndex]);

        _bitangentData[0].push_back(norm[vertexIndex + 2]);
        _bitangentData[0].push_back(norm[vertexIndex + 1]);
        _bitangentData[0].push_back(norm[vertexIndex]);

        _bitangentData[0].push_back(norm[vertexIndex + 1]);
        _bitangentData[0].push_back(norm[vertexIndex + 2]);
        _bitangentData[0].push_back(norm[vertexIndex]);
    }

    // Set up flattened indexing
    if (_positionData.size() && pidx.size())
    {
        _indexing.resize(pidx.size());
        std::iota(_indexing.begin(), _indexing.end(), 0);
    }
}

void ObjGeometryHandler::setIdentifier(const std::string& identifier)
{
    if (identifier != _identifier)
    {
        GeometryHandler::setIdentifier(identifier);
        clearData();
    }
}

const Vector3& ObjGeometryHandler::getMinimumBounds()
{
    readData();
    return _minimumBounds;
}

Vector3& ObjGeometryHandler::getMaximumBounds()
{
    readData();
    return _maximumBounds;
}


GeometryHandler::IndexBuffer& ObjGeometryHandler::getIndexing()
{
    readData();
    return _indexing;
}

GeometryHandler::FloatBuffer& ObjGeometryHandler::getPositions(unsigned int& stride, unsigned int /*index*/)
{
    readData();
    stride = 3;
    return _positionData;
}

GeometryHandler::FloatBuffer& ObjGeometryHandler::getNormals(unsigned int& stride, unsigned int /*index*/)
{
    readData();
    stride = 3;
    return _normalData;
}

GeometryHandler::FloatBuffer& ObjGeometryHandler::getTextureCoords(unsigned int& stride, unsigned int index)
{
    readData();
    stride = 2;
    return (index > 1 ? _texcoordData[0] : _texcoordData[index]);
}

GeometryHandler::FloatBuffer& ObjGeometryHandler::getTangents(unsigned int& stride, unsigned int index)
{
    readData();
    stride = 3;
    return (index > 1 ? _tangentData[0] : _tangentData[index]);
}

GeometryHandler::FloatBuffer& ObjGeometryHandler::getBitangents(unsigned int& stride, unsigned int index)
{
    readData();
    stride = 3;
    return (index > 1 ? _bitangentData[0] : _bitangentData[index]);
}

GeometryHandler::FloatBuffer& ObjGeometryHandler::getColors(unsigned int& stride, unsigned int index)
{
    readData();
    stride = 4;
    return (index > 1 ? _colorData[0] : _colorData[index]);
}

ObjGeometryHandler::FloatBuffer& ObjGeometryHandler::getAttribute(const std::string& attributeType, 
                                                                          unsigned int& stride, 
                                                                          unsigned int index)
{
    readData();
    if (attributeType.compare(0,
        ObjGeometryHandler::POSITION_ATTRIBUTE.size(),
        ObjGeometryHandler::POSITION_ATTRIBUTE) == 0)
    {
        return getPositions(stride, index);
    }
    else if (attributeType.compare(0,
        ObjGeometryHandler::NORMAL_ATTRIBUTE.size(),
        ObjGeometryHandler::NORMAL_ATTRIBUTE) == 0)
    {
        return getNormals(stride, index);
    }
    else if (attributeType.compare(0,
        ObjGeometryHandler::TEXCOORD_ATTRIBUTE.size(),
        ObjGeometryHandler::TEXCOORD_ATTRIBUTE) == 0)
    {
        return getTextureCoords(stride, index);
    }
    else if (attributeType.compare(0,
        ObjGeometryHandler::COLOR_ATTRIBUTE.size(),
        ObjGeometryHandler::COLOR_ATTRIBUTE) == 0)
    {
        return getColors(stride, index);
    }
    else if (attributeType.compare(0,
        ObjGeometryHandler::TANGENT_ATTRIBUTE.size(),
        ObjGeometryHandler::TANGENT_ATTRIBUTE) == 0)
    {
        return getTangents(stride, index);
    }
    else if (attributeType.compare(0,
        ObjGeometryHandler::BITANGENT_ATTRIBUTE.size(),
        ObjGeometryHandler::BITANGENT_ATTRIBUTE) == 0)
    {
        return getBitangents(stride, index);
    }
    return getPositions(stride, index);
}


}