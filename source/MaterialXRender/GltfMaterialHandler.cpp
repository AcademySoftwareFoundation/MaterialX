
#include <MaterialXCore/Value.h>
#include <MaterialXCore/Types.h>
#include <MaterialXCore/Library.h>
#include <MaterialXFormat/XmlIo.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXGenShader/Util.h>
#include <MaterialXGenShader/ShaderTranslator.h>
#include <MaterialXRender/StbImageLoader.h>
#include <MaterialXRender/GltfMaterialHandler.h>

#include <algorithm>

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
    #ifndef __clang__
        #if __GNUC_PREREQ(12,0)
            #pragma GCC diagnostic ignored "-Wformat-truncation"
        #endif
    #endif
#endif

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable : 4996)
#endif

#undef CGLTF_IMPLEMENTATION //-- don't set to avoid duplicate symbols
#include <MaterialXRender/External/Cgltf/cgltf.h>
#define CGLTF_WRITE_IMPLEMENTATION
#include <MaterialXRender/External/Cgltf/cgltf_write.h>

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

#include <cstring>
#include <limits>
#include <algorithm>
#include <sstream>
#include <iterator>

#include <iostream>

MATERIALX_NAMESPACE_BEGIN

namespace
{
const float TO_DEGREE = 180.0f / 3.1415926535f;
//const float TO_RADIAN = 3.1415926535f / 180.0f;
const std::string SPACE_STRING = " ";
const std::string IN_STRING = "in";
const std::string FLOAT_STRING = "float";
const std::string VEC3_STRING = "vector3";
using GLTFMaterialMeshList = std::unordered_map<string,string>;
const std::string DEFAULT_NODE_PREFIX = "NODE_";
const std::string DEFAULT_MESH_PREFIX = "MESH_";
const std::string DEFAULT_MATERIAL_NAME = "MATERIAL_0";
const std::string DEFAULT_SHADER_NAME = "SHADER_0";

void initialize_cgltf_texture_view(cgltf_texture_view& textureview)
{
    std::memset(&textureview, 0, sizeof(cgltf_texture_view));
    textureview.texture = nullptr;
    textureview.scale = 1.0;
    textureview.has_transform = false;
    textureview.extras.start_offset = 0;
    textureview.extras.end_offset = 0;
    textureview.extensions_count = 0;
    textureview.extensions = nullptr;
}

void initialize_cgtlf_texture(cgltf_texture& texture, const string& name, const string& uri, 
                              cgltf_image* image)
{
    std::memset(&texture, 0, sizeof(cgltf_texture));
    texture.has_basisu = false;
    texture.extras.start_offset = 0;
    texture.extras.end_offset = 0;
    texture.extensions_count = 0;
    texture.sampler = nullptr;
    texture.image = image;
    texture.image->extras.start_offset = 0;
    texture.image->extras.end_offset = 0;
    texture.image->extensions_count = 0;
    texture.image->buffer_view = nullptr;
    texture.image->mime_type = nullptr;
    texture.image->name = const_cast<char*>((new string(name))->c_str());
    texture.name = texture.image->name;
    FilePath uriPath = uri;
    texture.image->uri = const_cast<char*>((new string(uriPath.asString(FilePath::FormatPosix)))->c_str());
}

void writeTexcoordIndex(cgltf_texture_view& texture_view, NodePtr imageNode)
{
    // Check for any upstream `texcoord` node and grab it's uv index if not 0.
    int uvindex = 0;
    InputPtr texcoordInput = imageNode->getInput("texcoord");
    if (texcoordInput)
    {
        NodePtr texcoordNode = texcoordInput->getConnectedNode();
        if (texcoordNode)
        {
            InputPtr uvindexInput = texcoordNode->getInput("index");
            if (uvindexInput)
            {
                ValuePtr value = uvindexInput->getValue();
                if (value)
                {
                    uvindex = value->asA<int>();
                }
            }
        }
    }
    texture_view.texcoord = static_cast<cgltf_int>(uvindex);
}

void writeColor3Input(const NodePtr pbrNode, const string& inputName, 
                        cgltf_texture_view& texture_view, 
                        cgltf_float* write_value,
                        cgltf_bool& hasFlag,
                        std::vector<cgltf_texture>& textureList,
                        std::vector<cgltf_image>& imageList, size_t& imageIndex)
{
    string filename;

    NodePtr imageNode = pbrNode->getConnectedNode(inputName);
    if (imageNode)
    {
        InputPtr fileInput = imageNode->getInput(Implementation::FILE_ATTRIBUTE);
        filename = fileInput && fileInput->getAttribute(TypedElement::TYPE_ATTRIBUTE) == FILENAME_TYPE_STRING ?
            fileInput->getResolvedValueString() : EMPTY_STRING;
        if (filename.empty())
        {
            imageNode = nullptr;
        }
    }
    if (imageNode)
    {
        cgltf_texture* texture = &(textureList[imageIndex]);
        texture_view.texture = texture;
        initialize_cgtlf_texture(*texture, imageNode->getNamePath(), filename,
                                 &(imageList[imageIndex]));
        writeTexcoordIndex(texture_view, imageNode);

        write_value[0] = 1.0f;
        write_value[1] = 1.0f;
        write_value[2] = 1.0f;

        imageIndex++;
        hasFlag = false;
    }
    else
    {
        ValuePtr value = pbrNode->getInputValue(inputName);
        if (value)
        {
            Color3 color = value->asA<Color3>();
            write_value[0] = color[0];
            write_value[1] = color[1];
            write_value[2] = color[2];

            hasFlag = false;
        }
    }
}

void writeFloatInput(const NodePtr pbrNode, const string& inputName, 
                        cgltf_texture_view& texture_view, 
                        float* write_value,
                        cgltf_bool& hasFlag,
                        std::vector<cgltf_texture>& textureList,
                        std::vector<cgltf_image>& imageList, size_t& imageIndex)
{
    string filename;

    NodePtr imageNode = pbrNode->getConnectedNode(inputName);
    if (imageNode)
    {
        InputPtr fileInput = imageNode->getInput(Implementation::FILE_ATTRIBUTE);
        filename = fileInput && fileInput->getAttribute(TypedElement::TYPE_ATTRIBUTE) == FILENAME_TYPE_STRING ?
            fileInput->getResolvedValueString() : EMPTY_STRING;
        if (filename.empty())
        {
            imageNode = nullptr;
        }
    }
    if (imageNode)
    {
        cgltf_texture* texture = &(textureList[imageIndex]);
        texture_view.texture = texture;
        // Fix this to create a valid name...
        initialize_cgtlf_texture(*texture, imageNode->getNamePath(), filename,
                                 &(imageList[imageIndex]));
        writeTexcoordIndex(texture_view, imageNode);

        *write_value = 1.0f;
        imageIndex++;
        hasFlag = true;
    }
    else
    {
        ValuePtr value = pbrNode->getInputValue(inputName);
        if (value)
        {
            *write_value = value->asA<float>();
            hasFlag = true;
        }
    }
}

void computeMeshMaterials(GLTFMaterialMeshList& materialMeshList, StringSet& materialCPVList, void* cnodeIn, FilePath& path, unsigned int nodeCount,
                          unsigned int meshCount)
{
    cgltf_node* cnode = static_cast<cgltf_node*>(cnodeIn);

    // Push node name on to path
    FilePath prevPath = path;
    string cnodeName = cnode->name ? string(cnode->name) : DEFAULT_NODE_PREFIX + std::to_string(nodeCount++);
    path = path / ( createValidName(cnodeName) + "/" );
    cgltf_mesh* cmesh = cnode->mesh;
    if (cmesh)
    {
        // Set path to mesh if no transform path found
        if (path.isEmpty())
        {
            string meshName = cmesh->name ? string(cmesh->name) : DEFAULT_MESH_PREFIX + std::to_string(meshCount++);
            path = createValidName(meshName);
        }

        cgltf_primitive* prim = cmesh->primitives;
        if (prim && prim->material)
        {
            cgltf_material* material = prim->material;
            if (material)
            {
                // Check for CPV
                bool requiresCPV = false;
                for (cgltf_size primitiveIndex = 0; primitiveIndex < cmesh->primitives_count; ++primitiveIndex)
                {
                    cgltf_primitive* primitive = &cmesh->primitives[primitiveIndex];
                    if (!primitive)
                    {
                        continue;
                    }
                    for (cgltf_size attr = 0; attr < primitive->attributes_count; attr++)
                    {
                        cgltf_attribute* attribute = &primitive->attributes[attr];
                        if (attribute->type == cgltf_attribute_type_color)
                        {
                            requiresCPV = true;
                            break;
                        }
                    }
                }

                // Add reference to mesh (by name) to material 
                string stringPath = path.asString(FilePath::FormatPosix);
                string materialName = material->name;
                if (materialMeshList.count(materialName))
                {
                    materialMeshList[materialName].append(", " + stringPath);
                }
                else
                {
                    materialMeshList.insert({ materialName, stringPath });
                }

                if (requiresCPV)
                {
                    materialCPVList.insert(materialName);
                }
            }
        }
    }

    for (cgltf_size i = 0; i < cnode->children_count; i++)
    {
        if (cnode->children[i])
        {
            computeMeshMaterials(materialMeshList, materialCPVList, cnode->children[i], path, nodeCount, meshCount);
        }
    }

    // Pop path name
    path = prevPath;
}

}

void GltfMaterialHandler::translateShaders(DocumentPtr doc, StringVec& logger)
{
    if (!doc)
    {
        return;
    }

    // Perform translation to MaterialX gltf_pbr if it exists.
    // This basically expands ShaderTranslator::translateAllMaterials()
    // so that documents with mixed shader types can be handled correctly.
    //
    const string TARGET_GLTF = "gltf_pbr";
    ShaderTranslatorPtr translator = ShaderTranslator::create();
    vector<TypedElementPtr> materialNodes = findRenderableMaterialNodes(doc);
    for (auto elem : materialNodes)
    {
        NodePtr materialNode = elem->asA<Node>();
        if (!materialNode)
        {
            continue;
        }
        for (NodePtr shaderNode : getShaderNodes(materialNode))
        {
            try
            {
                const string& sourceCategory = shaderNode->getCategory();
                if (sourceCategory == TARGET_GLTF)
                {
                    continue;
                }
                string translateNodeString = sourceCategory + "_to_" + TARGET_GLTF;
                vector<NodeDefPtr> matchingNodeDefs = doc->getMatchingNodeDefs(translateNodeString);
                if (matchingNodeDefs.empty())
                {
                    continue;
                }
                translator->translateShader(shaderNode, TARGET_GLTF);
            }
            catch (std::exception& e)
            {
                logger.push_back("- Error in shader translation: " + string(e.what()));
            }
        }
    }
}

bool GltfMaterialHandler::save(const FilePath& filePath, StringVec& logger)
{
    if (!_materials)
    {
        return false;
    }

    const string input_filename = filePath.asString();
    const string ext = stringToLower(filePath.getExtension());
    const string BINARY_EXTENSION = "glb";
    const string ASCII_EXTENSION = "gltf";
    if (ext != BINARY_EXTENSION && ext != ASCII_EXTENSION)
    {
        return false;
    }

    cgltf_options options;
    std::memset(&options, 0, sizeof(options));
    cgltf_data* data = new cgltf_data();
	data->file_type = (ext == BINARY_EXTENSION) ? cgltf_file_type::cgltf_file_type_glb : cgltf_file_type::cgltf_file_type_gltf;
    data->file_data = nullptr;
	//cgltf_asset asset;
	data->meshes = nullptr;
	data->meshes_count = 0;
	data->materials = nullptr;
	data->materials_count = 0;
	data->accessors = nullptr;
	data->accessors_count = 0;
	data->buffer_views = nullptr;
	data->buffer_views_count = 0;
	data->buffers = nullptr;
	data->buffers_count = 0;
	data->images = nullptr;
	data->images_count = 0;
	data->textures = nullptr;
	data->textures_count = 0;
	data->samplers = nullptr;
	data->samplers_count = 0;
	data->skins = nullptr;
    data->skins_count = 0;
    data->cameras = nullptr;
	data->cameras_count = 0;
	data->lights = nullptr;
	data->lights_count = 0;
	data->nodes = nullptr;
	data->nodes_count = 0;
	data->scenes = nullptr;
    data->scenes_count = 0;
	data->scene = nullptr;
	data->animations = nullptr;
	data->animations_count = 0;
	data->variants = nullptr;
	data->variants_count = 0;
	//cgltf_extras extras;
	data->data_extensions_count = 0;
	data->data_extensions = nullptr;
	data->extensions_used = nullptr;
	data->extensions_used_count = 0;
	data->extensions_required = nullptr;
	data->extensions_required_count = 0;
	data->json = nullptr;
	data->json_size = 0;
	data->bin = nullptr;
	data->bin_size = 0;

    const string mtlx_versionString = getVersionString();
    const string gltf_versionString = "2.0";
	data->asset.generator = const_cast<char*>((new string("MaterialX " + mtlx_versionString + " to glTF " + gltf_versionString + " generator. https://github.com/kwokcb/glTF_MaterialX"))->c_str());;
    data->asset.version = const_cast<char*>((new string(gltf_versionString))->c_str());
    std::string *copyright = new string("Copyright 2022-2023: Bernard Kwok");
    data->asset.copyright = const_cast<char*>((copyright)->c_str());

    // Scan for PBR shader nodes
    const string PBR_CATEGORY_STRING("gltf_pbr");
    const string UNLIT_CATEGORY_STRING("surface_unlit");
    std::set<NodePtr> pbrNodes;
    std::set<NodePtr> unlitNodes;
    for (const NodePtr& material : _materials->getMaterialNodes())
    {
        vector<NodePtr> shaderNodes = getShaderNodes(material);
        for (const NodePtr& shaderNode : shaderNodes)
        {
            const string& category = shaderNode->getCategory();
            if (category == PBR_CATEGORY_STRING && pbrNodes.find(shaderNode) == pbrNodes.end())
            {
                pbrNodes.insert(shaderNode);
            }
            else if (category == UNLIT_CATEGORY_STRING && unlitNodes.find(shaderNode) == unlitNodes.end())
            {
                unlitNodes.insert(shaderNode);
            }
        }
    }

    cgltf_size materials_count = pbrNodes.size() + unlitNodes.size();
    if (!materials_count)
    {
        return false;
    }

    // Write materials
    // TODO: Convert absoluate image paths to relative paths to aoivd
    // warnings. Can be done outside as well.
    /*
    typedef struct cgltf_material
    {
	    char* name;
	    cgltf_bool has_pbr_metallic_roughness;
	    cgltf_bool has_pbr_specular_glossiness;
	    cgltf_bool has_clearcoat;
	    cgltf_bool has_transmission;
	    cgltf_bool has_volume;
	    cgltf_bool has_ior; 
	    cgltf_bool has_specular;
	    cgltf_bool has_sheen;
	    cgltf_bool has_emissive_strength;
	    cgltf_bool has_iridescence;
	    cgltf_pbr_metallic_roughness pbr_metallic_roughness; // DONE
	    cgltf_pbr_specular_glossiness pbr_specular_glossiness; // Not applicable
	    cgltf_clearcoat clearcoat; // DONE
	    cgltf_ior ior; // DONE
	    cgltf_specular specular; // DONE
	    cgltf_sheen sheen; // DONE
	    cgltf_transmission transmission; // DONE
	    cgltf_volume volume; // NOT HANDLED
	    cgltf_emissive_strength emissive_strength; // DONE
	    cgltf_iridescence iridescence; // DONE
	    cgltf_texture_view normal_texture; // DONE
	    cgltf_texture_view occlusion_texture; // DONE
	    cgltf_texture_view emissive_texture; // DONE
	    cgltf_float emissive_factor[3];
	    cgltf_alpha_mode alpha_mode; // DONE
	    cgltf_float alpha_cutoff; // DONE
	    cgltf_bool double_sided; // NOT APPLICABLE 
	    cgltf_bool unlit; // DONE
	    cgltf_extras extras;
	    cgltf_size extensions_count;
	    cgltf_extension* extensions;
    } cgltf_material;
    */
    cgltf_material* materials = new cgltf_material[materials_count];
    data->materials = materials;
    data->materials_count = materials_count;

    // Set of image nodes.
    std::vector<cgltf_texture> textureList;
    textureList.resize(64);
    std::vector<cgltf_image> imageList;
    imageList.resize(64);

    size_t material_idx = 0;
    size_t imageIndex = 0;

    // Handle unlit nodes
    for (const NodePtr& unlitNode : unlitNodes)
    {
        cgltf_material* material = &(materials[material_idx]);
        std::memset(material, 0, sizeof(cgltf_material));
        material->unlit = true;
	    material->has_pbr_metallic_roughness = false;
	    material->has_pbr_specular_glossiness = false;
	    material->has_clearcoat = false;
	    material->has_transmission = false;
	    material->has_volume = false;
	    material->has_ior = false;
	    material->has_specular = false;
	    material->has_sheen = false;
	    material->has_emissive_strength = false;
	    material->extensions_count = 0;
	    material->extensions = nullptr;
        material->emissive_texture.texture = nullptr;
        material->normal_texture.texture = nullptr;
        material->occlusion_texture.texture = nullptr;

        string* name = new string(unlitNode->getNamePath());
        material->name = const_cast<char*>(name->c_str());
        
        material->has_pbr_metallic_roughness = true;
        cgltf_pbr_metallic_roughness& roughness = material->pbr_metallic_roughness;
        initialize_cgltf_texture_view(roughness.base_color_texture);

        // Handle base color
        ValuePtr value = unlitNode->getInputValue("emission_color");
        if (value)
        {
            Color3 color = value->asA<Color3>();
            roughness.base_color_factor[0] = color[0];
            roughness.base_color_factor[1] = color[1];
            roughness.base_color_factor[2] = color[2];
        }

        value = unlitNode->getInputValue("opacity");
        if (value)
        {
            roughness.base_color_factor[3] = value->asA<float>();
        }

        material_idx++;
    }

    // Handle gltf pbr nodes
    for (const NodePtr& pbrNode : pbrNodes)
    {
        cgltf_material* material = &(materials[material_idx]);
        std::memset(material, 0, sizeof(cgltf_material));
	    material->has_pbr_metallic_roughness = false;
	    material->has_pbr_specular_glossiness = false;
	    material->has_clearcoat = false;
	    material->has_transmission = false;
	    material->has_volume = false;
	    material->has_ior = false;
	    material->has_specular = false;
	    material->has_sheen = false;
	    material->has_emissive_strength = false;
	    material->extensions_count = 0;
	    material->extensions = nullptr;
        material->emissive_texture.texture = nullptr;
        material->normal_texture.texture = nullptr;
        material->occlusion_texture.texture = nullptr;

        string* name = new string(pbrNode->getNamePath());
        material->name = const_cast<char*>(name->c_str());
        
        material->has_pbr_metallic_roughness = true;
        cgltf_pbr_metallic_roughness& roughness = material->pbr_metallic_roughness;
        initialize_cgltf_texture_view(roughness.base_color_texture);

        string filename;

        // Handle base color
        NodePtr imageNode = pbrNode->getConnectedNode("base_color");
        if (imageNode)
        {
            InputPtr fileInput = imageNode->getInput(Implementation::FILE_ATTRIBUTE);
            filename = fileInput && fileInput->getAttribute(TypedElement::TYPE_ATTRIBUTE) == FILENAME_TYPE_STRING ?
                fileInput->getResolvedValueString() : EMPTY_STRING;
            if (filename.empty())
                imageNode = nullptr;
        }
        if (imageNode)
        {
            cgltf_texture* texture = &(textureList[imageIndex]);
            roughness.base_color_texture.texture = texture;
            initialize_cgtlf_texture(*texture, imageNode->getNamePath(), filename,
                &(imageList[imageIndex]));            
            writeTexcoordIndex(roughness.base_color_texture, imageNode);

            roughness.base_color_factor[0] = 1.0;
            roughness.base_color_factor[1] = 1.0;
            roughness.base_color_factor[2] = 1.0;
            roughness.base_color_factor[3] = 1.0;

            imageIndex++;

            // Pull off color from gltf_colorImage node
            ValuePtr value = pbrNode->getInputValue(COLOR_SEMANTIC);
            if (value && value->isA<Color4>())
            {
                Color4 color = value->asA<Color4>();
                roughness.base_color_factor[0] = color[0];
                roughness.base_color_factor[1] = color[1];
                roughness.base_color_factor[2] = color[2];
                roughness.base_color_factor[3] = color[3];
            }
        }
        else
        {
            ValuePtr value = pbrNode->getInputValue("base_color");
            if (value)
            {
                Color3 color = value->asA<Color3>();
                roughness.base_color_factor[0] = color[0];
                roughness.base_color_factor[1] = color[1];
                roughness.base_color_factor[2] = color[2];
            }

            value = pbrNode->getInputValue("alpha");
            if (value)
            {
                roughness.base_color_factor[3] = value->asA<float>();
            }
        }

        // Handle metallic, roughness, occlusion
        // Handle partially mapped or when different channels map to different images
        // by merging into a single ORM image. Note that we save as BGR 24-bit fixed images
        // thus we scan by that order which results in an MRO image being written to disk.
        initialize_cgltf_texture_view(roughness.metallic_roughness_texture);
        ValuePtr value;
        string extractInputs[3] =
        {
            "metallic",
            "roughness",
            "occlusion"
        };
        FilePath filenames[3] =
        {
            EMPTY_STRING, EMPTY_STRING, EMPTY_STRING
        };
        string imageNamePaths[3] =
        {
            EMPTY_STRING, EMPTY_STRING, EMPTY_STRING
        };
        cgltf_float* roughnessInputs[3] =
        {
            &roughness.metallic_factor,
            &roughness.roughness_factor,
            nullptr
        };

        NodePtr ormNode= nullptr;
        imageNode = nullptr;
        const string extractCategory("extract");
        for (size_t e = 0; e < 3; e++)
        {
            const string& inputName = extractInputs[e];
            InputPtr pbrInput = pbrNode->getInput(inputName);
            if (pbrInput)
            {
                // Read past any extract node
                NodePtr connectedNode = pbrNode->getConnectedNode(inputName);
                if (connectedNode)
                {
                    if (connectedNode->getCategory() == extractCategory)
                    {
                        imageNode = connectedNode->getConnectedNode(IN_STRING);
                    }
                    else
                    {
                        imageNode = connectedNode;
                    }
                }

                if (imageNode)
                {
                    InputPtr fileInput = imageNode->getInput(Implementation::FILE_ATTRIBUTE);
                    filename = fileInput && fileInput->getAttribute(TypedElement::TYPE_ATTRIBUTE) == FILENAME_TYPE_STRING ?
                        fileInput->getResolvedValueString() : EMPTY_STRING;
                    filenames[e] = filename;
                    imageNamePaths[e] = imageNode->getNamePath();
                }

                // Write out constant factors. If there is an image node
                // then ignore any value stored as the image takes precedence.
                if (roughnessInputs[e])
                {
                    value = pbrInput->getValue();
                    if (value)
                    {
                        *(roughnessInputs[e]) = value->asA<float>();
                    }
                    else
                    {
                        *(roughnessInputs[e]) = 1.0f;
                    }
                }
            }

            // Set to default 1.0
            else
            {
                if (roughnessInputs[e])
                {
                    *(roughnessInputs[e]) = 1.0f;
                }
            }
        }

        // Determine how many images to export and if merging is required
        const FilePath metallicFilename = filenames[0];
        const FilePath roughnessFilename = filenames[1];
        const FilePath occlusionFilename = filenames[2];

        if (metallicFilename == roughnessFilename)
        {
            // Write one texture if filenames are all the same and not empty
            if (roughnessFilename == occlusionFilename)
            {
                if (!metallicFilename.isEmpty())
                {
                    logger.push_back("--> write SINGLE file for MRO: " + imageNamePaths[0]);
                    cgltf_texture* texture = &(textureList[imageIndex]);
                    roughness.metallic_roughness_texture.texture = texture;
                    material->occlusion_texture.texture = texture; // needed ?
                    initialize_cgtlf_texture(*texture, imageNamePaths[0], metallicFilename,
                        &(imageList[imageIndex]));
                    imageIndex++;
                }
            }

            else
            {
                // if metallic and roughness match but occlusion differs, Then export 2 textures if found
                if (!metallicFilename.isEmpty())
                {
                    logger.push_back("--> write SINGLE for MR only: " + imageNamePaths[0]);
                    cgltf_texture* texture = &(textureList[imageIndex]);
                    roughness.metallic_roughness_texture.texture = texture;
                    initialize_cgtlf_texture(*texture, imageNamePaths[0], metallicFilename,
                        &(imageList[imageIndex]));
                    imageIndex++;
                }

                if (!occlusionFilename.isEmpty())
                {
                    logger.push_back("  --> AND write SINGLE for O only: " + imageNamePaths[0]);
                    cgltf_texture* texture = &(textureList[imageIndex]);
                    material->occlusion_texture.texture = texture;
                    initialize_cgtlf_texture(*texture, imageNamePaths[2], occlusionFilename,
                        &(imageList[imageIndex]));
                    imageIndex++;
                }
            }
        }

        // Metallic and roughness do no match and one or both are images. Merge as necessary
        else if (!metallicFilename.isEmpty() || !roughnessFilename.isEmpty())
        {
            ImageLoaderPtr loader = StbImageLoader::create();
            if (loader)
            {
                FilePath ormFilename = metallicFilename.isEmpty() ? roughnessFilename : metallicFilename;

                logger.push_back("  --> Write MERGEED metallic and roughness: " + ormFilename.asString());

                Color4 color(0.0f);
                unsigned int imageWidth = 0;
                unsigned int imageHeight = 0;

                ImagePtr roughnessImage = !roughnessFilename.isEmpty() ? loader->loadImage(roughnessFilename) : nullptr;
                if (roughnessImage)
                {
                    imageWidth = std::max(roughnessImage->getWidth(), imageWidth);
                    imageHeight = std::max(roughnessImage->getWidth(), imageWidth);
                }
                ImagePtr metallicImage = !metallicFilename.isEmpty() ? loader->loadImage(metallicFilename) : nullptr;
                if (metallicImage)
                {
                    imageWidth = std::max(metallicImage->getWidth(), imageWidth);
                    imageHeight = std::max(metallicImage->getWidth(), imageWidth);
                }
                logger.push_back("  --> Find image size: w, h: " + std::to_string(imageWidth) +
                    std::to_string(imageHeight));

                ImagePtr outputImage = nullptr;
                if (imageWidth * imageHeight != 0)
                {
                    outputImage = createUniformImage(imageWidth, imageHeight, 3, Image::BaseType::UINT8, color);

                    logger.push_back("  -----> Merge roughness image: " + roughnessFilename.asString());
                    float uniformColor = roughness.roughness_factor;
                    if (roughnessImage)
                    {
                        roughness.roughness_factor = 1.0f;
                    }
                    for (unsigned int y = 0; y < imageHeight; y++)
                    {
                        for (unsigned int x = 0; x < imageWidth; x++)
                        {
                            Color4 finalColor = outputImage->getTexelColor(x, y);
                            finalColor[1] = roughnessImage ? roughnessImage->getTexelColor(x, y)[0] : uniformColor;
                            outputImage->setTexelColor(x, y, finalColor);
                        }
                    }

                    logger.push_back("  -----> Merge metallic image: " +  metallicFilename.asString());
                    uniformColor = roughness.metallic_factor;
                    if (metallicImage)
                    {
                        roughness.metallic_factor = 1.0f;
                    }
                    for (unsigned int y = 0; y < imageHeight; y++)
                    {
                        for (unsigned int x = 0; x < imageWidth; x++)
                        {
                            Color4 finalColor = outputImage->getTexelColor(x, y);
                            finalColor[2] = metallicImage ? metallicImage->getTexelColor(x, y)[0] : uniformColor;
                            outputImage->setTexelColor(x, y, finalColor);
                        }
                    }

                    ormFilename.removeExtension();
                    FilePath ormfilePath = ormFilename.asString() + "_combined.png";
                    bool saved = loader->saveImage(filePath, outputImage);
                    logger.push_back("  --> Write ORM image to disk: " +  filePath.asString() + 
                        ". SUCCESS: " +  std::to_string(saved));

                    cgltf_texture* texture = &(textureList[imageIndex]);
                    roughness.metallic_roughness_texture.texture = texture;
                    initialize_cgtlf_texture(*texture, imageNode->getNamePath(), ormfilePath,
                        &(imageList[imageIndex]));
                    imageIndex++;
                    logger.push_back("  --> Write cgltf image name: " +  ormfilePath.asString());
                }
            }
        }

        // Handle normal
        filename = EMPTY_STRING;
        imageNode = pbrNode->getConnectedNode("normal");
        initialize_cgltf_texture_view(material->normal_texture);
        if (imageNode)
        {
            // Read past normalmap node
            if (imageNode->getCategory() == "normalmap")
            {
                imageNode = imageNode->getConnectedNode(IN_STRING);
            }
            if (imageNode)
            {
                InputPtr fileInput = imageNode->getInput(Implementation::FILE_ATTRIBUTE);
                filename = fileInput && fileInput->getAttribute(TypedElement::TYPE_ATTRIBUTE) == FILENAME_TYPE_STRING ?
                    fileInput->getResolvedValueString() : EMPTY_STRING;
                if (filename.empty())
                    imageNode = nullptr;
            }
        }
        if (imageNode)
        {
            cgltf_texture* texture = &(textureList[imageIndex]);
            material->normal_texture.texture = texture;
            initialize_cgtlf_texture(*texture, imageNode->getNamePath(), filename,
                &(imageList[imageIndex]));
            writeTexcoordIndex(material->normal_texture, imageNode);

            imageIndex++;
        }

        // Handle transmission
        cgltf_transmission& transmission = material->transmission;
        writeFloatInput(pbrNode, "transmission",
            transmission.transmission_texture, &(transmission.transmission_factor),
            material->has_transmission, textureList, imageList, imageIndex);

        // Handle specular color
        cgltf_specular& specular = material->specular;
        writeColor3Input(pbrNode, "specular_color",
            specular.specular_color_texture, &(specular.specular_color_factor[0]), material->has_specular,
            textureList, imageList, imageIndex);
        // - Handle specular
        writeFloatInput(pbrNode, "specular",
            specular.specular_texture, &(specular.specular_factor), material->has_specular, 
            textureList, imageList, imageIndex);

        // Handle ior
        value = pbrNode->getInputValue("ior");
        if (value)
        {
            material->ior.ior = value->asA<float>();
            material->has_ior = true;
        }

        // Handle alphA mode, cutoff
        cgltf_alpha_mode& alpha_mode = material->alpha_mode;
        value = pbrNode->getInputValue("alpha_mode");
        if (value)
        {
            alpha_mode = static_cast<cgltf_alpha_mode>(value->asA<int>());
        }
        value = pbrNode->getInputValue("alpha_cutoff");
        if (value)
        {
            material->alpha_cutoff = value->asA<float>();
        }

        // Handle iridescence
        cgltf_iridescence& iridescence = material->iridescence;
        iridescence.iridescence_ior = 1.3f;
        iridescence.iridescence_factor = 0.0f;
        writeFloatInput(pbrNode, "iridescence",
            iridescence.iridescence_texture, &(iridescence.iridescence_factor),
            material->has_iridescence, textureList, imageList, imageIndex);
            
        // Scan for upstream <gltf_iridescence_thickness> node.
        // Note: This is the agreed upon upstream node to create to map
        // to gltf_pbr as part of the core implementation. It basically
        // represents this structure: 
        // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_iridescence/README.md
        iridescence.iridescence_thickness_max = 400.0f;
        iridescence.iridescence_thickness_min = 100.0f;
        initialize_cgltf_texture_view(iridescence.iridescence_thickness_texture);

        InputPtr thicknessInput = pbrNode->getInput("iridescence_thickness");
        if (thicknessInput)
        {
            NodePtr thicknessNode = thicknessInput->getConnectedNode();
            FilePath thicknessFileName;
            if (thicknessNode)
            {
                InputPtr fileInput = thicknessNode->getInput(Implementation::FILE_ATTRIBUTE);
                thicknessFileName = fileInput && fileInput->getAttribute(TypedElement::TYPE_ATTRIBUTE) == FILENAME_TYPE_STRING ?
                    fileInput->getResolvedValueString() : EMPTY_STRING;

                cgltf_texture* texture = &(textureList[imageIndex]);
                iridescence.iridescence_thickness_texture.texture = texture;
                initialize_cgtlf_texture(*texture, thicknessNode->getNamePath(), thicknessFileName,
                    &(imageList[imageIndex]));
                writeTexcoordIndex(iridescence.iridescence_thickness_texture, thicknessNode);
                imageIndex++;

                InputPtr thickessInput = thicknessNode->getInput("thicknessMin");
                ValuePtr thicknessValue = thickessInput ? thickessInput->getValue() : nullptr;
                if (thicknessValue)
                {
                    iridescence.iridescence_thickness_min = thicknessValue->asA<float>();
                }
                thickessInput = thicknessNode->getInput("thicknessMax");
                thicknessValue = thickessInput ? thickessInput->getValue() : nullptr;
                if (thicknessValue)
                {
                    iridescence.iridescence_thickness_max =  thicknessValue->asA<float>();
                }
            }
        }

        // Handle sheen color
        cgltf_sheen& sheen = material->sheen;
        writeColor3Input(pbrNode, "sheen_color",
            sheen.sheen_color_texture, &(sheen.sheen_color_factor[0]),
            material->has_sheen, textureList, imageList, imageIndex);
        // - Handle sheen roughness
        writeFloatInput(pbrNode, "sheen_roughness",
            sheen.sheen_roughness_texture, &(sheen.sheen_roughness_factor),
            material->has_sheen, textureList, imageList, imageIndex);

        // Handle clearcloat
        cgltf_clearcoat& clearcoat = material->clearcoat;
        writeFloatInput(pbrNode, "clearcoat",
            clearcoat.clearcoat_texture, &(clearcoat.clearcoat_factor),
            material->has_clearcoat, textureList, imageList, imageIndex);
        writeFloatInput(pbrNode, "clearcoat_roughness",
            clearcoat.clearcoat_roughness_texture, &(clearcoat.clearcoat_roughness_factor),
            material->has_clearcoat, textureList, imageList, imageIndex);

        // Handle clearcoat normal
        filename = EMPTY_STRING;
        imageNode = pbrNode->getConnectedNode("clearcoat_normal");
        initialize_cgltf_texture_view(material->normal_texture);
        if (imageNode)
        {
            // Read past normalmap node
            if (imageNode->getCategory() == "normalmap")
            {
                imageNode = imageNode->getConnectedNode(IN_STRING);
            }
            if (imageNode)
            {
                InputPtr fileInput = imageNode->getInput(Implementation::FILE_ATTRIBUTE);
                filename = fileInput && fileInput->getAttribute(TypedElement::TYPE_ATTRIBUTE) == FILENAME_TYPE_STRING ?
                    fileInput->getResolvedValueString() : EMPTY_STRING;
                if (filename.empty())
                    imageNode = nullptr;
            }
        }
        if (imageNode)
        {
            cgltf_texture* texture = &(textureList[imageIndex]);
            clearcoat.clearcoat_normal_texture.texture = texture;
            initialize_cgtlf_texture(*texture, imageNode->getNamePath(), filename,
                &(imageList[imageIndex]));
            writeTexcoordIndex(material->normal_texture, imageNode);

            imageIndex++;
            material->has_clearcoat = true;
        }

        // Handle emissive
        cgltf_bool dummy = false;
        writeColor3Input(pbrNode, "emissive",
            material->emissive_texture, &(material->emissive_factor[0]),
            dummy, textureList, imageList, imageIndex);
        // - Handle emissive strength
        value = pbrNode->getInputValue("emissive_strength");
        if (value)
        {
            material->emissive_strength.emissive_strength = value->asA<float>();
            material->has_emissive_strength = true;
        }

        material_idx++;
    }

    // Set image and texture lists
    data->images_count = imageIndex;
    data->images = &imageList[0];
    data->textures_count = imageIndex; 
    data->textures = &textureList[0];

    // Write to disk
    cgltf_result result = cgltf_write_file(&options, filePath.asString().c_str(), data);
    if (result != cgltf_result_success)
    {
        return false;
    }
    return true;
}

bool GltfMaterialHandler::load(const FilePath& filePath, StringVec& /*logger*/)
{
    const std::string input_filename = filePath.asString(FilePath::FormatPosix);
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

    loadMaterials(data);

    //cgltf_free(data);

    return true;
}

// Utilities
NodePtr GltfMaterialHandler::createColoredTexture(DocumentPtr& doc, const std::string & nodeName, const std::string& fileName,
                                                  const Color4& color, const std::string & colorspace)
{
    std::string newTextureName = doc->createValidChildName(nodeName);
    NodePtr newTexture = doc->addNode("gltf_colorimage", newTextureName, MULTI_OUTPUT_TYPE_STRING);
    if (!newTexture)
    {
        return nullptr;
    }
    if (_generateNodeDefs)
    {
        newTexture->setAttribute(InterfaceElement::NODE_DEF_ATTRIBUTE, "ND_gltf_colorimage");
    }
    if (_generateFullDefinitions)
    {
        newTexture->addInputsFromNodeDef();
    }
    InputPtr fileInput = newTexture->addInputFromNodeDef(Implementation::FILE_ATTRIBUTE);
    fileInput->setValue(fileName, FILENAME_TYPE_STRING);

    InputPtr colorInput = newTexture->addInputFromNodeDef(COLOR_SEMANTIC);
    ValuePtr colorValue = Value::createValue<Color4>(color);
    const string& cvs = colorValue->getValueString();
    colorInput->setValueString(cvs);

    if (!colorspace.empty())
    {
        fileInput->setAttribute(Element::COLOR_SPACE_ATTRIBUTE, colorspace);
    }
    return newTexture;
}


NodePtr GltfMaterialHandler::createTexture(DocumentPtr& doc, const std::string & nodeName, const std::string& fileName,
                                           const std::string& textureType, const std::string & colorspace, 
                                           const std::string& nodeType)
{
    std::string newTextureName = doc->createValidChildName(nodeName);
    NodePtr newTexture = doc->addNode(nodeType, newTextureName, textureType);
    if (!newTexture)
    {
        return nullptr;
    }
    if (_generateNodeDefs)
    {
        newTexture->setAttribute(InterfaceElement::NODE_DEF_ATTRIBUTE, "ND_image_" + textureType);
    }
    if (_generateFullDefinitions)
    {
        newTexture->addInputsFromNodeDef();
    }
    InputPtr fileInput = newTexture->addInputFromNodeDef(Implementation::FILE_ATTRIBUTE);
    if (fileInput)
    {
        fileInput->setValue(fileName, FILENAME_TYPE_STRING);
        if (!colorspace.empty())
        {
            fileInput->setAttribute(Element::COLOR_SPACE_ATTRIBUTE, colorspace);
        }
    }
    //else
    //{
    //    std::cout << "Invalid texture type node created: " +  textureType ;
    //}
    return newTexture;
}

void addTexCoordNode(NodePtr image, int uvindex)
{
    ElementPtr parent = image->getParent();
    GraphElementPtr parentGraph = parent ? parent->asA<GraphElement>() : nullptr;
    if (parentGraph)
    {
        const string texcoordName = parent->createValidChildName("texcoord");
        NodePtr texcoordNode = parentGraph->addNode("texcoord", texcoordName, "vector2");
        if (texcoordNode)
        {
            InputPtr uvIndexInput = texcoordNode->addInputFromNodeDef("index");
            if (uvIndexInput)
            {
                uvIndexInput->setValue<int>(uvindex);
            }

            // Connect to image node
            InputPtr texcoordInput = image->addInputFromNodeDef("texcoord");
            if (texcoordInput)
            {
                texcoordInput->setAttribute("nodename", texcoordNode->getName());
            }
        }
    }
}

void setImageProperties(NodePtr image, const cgltf_texture_view* textureView)
{
    cgltf_texture* texture = textureView ? textureView->texture : nullptr;
    if (!texture)
    {
        return;
    }

    // Handle uvset index
    InputPtr uvIndexInput = nullptr;
    if (textureView->texcoord != 0)
    {
        // Add upstream texcoord node if needed
        addTexCoordNode(image, textureView->texcoord);
    }

    // Handle transform
    if (textureView->has_transform)
    {
        const cgltf_texture_transform& transform = textureView->transform;
        InputPtr offsetInput = image->addInputFromNodeDef("offset");
        if (offsetInput)
        {
            // Note: Pivot is 0,1 in glTF and 0,0 in MaterialX
            // This is handled in the MaterialX implemenation
            // where the pivot is 0,1 and X offset are negative from there.
            offsetInput->setValueString(std::to_string(transform.offset[0]) + "," +
                std::to_string(transform.offset[1]));
        }
        InputPtr rotationInput = image->addInputFromNodeDef("rotate");
        if (rotationInput)
        {
            // Note: Rotation in glTF and MaterialX are opposite directions
            // This is handled in the MaterialX implementation
            rotationInput->setValue<float>(TO_DEGREE * transform.rotation);
        }
        InputPtr scaleInput = image->addInputFromNodeDef("scale");
        if (scaleInput)
        {
            // Scale is inverted between glTF and MaterialX.
            // This is handled in the MaterialX implementation
            scaleInput->setValueString(std::to_string(transform.scale[0]) + "," +
                std::to_string(transform.scale[1]));
        }

        // Handle uvset index
        if (transform.has_texcoord)
        {
            if (transform.texcoord != 0)
            {
                // Add upstream texcoord node if needed
                addTexCoordNode(image, textureView->texcoord);
            }
        }

        // Handle sampler. Magic numbers based upon:
        // https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/schema/sampler.schema.json
        cgltf_sampler* sampler = texture->sampler;
        if (sampler)
        {
            std::unordered_map<int, string> wrapMap;
            wrapMap[33071] = "clamp";
            wrapMap[33648] = "mirror";
            wrapMap[10497] = "periodic";

            if (wrapMap.count(sampler->wrap_s))
            {
                string uaddress = wrapMap[sampler->wrap_s];
                InputPtr addressInput = image->addInputFromNodeDef("uaddressmode");
                if (addressInput)
                {
                    addressInput->setValueString(uaddress);
                }
            }
            if (wrapMap.count(sampler->wrap_t))
            {
                string vaddress = wrapMap[sampler->wrap_s];
                InputPtr addressInput = image->addInputFromNodeDef("vaddressmode");
                if (addressInput)
                {
                    addressInput->setValueString(vaddress);
                }
            }

            // Filter. There is only one filter type so set based on the
            // mag filter.
            std::unordered_map<int, string> filterMap;
            filterMap[9728] = "closest";
            filterMap[9729] = "linear";
            filterMap[9984] = "cubic";
            filterMap[9985] = "closest";
            filterMap[9986] = "linear";
            filterMap[9987] = "cubic";

            if (filterMap.count(sampler->mag_filter))
            {
                string filterString = filterMap[sampler->mag_filter];
                InputPtr filterInput = image->addInputFromNodeDef("filtertype");
                if (filterInput)
                {
                    filterInput->setValueString(filterString);
                }
            }
        }
    }
}

void GltfMaterialHandler::setNormalMapInput(DocumentPtr materials, NodePtr shaderNode, const std::string& inputName,
    const void* textureViewIn, const std::string& inputImageNodeName)
{
    const cgltf_texture_view* textureView = (const cgltf_texture_view*)(textureViewIn);

    cgltf_texture* texture = textureView ? textureView->texture : nullptr;
    if (texture && texture->image)
    {
        std::string imageNodeName = texture->image->name ? texture->image->name :
            inputImageNodeName;
        imageNodeName = _materials->createValidChildName(imageNodeName);
        std::string uri = texture->image->uri ? texture->image->uri : SPACE_STRING;
        // Note: we create a gltf_normalmap here
        NodePtr newTexture = createTexture(_materials, imageNodeName, uri,
                                            VEC3_STRING, EMPTY_STRING, "gltf_normalmap");
        if (newTexture)
        {
            setImageProperties(newTexture, textureView);

            InputPtr normalInput = shaderNode->addInputFromNodeDef(inputName);
            if (normalInput)
            {
                normalInput->setAttribute(PortElement::NODE_NAME_ATTRIBUTE, newTexture->getName());
                normalInput->removeAttribute(AttributeDef::VALUE_ATTRIBUTE);
            }
        }
    }
}

void GltfMaterialHandler::setColorInput(DocumentPtr materials, NodePtr shaderNode, const std::string& colorInputName, 
                                       const Color3& color, float alpha, const std::string& alphaInputName, 
                                       const void* textureViewIn,
                                       const std::string& inputImageNodeName)
{
    const cgltf_texture_view* textureView = static_cast<const cgltf_texture_view*>(textureViewIn);

    InputPtr colorInput = nullptr; 
    InputPtr alphaInput = nullptr; 

    // Handle textured color / alpha input
    //
    cgltf_texture* texture = textureView ? textureView->texture : nullptr;
    if (texture && texture->image)
    {
        // Simple color3 image lookup
        if (!alphaInput)
        {
            std::string imageNodeName = texture->image->name ? texture->image->name : inputImageNodeName;
            imageNodeName = materials->createValidChildName(imageNodeName);
            std::string uri = texture->image->uri ? texture->image->uri : SPACE_STRING;
            NodePtr newTexture = createTexture(materials, imageNodeName, uri, "color3", "srgb_texture");
            if (newTexture)
            {
                setImageProperties(newTexture, textureView);
            }
            if (!colorInput)
            {
                colorInput = colorInputName.size() ? shaderNode->addInputFromNodeDef(colorInputName) : nullptr;
                colorInput->setAttribute(PortElement::NODE_NAME_ATTRIBUTE, newTexture->getName());
                colorInput->removeAttribute(AttributeDef::VALUE_ATTRIBUTE);
            }
        }

        // Color, alpha lookup
        else
        {
            std::string imageNodeName = texture->image->name ? texture->image->name : inputImageNodeName;
            imageNodeName = materials->createValidChildName(imageNodeName);
            std::string uri = texture->image->uri ? texture->image->uri : SPACE_STRING;

            const Color4 color4(color[0], color[1], color[2], alpha);
            NodePtr newTexture = createColoredTexture(materials, imageNodeName, uri, color4, "srgb_texture");
            if (newTexture)
            {
                setImageProperties(newTexture, textureView);

                const string& newTextureName = newTexture->getName();
                if (!colorInput)
                {
                    colorInput = colorInputName.size() ? shaderNode->addInputFromNodeDef(colorInputName) : nullptr;
                    colorInput->setAttribute(PortElement::NODE_NAME_ATTRIBUTE, newTextureName);
                    colorInput->setOutputString("outcolor");
                    colorInput->removeAttribute(AttributeDef::VALUE_ATTRIBUTE);
                }

                if (!alphaInput)
                {
                    alphaInput = alphaInputName.size() ? shaderNode->addInputFromNodeDef(alphaInputName) : nullptr;
                    alphaInput->setAttribute(PortElement::NODE_NAME_ATTRIBUTE, newTextureName);
                    alphaInput->setOutputString("outa");
                    alphaInput->removeAttribute(AttributeDef::VALUE_ATTRIBUTE);
                }
            }
        }
    }

    // Handle simple color / alpha input
    else
    {
        if (!colorInput)
        {
            colorInput = colorInputName.size() ? shaderNode->addInputFromNodeDef(colorInputName) : nullptr;
        }
        if (colorInput)
        {
            ValuePtr color3Value = Value::createValue<Color3>(color);
            colorInput->setValueString(color3Value->getValueString());
        }

        if (!alphaInput)
        {
            alphaInput = alphaInputName.size() ? shaderNode->addInputFromNodeDef(alphaInputName) : nullptr;
        }
        if (alphaInput)
        {
            alphaInput->setValue<float>(alpha);
        }
    }
}

void GltfMaterialHandler::setFloatInput(DocumentPtr materials, NodePtr shaderNode, const std::string& inputName, 
                                       float floatFactor, const void* textureViewIn,
                                       const std::string& inputImageNodeName)
{
    const cgltf_texture_view* textureView = static_cast<const cgltf_texture_view*>(textureViewIn);

    InputPtr floatInput = shaderNode->addInputFromNodeDef(inputName);
    if (floatInput)
    {
        cgltf_texture* texture = textureView ? textureView->texture : nullptr;
        if (texture && texture->image)
        {
            std::string imageNodeName = materials->createValidChildName(inputImageNodeName);
            std::string uri = texture->image->uri ? texture->image->uri : SPACE_STRING;
            NodePtr newTexture = createTexture(materials, imageNodeName, uri,
                                               FLOAT_STRING, EMPTY_STRING);
            if (newTexture)
            {
                setImageProperties(newTexture, textureView);
            }
            floatInput->setAttribute(PortElement::NODE_NAME_ATTRIBUTE, newTexture->getName());
            floatInput->removeAttribute(AttributeDef::VALUE_ATTRIBUTE);               

            floatInput = shaderNode->addInputFromNodeDef("factor");
            if (floatInput)
            {
                floatInput->setValue<float>(floatFactor);
            }
        }
        else
        {
            floatInput->setValue<float>(floatFactor);
        }
    }
}

void GltfMaterialHandler::setVector3Input(DocumentPtr materials, NodePtr shaderNode, const std::string& inputName, 
                                       const Vector3& vecFactor, const void* textureViewIn,
                                       const std::string& inputImageNodeName)
{
    const cgltf_texture_view* textureView = static_cast<const cgltf_texture_view*>(textureViewIn);

    InputPtr vecInput = shaderNode->addInputFromNodeDef(inputName);
    if (vecInput)
    {
        ValuePtr factor = Value::createValue<Vector3>(vecFactor);
        const string factorString = factor->getValueString();

        cgltf_texture* texture = textureView ? textureView->texture : nullptr;
        if (texture && texture->image)
        {
            std::string imageNodeName = materials->createValidChildName(inputImageNodeName);
            std::string uri = texture->image->uri ? texture->image->uri : SPACE_STRING;
            NodePtr newTexture = createTexture(materials, imageNodeName, uri,
                                               VEC3_STRING, EMPTY_STRING);
            if (newTexture)
            {
                setImageProperties(newTexture, textureView);
            }
            vecInput->setAttribute(PortElement::NODE_NAME_ATTRIBUTE, newTexture->getName());
            vecInput->removeAttribute(AttributeDef::VALUE_ATTRIBUTE);               

            vecInput = shaderNode->addInputFromNodeDef("factor");
            if (vecInput)
            {
                vecInput->setValueString(factorString);
            }
        }
        else
        {
            vecInput->setValueString(factorString);
        }
    }
}

void GltfMaterialHandler::loadMaterials(void *vdata)
{
    cgltf_data* data = static_cast<cgltf_data*>(vdata);

    // Scan materials
    /*
    * typedef struct cgltf_material
    {
	    char* name;
	    cgltf_bool has_pbr_metallic_roughness;
	    cgltf_bool has_pbr_specular_glossiness;
	    cgltf_bool has_clearcoat;
	    cgltf_bool has_transmission;
	    cgltf_bool has_volume;
	    cgltf_bool has_ior;
	    cgltf_bool has_specular;
	    cgltf_bool has_sheen;
	    cgltf_bool has_emissive_strength;
	    cgltf_pbr_metallic_roughness pbr_metallic_roughness;
	    cgltf_pbr_specular_glossiness pbr_specular_glossiness;
	    cgltf_clearcoat clearcoat;
	    cgltf_ior ior;
	    cgltf_specular specular;
	    cgltf_sheen sheen;
	    cgltf_transmission transmission;
	    cgltf_volume volume;
	    cgltf_emissive_strength emissive_strength;
	    cgltf_texture_view normal_texture;
	    cgltf_texture_view occlusion_texture;
	    cgltf_texture_view emissive_texture;
	    cgltf_float emissive_factor[3];
	    cgltf_alpha_mode alpha_mode;
	    cgltf_float alpha_cutoff;
	    cgltf_bool double_sided;
	    cgltf_bool unlit;
	    cgltf_extras extras;
	    cgltf_size extensions_count;
	    cgltf_extension* extensions;
    } cgltf_material;
    */
    if (!data->materials_count)
    {
        _materials = nullptr;
        return;
    }
    _materials = Document::createDocument<Document>();
    _materials->importLibrary(_definitions);

    for (size_t m = 0; m < data->materials_count; m++)
    {
        cgltf_material* material = &(data->materials[m]);
        if (!material)
        {
            continue;
        }

        // Create a default gltf_pbr node
        std::string shaderName;
        std::string materialName;
        if (!material->name)
        {
            materialName = DEFAULT_MATERIAL_NAME;
            shaderName = DEFAULT_SHADER_NAME;
        }
        else
        {
            StringMap invalidTokens;
            invalidTokens["__"] = "_";
            string origName = material->name;
            origName = replaceSubstrings(origName, invalidTokens);

            materialName = "MAT_" + origName;
            shaderName = "SHD_" + origName;
        }
        materialName = _materials->createValidChildName(materialName);
        string* name = new string(materialName);
        material->name = const_cast<char*>(name->c_str());

        shaderName = _materials->createValidChildName(shaderName);

        // Check for unlit
        bool use_unlit = material->unlit;
        string shaderCategory = use_unlit ? "surface_unlit" : "gltf_pbr";
        string nodedefString = use_unlit ? "ND_surface_unlit" : "ND_gltf_pbr_surfaceshader";
        NodePtr shaderNode = _materials->addNode(shaderCategory, shaderName, SURFACE_SHADER_TYPE_STRING);
        if (_generateNodeDefs)
        {
            shaderNode->setAttribute(InterfaceElement::NODE_DEF_ATTRIBUTE, nodedefString);
        }
        if (_generateFullDefinitions)
        {
            shaderNode->addInputsFromNodeDef();
        }

        // Create a surface material for the shader node
        NodePtr materialNode = _materials->addNode(SURFACE_MATERIAL_NODE_STRING, materialName, MATERIAL_TYPE_STRING);
        InputPtr shaderInput = materialNode->addInput(SURFACE_SHADER_TYPE_STRING, SURFACE_SHADER_TYPE_STRING);
        shaderInput->setAttribute(PortElement::NODE_NAME_ATTRIBUTE, shaderNode->getName());

        // Handle separate occlusion texture
        bool haveSeparateOcclusion = false;
        cgltf_texture* occlusion_texture = material->occlusion_texture.texture;
        if (!use_unlit && occlusion_texture)
        {
            std::string oURI;
            std::string mrURI;
            if (occlusion_texture->image)
            {
                oURI = occlusion_texture->image->uri ? occlusion_texture->image->uri : SPACE_STRING;
            }

            if (!oURI.empty() && material->has_pbr_metallic_roughness)
            {
                cgltf_pbr_metallic_roughness& roughness = material->pbr_metallic_roughness;
                cgltf_texture_view& textureView = roughness.metallic_roughness_texture;
                cgltf_texture* texture = textureView.texture;
                if (texture && texture->image)
                {
                    mrURI = texture->image->uri ? texture->image->uri : SPACE_STRING;
                }

                if (mrURI != oURI)
                {
                    haveSeparateOcclusion = true;
                    InputPtr occlusionInput = shaderNode->addInputFromNodeDef("occlusion");
                    setFloatInput(_materials, shaderNode, "occlusion", 1.0, &material->occlusion_texture, "image_occlusion");
                }
            }
        }

        if (material->has_pbr_metallic_roughness)
        {
            StringVec colorAlphaInputs = { "base_color", "alpha", "emission_color", "opacity" };
            size_t  colorAlphaInputOffset = use_unlit ? 2 : 0;

            cgltf_pbr_metallic_roughness& roughness = material->pbr_metallic_roughness;

            // Parse base color and alpha
            Color3 colorFactor(roughness.base_color_factor[0],
                roughness.base_color_factor[1],
                roughness.base_color_factor[2]);
            float alpha = roughness.base_color_factor[3];
            setColorInput(_materials, shaderNode, colorAlphaInputs[colorAlphaInputOffset],
                colorFactor, alpha, colorAlphaInputs[colorAlphaInputOffset + 1],
                &roughness.base_color_texture, "image_basecolor");

            // Ignore any other information unsupported by unlit.
            if (use_unlit)
            {
                continue;
            }

            // Parse metalic, roughness, and occlusion (if not specified separately)
            InputPtr metallicInput = shaderNode->addInputFromNodeDef("metallic");
            InputPtr roughnessInput = shaderNode->addInputFromNodeDef("roughness");
            InputPtr occlusionInput = haveSeparateOcclusion ? nullptr : shaderNode->addInputFromNodeDef("occlusion");

            // Check for occlusion/metallic/roughness texture
            cgltf_texture_view& textureView = roughness.metallic_roughness_texture;
            cgltf_texture* texture = textureView.texture;
            if (texture && texture->image)
            {
                std::string imageNodeName = texture->image->name ? texture->image->name :
                    "image_orm";
                imageNodeName = _materials->createValidChildName(imageNodeName);
                std::string uri = texture->image->uri ? texture->image->uri : SPACE_STRING;
                NodePtr textureNode = createTexture(_materials, imageNodeName, uri,
                    VEC3_STRING, EMPTY_STRING);
                if (textureNode)
                {
                    setImageProperties(textureNode, &textureView);
                }

                // Map image channesl to inputs
                StringVec indexName = { "x", "y", "z" };
                std::vector<InputPtr> inputs = { occlusionInput, roughnessInput, metallicInput };
                for (size_t i = 0; i < inputs.size(); i++)
                {
                    if (inputs[i])
                    {
                        inputs[i]->setAttribute(PortElement::NODE_NAME_ATTRIBUTE, textureNode->getName());
                        inputs[i]->setType(FLOAT_STRING);
                        inputs[i]->setChannels(indexName[i]);
                    }
                }

                // See note about both AttributeDef::VALUE_ATTRIBUTE and PortElement::NODE_NAME_ATTRIBUTE being
                // specified.
                metallicInput->setValue<float>(roughness.metallic_factor);
                roughnessInput->setValue<float>(roughness.roughness_factor);
            }
            else
            {
                metallicInput->setValue<float>(roughness.metallic_factor);;
                roughnessInput->setValue<float>(roughness.roughness_factor);
            }
        }

        // Firewall : Skip any other mappings as they will not exist on unlit_surface
        if (use_unlit)
        {
            continue;
        }

        // Parse unmapped alpha parameters
        //
        if (material->alpha_mode != 0)
        {
            InputPtr alphaMode = shaderNode->addInputFromNodeDef("alpha_mode");
            if (alphaMode)
            {
                cgltf_alpha_mode alpha_mode = material->alpha_mode;
                alphaMode->setValue<int>(static_cast<int>(alpha_mode));
            }
        }
        if (material->alpha_cutoff != 0.5f)
        {
            InputPtr alphaCutoff = shaderNode->addInputFromNodeDef("alpha_cutoff");
            if (alphaCutoff)
            {
                alphaCutoff->setValue<float>(material->alpha_cutoff);
            }
        }

        // Normal texture
        setNormalMapInput(_materials, shaderNode, "normal", &(material->normal_texture), "image_normal");

        // Handle sheen
        if (material->has_sheen)
        {
            cgltf_sheen& sheen = material->sheen;
            
            Color3 colorFactor(sheen.sheen_color_factor[0],
                               sheen.sheen_color_factor[1],
                               sheen.sheen_color_factor[2]);
            setColorInput(_materials, shaderNode, "sheen_color",
                colorFactor, 1.0f, EMPTY_STRING, &sheen.sheen_color_texture, "image_sheen");

            setFloatInput(_materials, shaderNode, "sheen_roughness",
                sheen.sheen_roughness_factor, &sheen.sheen_roughness_texture,
                "image_sheen_roughness");
        }

        // Only 1.35.6 and newer has iridescence
        //std::tuple<int, int, int> version = getVersionIntegers();
        //if (version >= std::tuple<int,int,int>(1, 35, 6))
        {
            // Parse iridescence
            // typedef struct cgltf_iridescence
            //{
            //	cgltf_float iridescence_factor;
            //	cgltf_texture_view iridescence_texture;
            //	cgltf_float iridescence_ior;
            //	cgltf_float iridescence_thickness_min;
            //	cgltf_float iridescence_thickness_max;
            //	cgltf_texture_view iridescence_thickness_texture;
            //} cgltf_iridescence;
            // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_iridescence/README.md
            if (material->has_iridescence)
            {
                const cgltf_iridescence& iridescence = material->iridescence;

                setFloatInput(_materials, shaderNode, "iridescence",
                    iridescence.iridescence_factor, &iridescence.iridescence_texture,
                    "image_iridescence");

                setFloatInput(_materials, shaderNode, "iridescence_ior",
                    iridescence.iridescence_ior, nullptr,
                    "image_iridescence_ior");

                // Create special node to map thickness min, max and input texture
                InputPtr floatInput = shaderNode->addInputFromNodeDef("iridescence_thickness");
                if (floatInput)
                {
                    const cgltf_texture_view& textureView = iridescence.iridescence_thickness_texture;
                    cgltf_texture* texture = textureView.texture;
                    if (texture && texture->image)
                    {
                        std::string imageNodeName = _materials->createValidChildName("image_iridescence_thickness");
                        std::string uri = texture->image->uri ? texture->image->uri : SPACE_STRING;
                        NodePtr newTexture = createTexture(_materials, imageNodeName, uri, FLOAT_STRING, EMPTY_STRING,
                            "gltf_iridescence_thickness");
                        if (newTexture)
                        {
                            InputPtr minInput = newTexture->addInputFromNodeDef("thicknessMin");
                            if (minInput)
                            {
                                minInput->setValue<float>(iridescence.iridescence_thickness_min);
                            }
                            InputPtr maxInput = newTexture->addInputFromNodeDef("thicknessMax");
                            if (maxInput)
                            {
                                maxInput->setValue<float>(iridescence.iridescence_thickness_max);
                            }
                            setImageProperties(newTexture, &textureView);
                        }
                        floatInput->setAttribute(PortElement::NODE_NAME_ATTRIBUTE, newTexture->getName());
                        floatInput->removeAttribute(AttributeDef::VALUE_ATTRIBUTE);
                    }
                }
            }
        }

        // Parse clearcoat
        // typedef struct cgltf_clearcoat
        // {
        //	    cgltf_texture_view clearcoat_texture;
        //	    cgltf_texture_view clearcoat_roughness_texture;
        //	    cgltf_texture_view clearcoat_normal_texture;
        //
        //	    cgltf_float clearcoat_factor;
        //	    cgltf_float clearcoat_roughness_factor;
        // } cgltf_clearcoat;
        if (material->has_clearcoat)
        {
            cgltf_clearcoat& clearcoat = material->clearcoat;

            // Mapped or unmapped clearcoat
            setFloatInput(_materials, shaderNode, "clearcoat",
                clearcoat.clearcoat_factor,
                &clearcoat.clearcoat_texture,
                "image_clearcoat");

            // Mapped or unmapped clearcoat roughness
            setFloatInput(_materials, shaderNode, "clearcoat_roughness",
                clearcoat.clearcoat_roughness_factor,
                &clearcoat.clearcoat_roughness_texture,
                "image_clearcoat_roughness");

            // Normal map clearcoat_normal
            setNormalMapInput(_materials, shaderNode, "clearcoat_normal", &material->normal_texture, 
                            "image_clearcoat_normal");
        }

        // Parse transmission
        // typedef struct cgltf_transmission
        // {
        //	    cgltf_texture_view transmission_texture;
        //	    cgltf_float transmission_factor;
        // } cgltf_transmission;
        if (material->has_transmission)
        {
            cgltf_transmission& transmission = material->transmission;

            setFloatInput(_materials, shaderNode, "transmission",
                transmission.transmission_factor,
                &transmission.transmission_texture,
                "image_transmission");
        }

        // Parse specular and specular color
        // typedef struct cgltf_specular {
        //      cgltf_texture_view specular_texture;
        //      cgltf_texture_view specular_color_texture;
        //      cgltf_float specular_color_factor[3];
        //      cgltf_float specular_factor;
        // } cgltf_specular
        //
        if (material->has_specular)
        {
            cgltf_specular& specular = material->specular;

            // Mapped or unmapped specular
            Color3 colorFactor(specular.specular_color_factor[0],
                specular.specular_color_factor[1],
                specular.specular_color_factor[2]);
            setColorInput(_materials, shaderNode, "specular_color",
                colorFactor, 1.0f, EMPTY_STRING,
                &specular.specular_color_texture,
                "image_specularcolor");

            // Mapped or unmapped specular color
            setFloatInput(_materials, shaderNode, "specular",
                specular.specular_factor,
                &specular.specular_texture,
                "image_specular");
        }

        // Parse untextured ior 
        if (material->has_ior)
        {
            cgltf_ior& ior = material->ior;
            InputPtr iorInput = shaderNode->addInputFromNodeDef("ior");
            if (iorInput)
            {
                iorInput->setValue<float>(ior.ior);
            }
        }

        // Parse emissive inputs
        //
        // cgltf_texture_view emissive_texture;
        // cgltf_float emissive_factor[3];
        //
        Color3 colorFactor(material->emissive_factor[0],
            material->emissive_factor[1],
            material->emissive_factor[2]);
        setColorInput(_materials, shaderNode, "emissive",
            colorFactor, 1.0f, EMPTY_STRING, &material->emissive_texture, "image_emission");

        if (material->has_emissive_strength)
        {
            cgltf_emissive_strength& emissive_strength = material->emissive_strength;
            InputPtr input = shaderNode->addInputFromNodeDef("emissive_strength");
            if (input)
            {
                input->setValue<float>(emissive_strength.emissive_strength);
            }
        }

        // Parse Volume Inputs:
        // 
        // typedef struct cgltf_volume
        // {
        //      cgltf_texture_view thickness_texture;
        //      cgltf_float thickness_factor;
        //      cgltf_float attenuation_color[3];
        //      cgltf_float attenuation_distance;
        // } cgltf_volume;
        //
        if (material->has_volume)
        {
            cgltf_volume& volume = material->volume;

            // Textured or untexture thickness
            setFloatInput(_materials, shaderNode, "thickness",
                volume.thickness_factor,
                &volume.thickness_texture,
                "thickness");

            // Untextured attenuation color
            Color3 attenFactor(volume.attenuation_color[0],
                volume.attenuation_color[1],
                volume.attenuation_color[2]);
            setColorInput(_materials, shaderNode, "attenuation_color",
                          attenFactor, 1.0f, EMPTY_STRING, nullptr, EMPTY_STRING);

            // Untextured attenuation distance
            setFloatInput(_materials, shaderNode, "attenuation_distance",
                volume.attenuation_distance, nullptr, EMPTY_STRING);
        }
    }

    // Create material associations. Needed to check if a mesh has CPV which 
    // the assumption is that the corresponding material is using always it.
    GLTFMaterialMeshList materialMeshList;
    StringSet materialCPVList;
    FilePath meshPath;
    unsigned int nodeCount = 0;
    unsigned int meshCount = 0;
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
            computeMeshMaterials(materialMeshList, materialCPVList, cnode, meshPath, nodeCount, meshCount);
        }
    }

    for (const string& materialName : materialCPVList)
    {
        NodePtr materialNode = _materials->getNode(materialName);
        InputPtr shaderInput = materialNode ? materialNode->getInput("surfaceshader") : nullptr;
        NodePtr shaderNode = shaderInput ? shaderInput->getConnectedNode() : nullptr;
        InputPtr baseColorInput = shaderNode ? shaderNode->getInput("base_color") : nullptr;
        NodePtr baseColorNode = baseColorInput ? baseColorInput->getConnectedNode() : nullptr;
        if (baseColorNode)
        {
            InputPtr geomcolorInput = baseColorNode->addInputFromNodeDef("geomcolor");
            if (geomcolorInput)
            {
                NodePtr geomcolor = _materials->addNode("geomcolor", EMPTY_STRING, "color4");
                geomcolorInput->setNodeName(geomcolor->getName());
            }
        }
    }

    // Create a material assignment for this material if requested
    if (_generateAssignments)
    {
        // Add look with material assignments if requested
        LookPtr look = _materials->addLook();
        for (auto materialItem : materialMeshList)
        {
            const string& materialName = materialItem.first;
            const string& paths = materialItem.second;

            // Keep the assignment as simple as possible as more complex
            // systems such as USD can parse these files easily.
            MaterialAssignPtr matAssign = look->addMaterialAssign(EMPTY_STRING, materialName);
            matAssign->setGeom(paths);
        }
    }
}

MATERIALX_NAMESPACE_END
