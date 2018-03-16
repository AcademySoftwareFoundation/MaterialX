#include <MaterialXView/Handlers/DefaultGeometryHandler.h>

namespace MaterialX
{ 
DefaultGeometryHandler::DefaultGeometryHandler() :
    GeometryHandler()
{
}

DefaultGeometryHandler::~DefaultGeometryHandler()
{
}

void DefaultGeometryHandler::clearData()
{
    _indexing.clear();
    _positionData.clear();
    _normalData.clear();
    _texcoordData.clear();
    _tangentData.clear();
    _bitangentData.clear();
    _colorData.clear();
}

void DefaultGeometryHandler::setIdentifier(const std::string identifier)
{
    if (identifier != _identifier)
    {
        GeometryHandler::setIdentifier(identifier);
        clearData();
    }
}

GeometryHandler::IndexBuffer& DefaultGeometryHandler::getIndexing(size_t &bufferSize)
{
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (_indexing.empty())
        {
            _indexing = {
                0, 1, 2, 0, 2, 3
            };
        }
    }

    bufferSize = _indexing.size() * sizeof(unsigned int);
    return _indexing;
}

GeometryHandler::FloatBuffer& DefaultGeometryHandler::getPositions(size_t &bufferSize)
{
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (_positionData.empty())
        {
            const float border = (float)_inputProperties.screenOffset;
            const float bufferWidth = (float)_inputProperties.screenWidth;
            const float bufferHeight = (float)_inputProperties.screenHeight;

            _positionData = 
            {
                border, border, 0.0f,
                    border, (bufferHeight - border), 0.0f,
                    (bufferWidth - border), (bufferHeight - border), 0.0f,
                    (bufferWidth - border), border, 0.0f
            };
        }
    }

    bufferSize = _positionData.size() * sizeof(float);
    return _positionData;
}

GeometryHandler::FloatBuffer& DefaultGeometryHandler::getNormals(size_t &bufferSize)
{
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (_normalData.empty())
        {
            _normalData = {
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f
            };
        }
    }

    bufferSize = _normalData.size() * sizeof(float);
    return _normalData;
}

GeometryHandler::FloatBuffer& DefaultGeometryHandler::getTextureCoords(size_t &bufferSize, unsigned int index)
{
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (_texcoordData.empty())
        {
            if (index == 0)
            {
                _texcoordData = {
                    0.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f,
                    1.0f, 0.0f
                };
            }
            else
            {
                _texcoordData = {
                    1.0f, 0.0f,
                    0.0f, 0.0f,
                    0.0f, 1.0f,
                    1.0f, 1.0f
                };
            }
        }
    }

    bufferSize = _texcoordData.size() * sizeof(float);
    return _texcoordData;
}

GeometryHandler::FloatBuffer& DefaultGeometryHandler::getTangents(size_t &bufferSize, unsigned int index)
{
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (_tangentData.empty())
        {
            if (index == 0)
            {
                _tangentData = {
                    .0f, 1.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    0.0f, -1.0f, 0.0f
                };
            }
            else
            {
                _tangentData = {
                    0.0f, -1.0f, 0.0f,
                    .0f, 1.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f
                };
            }
        }
    }

    bufferSize = _tangentData.size() * sizeof(float);
    return _tangentData;
}

GeometryHandler::FloatBuffer& DefaultGeometryHandler::getBitangents(size_t &bufferSize, unsigned int index)
{
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (_bitangentData.empty())
        {
            if (index == 0)
            {
                _bitangentData = {
                    1.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f,
                    0.0f, -1.0f, 0.0f
                };
            }
            else
            {
                _bitangentData = {
                    0.0f, -1.0f, 0.0f,
                    1.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f,
                    -1.0f, 0.0f, 0.0f
                };
            }
        }
    }

    bufferSize = _bitangentData.size() * sizeof(float);
    return _bitangentData;
}

GeometryHandler::FloatBuffer& DefaultGeometryHandler::getColors(size_t &bufferSize, unsigned int index)
{
    if (_identifier == SCREEN_ALIGNED_QUAD)
    {
        if (_colorData.empty())
        {
            if (index == 0)
            {
                _colorData = {
                    1.0f, 0.0f, 0.0f, 1.0f,
                    0.0f, 1.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, 1.0f, 0.0f, 1.0f
                };
            }
            else
            {
                _colorData = {
                    1.0f, 0.0f, 1.0f, 1.0f,
                    0.0f, 1.0f, 1.0f, 1.0f,
                    1.0f, 0.0f, 1.0f, 1.0f,
                    1.0f, 1.0f, 0.5f, 1.0f
                };
            }
        }
    }

    bufferSize = _colorData.size() * sizeof(float);
    return _colorData;
}

}