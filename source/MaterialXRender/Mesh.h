//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_MESH_H
#define MATERIALX_MESH_H

/// @file
/// Mesh interfaces

#include <MaterialXCore/Types.h>

namespace MaterialX
{

/// Geometry index buffer
using MeshIndexBuffer = vector<unsigned int>;
/// Float geometry buffer
using MeshFloatBuffer = vector<float>;

/// Shared pointer to a mesh stream
using MeshStreamPtr = shared_ptr<class MeshStream>;

/// List of mesh streams
using MeshStreamList = vector<MeshStreamPtr>;

/// @class MeshStream
/// Class to represent a mesh data stream
class MeshStream
{
  public:
    static const string POSITION_ATTRIBUTE;
    static const string NORMAL_ATTRIBUTE;
    static const string TEXCOORD_ATTRIBUTE;
    static const string TANGENT_ATTRIBUTE;
    static const string BITANGENT_ATTRIBUTE;
    static const string COLOR_ATTRIBUTE;
    static const string GEOMETRY_PROPERTY_ATTRIBUTE;

    static const unsigned int STRIDE_3D = 3;
    static const unsigned int STRIDE_2D = 2;
    static const unsigned int DEFAULT_STRIDE = STRIDE_3D;

  public:
    MeshStream(const string& name, const string& type, unsigned int index) :
        _name(name),
        _type(type),
        _index(index),
        _stride(DEFAULT_STRIDE)
    {
    }
    ~MeshStream() { }

    /// Create a new mesh stream
    static MeshStreamPtr create(const string& name, const string& type, unsigned int index = 0)
    {
        return std::make_shared<MeshStream>(name, type, index);
    }

    /// Resize data to an given number of elements
    void resize(unsigned int elementCount)
    {
        _data.resize((size_t) elementCount * (size_t) _stride);
    }

    /// Get stream name
    const string& getName() const
    {
        return _name;
    }

    /// Get stream attribute name
    const string& getType() const
    {
        return _type;
    }

    /// Get stream index
    unsigned int getIndex() const
    {
        return _index;
    }

    /// Get stream data
    MeshFloatBuffer& getData()
    {
        return _data;
    }

    /// Get stream data
    const MeshFloatBuffer& getData() const
    {
        return _data;
    }

    /// Get stride between elements
    unsigned int getStride() const
    {
        return _stride;
    }

    /// Set stride between elements
    void setStride(unsigned int stride)
    {
        _stride = stride;
    }

    size_t getSize() const
    {
        return _data.size();
    }

    void transform(const Matrix44 &matrix);

  protected:
    string _name;
    string _type;
    unsigned int _index;
    MeshFloatBuffer _data;
    unsigned int _stride;
};

/// Shared pointer to a mesh partition
using MeshPartitionPtr = shared_ptr<class MeshPartition>;

/// @class MeshPartition
/// Class that describes a sub-region of a mesh using vertex indexing.
/// Note that a face is considered to be a triangle.
class MeshPartition
{
  public:
    MeshPartition() :
        _faceCount(0)
    {
    }
    ~MeshPartition() { }

    /// Create a new mesh partition
    static MeshPartitionPtr create()
    {
        return std::make_shared<MeshPartition>();
    }

    /// Resize data to the given number of indices
    void resize(unsigned int indexCount)
    {
        _indices.resize(indexCount);
    }

    /// Get geometry identifier
    const string& getIdentifier() const
    {
        return _identifier;
    }

    /// Set geometry identifier
    void setIdentifier(const string& val)
    {
        _identifier = val;
    }

    /// Return indexing
    MeshIndexBuffer& getIndices()
    {
        return _indices;
    }

    /// Return indexing
    const MeshIndexBuffer& getIndices() const
    {
        return _indices;
    }

    /// Return number of faces
    size_t getFaceCount() const
    {
        return _faceCount;
    }

    /// Set face count
    void setFaceCount(size_t val)
    {
        _faceCount = val;
    }

  private:
    string _identifier;
    MeshIndexBuffer _indices;
    size_t _faceCount;
};

/// Shared pointer to a mesh
using MeshPtr = shared_ptr<class Mesh>;

/// List of meshes
using MeshList = vector<MeshPtr>;

/// Map from names to meshes
using MeshMap = std::unordered_map<string, MeshPtr>;

/// @class Mesh
/// Container for mesh data
class Mesh
{
  public:
    Mesh(const string& identifier);
    ~Mesh() { }

    /// Create a new mesh
    static MeshPtr create(const string& identifier)
    {
        return std::make_shared<Mesh>(identifier);
    }

    /// Get mesh identifier
    const string& getIdentifier() const
    {
        return _identifier;
    }

    /// Set the mesh's source URI.
    void setSourceUri(const string& sourceUri)
    {
        _sourceUri = sourceUri;
    }

    /// Return true if this mesh has a source URI.
    bool hasSourceUri() const
    {
        return !_sourceUri.empty();
    }

    /// Return the mesh's source URI.
    const string& getSourceUri() const
    {
        return _sourceUri;
    }

    /// Get a mesh stream by name
    /// @param name Name of stream
    /// @return Reference to a mesh stream if found
    MeshStreamPtr getStream(const string& name) const
    {
        for (const auto& stream : _streams)
        {
            if (stream->getName() == name)
            {
                return stream;
            }
        }
        return MeshStreamPtr();
    }

    /// Get a mesh stream by type and index
    /// @param type Type of stream
    /// @param index Index of stream
    /// @return Reference to a mesh stream if found
    MeshStreamPtr getStream(const string& type, unsigned int index) const
    {
        for (const auto& stream : _streams)
        {
            if (stream->getType() == type &&
                stream->getIndex() == index)
            {
                return stream;
            }
        }
        return MeshStreamPtr();
    }

    /// Add a mesh stream
    void addStream(MeshStreamPtr stream)
    {
        _streams.push_back(stream);
    }

    /// Set vertex count
    void setVertexCount(size_t val)
    {
        _vertexCount = val;
    }

    /// Get vertex count
    size_t getVertexCount() const
    {
        return _vertexCount;
    }

    /// Set the minimum bounds for the geometry
    void setMinimumBounds(const Vector3& val)
    {
        _minimumBounds = val;
    }

    /// Return the minimum bounds for the geometry
    const Vector3& getMinimumBounds() const
    {
        return _minimumBounds;
    }

    /// Set the minimum bounds for the geometry
    void setMaximumBounds(const Vector3& v)
    {
        _maximumBounds = v;
    }

    /// Return the minimum bounds for the geometry
    const Vector3& getMaximumBounds() const
    {
        return _maximumBounds;
    }

    /// Set center of the bounding sphere
    void setSphereCenter(const Vector3& val)
    {
        _sphereCenter = val;
    }

    /// Return center of the bounding sphere
    const Vector3& getSphereCenter() const
    {
        return _sphereCenter;
    }

    /// Set radius of the bounding sphere
    void setSphereRadius(float val)
    {
        _sphereRadius = val;
    }

    /// Return radius of the bounding sphere
    float getSphereRadius() const
    {
        return _sphereRadius;
    }

    /// Return the number of mesh partitions
    size_t getPartitionCount() const
    {
        return _partitions.size();
    }

    /// Add a partition
    void addPartition(MeshPartitionPtr partition)
    {
        _partitions.push_back(partition);
    }

    /// Return a reference to a mesh partition
    MeshPartitionPtr getPartition(size_t partIndex) const
    {
        return _partitions[partIndex];
    }

    /// Generate tangents and optionally bitangents for a given
    /// set of positions, texture coordinates and normals.
    /// @param positionStream Positions to use
    /// @param texcoordStream Texture coordinates to use
    /// @param normalStream Normals to use
    /// @param tangentStream Tangents to produce
    /// @param bitangentStream Bitangents to produce.
    /// Returns true if successful.
    bool generateTangents(MeshStreamPtr positionStream, MeshStreamPtr texcoordStream, MeshStreamPtr normalStream,
                          MeshStreamPtr tangentStream, MeshStreamPtr bitangentStream);

    /// Merge all mesh partitions into one.
    void mergePartitions();

    /// Split the mesh into a single partition per UDIM.
    void splitByUdims();

  private:
    string _identifier;
    string _sourceUri;

    Vector3 _minimumBounds;
    Vector3 _maximumBounds;

    Vector3 _sphereCenter;
    float _sphereRadius;

    MeshStreamList _streams;
    size_t _vertexCount;
    vector<MeshPartitionPtr> _partitions;
};

} // namespace MaterialX

#endif
