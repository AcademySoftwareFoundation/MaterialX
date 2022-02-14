//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRender/CGLTFLoader.h>
#include <MaterialXCore/Util.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Document.h>

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif

#define CGLTF_IMPLEMENTATION
#include <MaterialXRender/External/cgltf/cgltf.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

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

// List of transforms which match to meshes
using GLTFMeshMatrixList = std::unordered_map<cgltf_mesh*, Matrix44>;

// Compute matrices for each mesh. Does not support transform instancing.
void computeMeshMatrices(GLTFMeshMatrixList& meshMatrices, cgltf_node* cnode)
{
    std::string indent;

    cgltf_mesh* cmesh = cnode->mesh;
    if (cmesh)
    {
        float t[16];
        cgltf_node_transform_world(cnode, t);
        Matrix44 positionMatrix = Matrix44(
            (float)t[0], (float)t[1], (float)t[2], (float)t[3],
            (float)t[4], (float)t[5], (float)t[6], (float)t[7],
            (float)t[8], (float)t[9], (float)t[10], (float)t[11],
            (float)t[12], (float)t[13], (float)t[14], (float)t[15]);
		meshMatrices[cmesh] = positionMatrix; //.getTranspose();
    }
    else
    {
        // Iterate over all children.
        for (cgltf_size i=0; i<cnode->children_count; i++)
        {
            computeMeshMatrices(meshMatrices, cnode->children[i]);
        }
    }
}

} // anonymous namespace

bool CGLTFLoader::load(const FilePath& filePath, MeshList& meshList, bool texcoordVerticalFlip)
{	
	const std::string input_filename = filePath.asString();

	const std::string ext = stringToLower(filePath.getExtension());
    const std::string BINARY_EXTENSION = "glb";
    const std::string ASCII_EXTENSION = "gltf";
	if (ext != BINARY_EXTENSION && ext != ASCII_EXTENSION)
	{
		return false;
	}

    cgltf_options options;
	std::memset(&options, 0, sizeof(options));
    cgltf_data* data = nullptr;

    // Read file
    cgltf_result result = cgltf_parse_file(&options, input_filename.c_str(), &data);
    if (result != cgltf_result_success)
    {
        return false;
    }
    if (cgltf_load_buffers(&options, data, input_filename.c_str()) != cgltf_result_success) 
    {
        return false;
    }

	// Precompute mesh / matrix associations
    GLTFMeshMatrixList gltfMeshMatrixList;
    for (cgltf_size sceneIndex = 0; sceneIndex < data->scenes_count; ++sceneIndex)
    {
        cgltf_scene* scene = &data->scenes[sceneIndex];
        for (cgltf_size nodeIndex = 0; nodeIndex < scene->nodes_count; ++nodeIndex)
        {
            cgltf_node* cnode = scene->nodes[nodeIndex];
            if (!cnode)
            {
                continue;
            }
            computeMeshMatrices(gltfMeshMatrixList, cnode);
        }
    }

	// Read in all meshes
	for (size_t m = 0; m < data->meshes_count; m++)
	{
		cgltf_mesh* cmesh = &(data->meshes[m]);
		if (!cmesh)
		{
			continue;
		}
		Matrix44 positionMatrix = Matrix44::IDENTITY;
		if (gltfMeshMatrixList.find(cmesh) != gltfMeshMatrixList.end())
		{
			positionMatrix = gltfMeshMatrixList[cmesh];
		}

		Vector3 boxMin = { MAX_FLOAT, MAX_FLOAT, MAX_FLOAT };
		Vector3 boxMax = { -MAX_FLOAT, -MAX_FLOAT, -MAX_FLOAT };

		std::string meshName = cmesh->name ? cmesh->name : EMPTY_STRING;
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
		MeshStreamPtr colorStream = nullptr;
		MeshStreamPtr texcoordStream = nullptr;
		MeshStreamPtr tangentStream = nullptr;

		for (cgltf_size primitiveIndex = 0; primitiveIndex < cmesh->primitives_count; ++primitiveIndex)
		{
			cgltf_primitive* primitive = &cmesh->primitives[primitiveIndex];

			if (!primitive)
			{
				continue;
			}

			cgltf_accessor* indexAccessor = primitive->indices;
			if (indexAccessor &&
				indexAccessor->component_type != cgltf_primitive_type_triangles)
			{
				if (_debugLevel > 0)
					std::cout << "Skip non triangle indexed mesh: " << cmesh->name << std::endl;
				continue;
			}

			// Read in vertex streams
			for (cgltf_size prim = 0; prim < primitive->attributes_count; prim++)
			{
				cgltf_attribute* attribute = &primitive->attributes[prim];
				cgltf_accessor* accessor = attribute->data;
				if (!accessor)
				{
					continue;
				}
				// Only load one stream of each type for now.
				cgltf_int streamIndex = attribute->index;
				if (streamIndex != 0)
				{
					continue;
				}

				// Get data as floats
				cgltf_size floatCount = cgltf_accessor_unpack_floats(accessor, NULL, 0);
				std::vector<float> attributeData;
				attributeData.resize(floatCount);
				floatCount = cgltf_accessor_unpack_floats(accessor, &attributeData[0], floatCount);

				cgltf_size vectorSize = cgltf_num_components(accessor->type);
				size_t desiredVectorSize = 3;

				MeshStreamPtr geomStream = nullptr;

				bool isPositionStream = (attribute->type == cgltf_attribute_type_position);
				bool isNormalStream = (attribute->type == cgltf_attribute_type_normal);
				bool isTexCoordSteram = (attribute->type == cgltf_attribute_type_texcoord);
				if (isPositionStream)
				{
					// Create position stream
					positionStream = MeshStream::create("i_" + MeshStream::POSITION_ATTRIBUTE, MeshStream::POSITION_ATTRIBUTE, streamIndex);
					mesh->addStream(positionStream);
					geomStream = positionStream;
				}
				else if (attribute->type == cgltf_attribute_type_normal)
				{
					normalStream = MeshStream::create("i_" + MeshStream::NORMAL_ATTRIBUTE, MeshStream::NORMAL_ATTRIBUTE, streamIndex);
					mesh->addStream(normalStream);
					geomStream = normalStream;
				}
				else if (attribute->type == cgltf_attribute_type_tangent)
				{
					tangentStream = MeshStream::create("i_" + MeshStream::TANGENT_ATTRIBUTE, MeshStream::TANGENT_ATTRIBUTE, streamIndex);
					mesh->addStream(tangentStream);
					geomStream = tangentStream;
				}
				else if (attribute->type == cgltf_attribute_type_color)
				{
					colorStream = MeshStream::create("i_" + MeshStream::COLOR_ATTRIBUTE, MeshStream::COLOR_ATTRIBUTE, streamIndex);
					geomStream = colorStream;
					if (vectorSize == 4)
					{
						desiredVectorSize = 4;
					}
				}
				else if (attribute->type == cgltf_attribute_type_texcoord)
				{
					texcoordStream = MeshStream::create("i_" + MeshStream::TEXCOORD_ATTRIBUTE + "_0", MeshStream::TEXCOORD_ATTRIBUTE, 0);
					mesh->addStream(texcoordStream);
					if (vectorSize == 2)
					{
						texcoordStream->setStride(MeshStream::STRIDE_2D);
						desiredVectorSize = 2;
					}
					geomStream = texcoordStream;
				}
				else
				{
					if (_debugLevel > 0)
						std::cout << "Unknown stream type: " << std::to_string(attribute->type)
						<< std::endl;
				}

				// Fill in stream 
				if (geomStream)
				{
					MeshFloatBuffer& buffer = geomStream->getData();
					cgltf_size vertexCount = accessor->count;

					if (_debugLevel > 0)
					{
						std::cout << "** Read stream: " << geomStream->getName() << std::endl;
						std::cout << " - vertex count: " << std::to_string(vertexCount) << std::endl;
						std::cout << " - vector size: " << std::to_string(vectorSize) << std::endl;
					}

					for (cgltf_size i = 0; i < vertexCount; i++)
					{
						const float* input = &attributeData[vectorSize * i];
						if (isPositionStream)
						{
							Vector3 position;
							for (cgltf_size v = 0; v < desiredVectorSize; v++)
							{
								// Update bounding box
								float floatValue = (v < vectorSize) ? input[v] : 0.0f;
								position[v] = floatValue;
							}
							position = positionMatrix.transformPoint(position);
							for (cgltf_size v = 0; v < desiredVectorSize; v++)
							{
								buffer.push_back(position[v]);
								boxMin[v] = std::min(position[v], boxMin[v]);
								boxMax[v] = std::max(position[v], boxMax[v]);
							}
						}
						else if (isNormalStream)
						{
							Vector3 normal;
							for (cgltf_size v = 0; v < desiredVectorSize; v++)
							{
								float floatValue = (v < vectorSize) ? input[v] : 0.0f;
								normal[v] = floatValue;
							}
							normal = positionMatrix.transformNormal(normal);
							for (cgltf_size v = 0; v < desiredVectorSize; v++)
							{
								buffer.push_back(normal[v]);
							}
						}
						else
						{
							for (cgltf_size v = 0; v < desiredVectorSize; v++)
							{
								float floatValue = (v < vectorSize) ? input[v] : 0.0f;
								// Perform v-flip
								if (isTexCoordSteram && v == 1)
								{
									if (!texcoordVerticalFlip)
									{ 
										floatValue = 1.0f - floatValue;
									}
								}
								buffer.push_back(floatValue);
							}
						}
					}
				}

				// Read indexing
				MeshPartitionPtr part = MeshPartition::create();
				size_t indexCount = 0;
				if (indexAccessor)
				{
					indexCount = indexAccessor->count;
				}
				else if (positionStream)
				{
					indexCount = positionStream->getData().size();;
				}
				size_t faceCount = indexCount / FACE_VERTEX_COUNT;
				part->setFaceCount(faceCount);
				part->setName(meshName);

				MeshIndexBuffer& indices = part->getIndices();
				if (_debugLevel > 0)
					std::cout << "** Read indexing: Count = " << std::to_string(indexCount) << std::endl;
				if (indexAccessor)
				{
					for (cgltf_size i = 0; i < indexCount; i++)
					{
						uint32_t vertexIndex = static_cast<uint32_t>
							(cgltf_accessor_read_index(indexAccessor, i));
						indices.push_back(vertexIndex);
					}
				}
				else
				{
					for (cgltf_size i = 0; i < indexCount; i++)
					{
						indices.push_back(static_cast<uint32_t>(i));
					}
				}
				mesh->addPartition(part);

				// TODO handle materials
				cgltf_material* material = primitive->material;
				if (material)
				{

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
	}

    if (data)
    {
        cgltf_free(data);
    }

	return true;
}

MATERIALX_NAMESPACE_END
