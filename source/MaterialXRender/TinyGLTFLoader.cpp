//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/TinyGLTFLoader.h>
#include <MaterialXCore/Util.h>

// Want implementation but not image capabilities
#define TINYGLTF_IMPLEMENTATION 1
#define TINYGLTF_NO_STB_IMAGE 1
#define TINYGLTF_NO_STB_IMAGE_WRITE 1
#define TINYGLTF_NO_EXTERNAL_IMAGE 1

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
#endif

#include <MaterialXRender/External/tinygltf/tiny_gltf.h>

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

#include <iostream>
#include <algorithm>
#include <stack>

MATERIALX_NAMESPACE_BEGIN

namespace {

const float MAX_FLOAT = std::numeric_limits<float>::max();
const size_t FACE_VERTEX_COUNT = 3;

uint32_t VALUE_AS_UINT32(int type, const unsigned char* value)
{
    switch (type)
    {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    case TINYGLTF_COMPONENT_TYPE_BYTE:
        return static_cast<uint32_t>(*value);
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    case TINYGLTF_COMPONENT_TYPE_SHORT:
        return static_cast<uint32_t>(*(short*)value);
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    case TINYGLTF_COMPONENT_TYPE_INT:
        return static_cast<uint32_t>(*(int*)value);
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
        return static_cast<uint32_t>(*(int*)value);
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
        return static_cast<uint32_t>(*(double*)value);
    default:
        return 0;
    }
}

// List of transforms which match to model meshes
using MeshMatrixList = std::unordered_map<size_t, Matrix44>;
// Use to cache object-to-world transforms during traversal. 
using Matrix44Stack = std::stack<Matrix44>;

const float PI = std::acos(-1.0f);

// Iterate through all levels until meshes are found. For each
// mesh cache it's object-to-world matrix
void computeMeshMatrices(MeshMatrixList& meshMatrices, tinygltf::Model& model, const tinygltf::Node& node,
                         Matrix44Stack& matrixStack, unsigned int debugLevel)
{
    std::string indent;
    for (size_t i = 0; i < matrixStack.size(); i++)
    {
        indent += "\t";
    }
    if (debugLevel > 0)
        std::cout << indent + "Visit node: " << node.name << std::endl;

    Matrix44 matrix = Matrix44::IDENTITY;
    if (node.matrix.size() == 16)
    {
        matrix = Matrix44(
            (float)node.matrix[0], (float)node.matrix[1], (float)node.matrix[2], (float)node.matrix[3],
            (float)node.matrix[4], (float)node.matrix[5], (float)node.matrix[6], (float)node.matrix[7],
            (float)node.matrix[8], (float)node.matrix[9], (float)node.matrix[10], (float)node.matrix[11],
            (float)node.matrix[12], (float)node.matrix[13], (float)node.matrix[14], (float)node.matrix[15]);
    }
    else
    {
        if (node.scale.size() == 3) {
            Matrix44 scale = matrix.createScale({ (float)node.scale[0], (float)node.scale[1], (float)node.scale[2] });
            matrix *= scale;
        }

        if (node.rotation.size() == 4)
        {
            Matrix44 rotation = Matrix44::createRotationZ((float)node.rotation[2] / 180.0f * PI) *
                Matrix44::createRotationY((float)node.rotation[1] / 180.0f * PI) *
                Matrix44::createRotationX((float)node.rotation[0] / 180.0f * PI);
            matrix *= rotation;
        }

        if (node.translation.size() == 3)
        {
            Vector3 transVec = { (float)node.translation[0], (float)node.translation[1], (float)node.translation[2] };
            Matrix44 translation = matrix.createTranslation(transVec);
            translation *= translation;
        }
    }
    if (matrixStack.size())
    {
        matrixStack.push(matrixStack.top() * matrix);
    }
    else
    {
        matrixStack.push(matrix);
    }

    // Cache the matrix if this is a mesh
    if (node.mesh > -1 && ((size_t)node.mesh < model.meshes.size()))
    {
        tinygltf::Mesh mesh = model.meshes[node.mesh];
        Matrix44 meshMatrix = matrixStack.top();
        meshMatrices[node.mesh] = meshMatrix.getTranspose();
        if (debugLevel > 0)
        {
            std::cout << indent + "Set Mesh[" + std::to_string(node.mesh) + "] = " <<
                mesh.name << ".Matrix : \n";
            for (size_t m = 0; m < 4; m++)
            {
                std::cout << indent;
                for (size_t n = 0; n < 4; n++)
                {
                    std::cout << std::to_string(meshMatrix[m][n]) + " ";
                }
                std::cout << std::endl;
            }
        }
    }

    // Iterate over all children.
    for (auto childNodeIndex: node.children)
    {
        computeMeshMatrices(meshMatrices, model, model.nodes[childNodeIndex], matrixStack, debugLevel);
    }
    
    matrixStack.pop();
}

} // anonymous namespace

bool TinyGLTFLoader::load(const FilePath& filePath, MeshList& meshList, bool texcoordVerticalFlip)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF gltf_ctx;
	std::string err;
	std::string warn;
	const std::string input_filename = filePath.asString();

	bool store_original_json_for_extras_and_extensions = false;
	gltf_ctx.SetStoreOriginalJSONForExtrasAndExtensions(
		store_original_json_for_extras_and_extensions);

	const std::string ext = filePath.getExtension();
    const std::string BINARY_EXTENSION = "glb";
    const std::string ASCII_EXTENSION = "gltf";

	bool ret = false;
	if (ext.compare(BINARY_EXTENSION) == 0)
	{
		// Try to read as binary
		ret = gltf_ctx.LoadBinaryFromFile(&model, &err, &warn,
			input_filename.c_str());
	}
	else if (ext.compare(ASCII_EXTENSION) == 0)
    {
        // Try to read as ascii
		ret =
			gltf_ctx.LoadASCIIFromFile(&model, &err, &warn, input_filename.c_str());
	}

	if (!warn.empty() || !err.empty() || !ret)
	{
	    return false;
	}

    if (model.scenes.size() == 0)
    {
        return false;
    }

    Vector3 boxMin = { MAX_FLOAT, MAX_FLOAT, MAX_FLOAT };
    Vector3 boxMax = { -MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT };

    MeshMatrixList meshMatrices;
    Matrix44Stack matrixStack;
    matrixStack.push(Matrix44::IDENTITY);
    int scene_to_display = model.defaultScene > -1 ? model.defaultScene : 0;
    const tinygltf::Scene& scene = model.scenes[scene_to_display];
    for (size_t i = 0; i < scene.nodes.size(); i++) 
    {
        computeMeshMatrices(meshMatrices, model, model.nodes[scene.nodes[i]], matrixStack, _debugLevel);
    }

    // Load model 
    // For each gltf mesh a new mesh is created
    // - A MeshStream == buffer view for an attribute + associated data.
    // - A MeshPartition == buffer view for indexing + associated data.
    for (size_t m=0; m< model.meshes.size(); m++)
    {
        tinygltf::Mesh& gMesh = model.meshes[m];

        // Create new mesh. Generate a name if the mesh does not have one.
        std::string meshName = gMesh.name;
        if (meshName.empty())
        {
            meshName = "generatedName_" + std::to_string(m);
        }
        MeshPtr mesh = Mesh::create(meshName);
        if (_debugLevel > 0)
            std::cout << "Translate mesh: " << meshName << std::endl;
        meshList.push_back(mesh);
        mesh->setSourceUri(filePath);

        MeshStreamPtr positionStream = nullptr; 
        MeshStreamPtr normalStream = nullptr;
        MeshStreamPtr texcoordStream = nullptr;
        MeshStreamPtr tangentStream = nullptr;

        // Scan primitives on the mesh
        for (tinygltf::Primitive& gPrim : gMesh.primitives)
        {
            // Get index accessor for the prim and create a partition
            // Only support triangle indexing for now
            int accessorIndex = gPrim.indices;
            if ((accessorIndex >= 0) &&
                (gPrim.mode == TINYGLTF_MODE_TRIANGLES))
            {
                const tinygltf::Accessor& gaccessor = model.accessors[accessorIndex];
                const tinygltf::BufferView& gBufferView = model.bufferViews[gaccessor.bufferView];
                const tinygltf::Buffer& gBuffer = model.buffers[gBufferView.buffer];

                size_t indexCount = gaccessor.count;
                MeshPartitionPtr part = MeshPartition::create();
                size_t faceCount = indexCount / FACE_VERTEX_COUNT;
                part->setFaceCount(faceCount);
                part->setName(meshName); 

                MeshIndexBuffer& indices = part->getIndices();
                size_t startLocation = gBufferView.byteOffset + gaccessor.byteOffset;
                size_t byteStride = gaccessor.ByteStride(gBufferView);

                bool isTriangleList = false;
                std::string indexingType = "invalid type";
                switch (gPrim.mode) {
                case TINYGLTF_MODE_POINTS:
                    indexingType = "point list";
                    break;
                case TINYGLTF_MODE_LINE:
                    indexingType = "line list";
                    break;
                case TINYGLTF_MODE_LINE_STRIP:
                    indexingType = "line string";
                    break;
                case TINYGLTF_MODE_TRIANGLES:
                    indexingType = "triangle list";
                    isTriangleList = true;
                    break;
                case TINYGLTF_MODE_TRIANGLE_STRIP:
                    indexingType = "triangle strip";
                    break;       
                default:
                    break;
                }
                if (!isTriangleList)
                {
                    if (_debugLevel > 0)
                    {
                        std::cout << "Skip unsupported prim type: " << indexingType << " on mesh" <<
                            meshName << std::endl;
                    }
                    continue;
                }

                if (_debugLevel > 0)
                {
                    std::cout << "*** Read mesh: " << meshName << std::endl;
                    std::cout << "Index start byte offset: " << std::to_string(startLocation) << std::endl;
                    std::cout << "-- Index byte stride: " << std::to_string(byteStride) << std::endl;
                    if (_debugLevel > 1)
                        std::cout << "{\n";
                }
                for (size_t i = 0; i < indexCount; i++)
                {
                    size_t offset = startLocation + (i * byteStride);
                    uint32_t bufferIndex = VALUE_AS_UINT32(gaccessor.componentType, &(gBuffer.data[offset]));
                    indices.push_back(bufferIndex);
                    if (_debugLevel > 1)
                        std::cout << "[" + std::to_string(i) + "] = " + std::to_string(bufferIndex) + "\n";
                }
                if (_debugLevel > 1)
                    std::cout << "}\n";
                mesh->addPartition(part);
            }

            // Check for any matrix transform for positions
            Matrix44 positionMatrix = Matrix44::IDENTITY;
            if (meshMatrices.find(m) != meshMatrices.end())
            {
                positionMatrix = meshMatrices[m];
            }

            // Get attributes. Note that bufferViews contain the content descriptioon
            for (auto& gattrib : gPrim.attributes)
            {
                // Find out the byteStride
                const tinygltf::Accessor& gAccessor = model.accessors[gattrib.second];
                const tinygltf::BufferView& gBufferView = model.bufferViews[gAccessor.bufferView];
                const tinygltf::Buffer& gBuffer = model.buffers[gBufferView.buffer];
                size_t byteStride = gAccessor.ByteStride(gBufferView);
                size_t floatStride = byteStride / sizeof(float);

                // Make sure to offset by both view and accessor
                size_t byteOffset = gBufferView.byteOffset + gAccessor.byteOffset;
                size_t startLocation = byteOffset / sizeof(float);

                unsigned int vectorSize = 3;
                if (gAccessor.type == TINYGLTF_TYPE_VEC2)
                {

                    vectorSize = 2;
                }
                else if (gAccessor.type == TINYGLTF_TYPE_VEC4)
                {
                    vectorSize = 4;
                }

                if (_debugLevel > 0)
                {
                    std::cout << "** READ ATTRIB: " << gattrib.first <<
                        " from buffer: " << std::to_string(gBufferView.buffer) << std::endl;
                    std::cout << "-- Buffer start byte offset: " << std::to_string(byteOffset) << std::endl;
                    std::cout << "-- Buffer start float offset: " << std::to_string(startLocation) << std::endl;
                    std::cout << "-- Byte stride: " << std::to_string(byteStride) << std::endl;
                    std::cout << "-- Float stride: " << std::to_string(floatStride) << std::endl;
                    std::cout << "-- Vector size: " << std::to_string(vectorSize) << std::endl;
                }

                bool isPositionStream = gattrib.first.compare("POSITION") == 0;
                MeshStreamPtr geomStream = nullptr;
                if (isPositionStream)
                {
                    if (!positionStream)
                    {
                        positionStream = MeshStream::create("i_" + MeshStream::POSITION_ATTRIBUTE, MeshStream::POSITION_ATTRIBUTE, 0);
                    }
                    geomStream = positionStream;
                }
                else if (gattrib.first.compare("NORMAL") == 0)
                {
                    normalStream = MeshStream::create("i_" + MeshStream::NORMAL_ATTRIBUTE, MeshStream::NORMAL_ATTRIBUTE, 0);
                    geomStream = normalStream;
                }
                else if (gattrib.first.compare("TEXCOORD_0") == 0)
                {
                    if (!texcoordStream)
                    {
                        texcoordStream = MeshStream::create("i_" + MeshStream::TEXCOORD_ATTRIBUTE + "_0", MeshStream::TEXCOORD_ATTRIBUTE, 0);
                    }

                    if (vectorSize == 2)
                    {
                        texcoordStream->setStride(MeshStream::STRIDE_2D);
                    }
                    geomStream = texcoordStream;
                }
                else if (gattrib.first.compare("TANGENT") == 0)
                {
                    tangentStream = MeshStream::create("i_" + MeshStream::TANGENT_ATTRIBUTE, MeshStream::NORMAL_ATTRIBUTE, 0);
                    geomStream = tangentStream;

                    // 4-channel tangents are not supported. Drop the 4th coordinate for now
                    if (vectorSize > 3)
                    {
                        vectorSize = 3;
                    }
                }
                if (geomStream)
                {
                    // Fill in stream 
                    MeshFloatBuffer& buffer = geomStream->getData();

                    size_t dataCount = gAccessor.count;
                    const unsigned char* charPointer = &(gBuffer.data[byteOffset]);
                    float* floatPointer = const_cast<float *>(reinterpret_cast<const float*>(charPointer));
                    if (_debugLevel > 1)
                        std::cout << "{\n";
                    for (size_t i = 0; i < dataCount; i++)
                    {
                        // Copy the vector over
                        if (_debugLevel > 1)
                            std::cout << "[" + std::to_string(i) + "] = { ";

                        if (!isPositionStream)
                        {
                            for (size_t v = 0; v < vectorSize; v++)
                            {
                                float bufferData = *(floatPointer + v);
                                if (geomStream == texcoordStream && v==1)
                                {
                                    if (!texcoordVerticalFlip)
                                    { 
                                        bufferData = 1.0f - bufferData;
                                    }
                                }
                                buffer.push_back(bufferData);
                                if (_debugLevel > 1)
                                    std::cout << std::to_string(buffer[i]) + " ";
                            }
                        }

                        // Transform positions by an appropriate matrix
                        if (isPositionStream)
                        {
                            Vector3 position(*(floatPointer + 0), *(floatPointer + 1), *(floatPointer + 2));
                            position = positionMatrix.transformPoint(position);

                            // Update bounds.
                            for (size_t v = 0; v < 3; v++)
                            {
                                buffer.push_back(position[v]);
                                boxMin[v] = std::min(position[v], boxMin[v]);
                                boxMax[v] = std::max(position[v], boxMax[v]);
                            }
                        }
                        if (_debugLevel > 1)
                            std::cout << " }" << std::endl;

                        // Jump to next vector
                        floatPointer += floatStride;
                    }
                    if (_debugLevel > 1)
                        std::cout << "}\n";
                }
            }
        }

        // General noramsl if none provided
        if (!normalStream && positionStream)
        {
            normalStream = mesh->generateNormals(positionStream);
        }

        // Generate tangents if none provided
        if (!tangentStream && texcoordStream && positionStream && normalStream)
        {
            tangentStream = mesh->generateTangents(positionStream, normalStream, texcoordStream);
        }

        // Assign streams to mesh.
        if (positionStream)
        {
            mesh->addStream(positionStream);
        }
        if (normalStream)
        {
            mesh->addStream(normalStream);
        }
        if (texcoordStream)
        {
            mesh->addStream(texcoordStream);
        }
        if (tangentStream)
        {
            mesh->addStream(tangentStream);
        }

        // Assign properties to mesh.
        if (positionStream)
        {
            mesh->setVertexCount(positionStream->getData().size() / MeshStream::STRIDE_3D);
        }
        mesh->setMinimumBounds(boxMin);
        mesh->setMaximumBounds(boxMax);
        Vector3 sphereCenter = (boxMax + boxMin) * 0.5;
        mesh->setSphereCenter(sphereCenter);
        mesh->setSphereRadius((sphereCenter - boxMin).getMagnitude());
    }
	return true;
}

MATERIALX_NAMESPACE_END
