//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Util.h>
#include <MaterialXRender/Handlers/GeometryHandler.h>

namespace MaterialX
{
void GeometryHandler::addLoader(GeometryLoaderPtr loader)
{
    const StringVec& extensions = loader->supportedExtensions();
    for (auto extension : extensions)
    {
        _geometryLoaders.insert(std::pair<std::string, GeometryLoaderPtr>(extension, loader));
    }
}

void GeometryHandler::clearGeometry()
{
    _meshes.clear();
    computeBounds();
}

bool GeometryHandler::hasGeometry(const string& location)
{
    for (auto mesh : _meshes)
    {
        if (mesh->getSourceUri() == location)
        {
            return true;
        }
    }
    return false;
}

void GeometryHandler::getGeometry(MeshList& meshes, const string& location)
{
    meshes.clear();
    for (auto mesh : _meshes)
    {
        if (mesh->getSourceUri() == location)
        {
            meshes.push_back(mesh);
        }
    }
}

void GeometryHandler::clearGeometry(const string& location)
{
    for (auto mesh : _meshes)
    {
        if (mesh->getSourceUri() == location)
        {
            _meshes.erase(std::find(_meshes.begin(), _meshes.end(), mesh));
        }
    }
}

void GeometryHandler::computeBounds()
{
    const float MAX_FLOAT = std::numeric_limits<float>::max();
    _minimumBounds = { MAX_FLOAT, MAX_FLOAT, MAX_FLOAT };
    _maximumBounds = { -MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT };
    for (auto mesh : _meshes)
    {
        const Vector3& minMesh = mesh->getMinimumBounds();
        _minimumBounds[0] = std::min(minMesh[0], _minimumBounds[0]);
        _minimumBounds[1] = std::min(minMesh[1], _minimumBounds[1]);
        _minimumBounds[2] = std::min(minMesh[2], _minimumBounds[2]);
        const Vector3& maxMesh = mesh->getMaximumBounds();
        _maximumBounds[0] = std::max(maxMesh[0], _maximumBounds[0]);
        _maximumBounds[1] = std::max(maxMesh[1], _maximumBounds[1]);
        _maximumBounds[2] = std::max(maxMesh[2], _maximumBounds[2]);
    }
}

bool GeometryHandler::loadGeometry(const FilePath& filePath)
{
    // Early return if already loaded
    if (hasGeometry(filePath))
        return true;

    bool loaded = false;

    std::pair <GeometryLoaderMap::iterator, GeometryLoaderMap::iterator> range;
    string extension = MaterialX::getFileExtension(filePath);
    range = _geometryLoaders.equal_range(extension);
    GeometryLoaderMap::iterator first = --range.second;
    GeometryLoaderMap::iterator last = --range.first;
    for (auto it = first; it != last; --it)
    {
        loaded = it->second->load(filePath, _meshes);
        if (loaded)
        {
            break;
        }
    }

    // Recompute bounds if load was successful
    if (loaded)
    {
        computeBounds();
    }

    return loaded;
}

}
