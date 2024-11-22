/******************************************************************************
 * Copyright (c) 2021-2024, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

 // examples/mdl_sdk/df_vulkan/example_df_vulkan.cpp
 //
 // Simple Vulkan renderer using compiled BSDFs with a material parameter editor GUI.

#include "mdl_generator.h"
#include "utils/example_shared.h"
#include "utils/vulkan_base_application.h"

#include <chrono>
#include <numeric>
#define _USE_MATH_DEFINES
#include <math.h>

static const VkFormat g_accumulation_texture_format = VK_FORMAT_R32G32B32A32_SFLOAT;

// Local group size for the path tracing compute shader
static const uint32_t g_local_size_x = 16;
static const uint32_t g_local_size_y = 8;

// Descriptor set bindings. Used as a define in the shader.
static const uint32_t g_binding_beauty_buffer = 0;
static const uint32_t g_binding_render_params = 5;
static const uint32_t g_binding_environment_map = 6;
static const uint32_t g_binding_environment_sampling_data = 7;
static const uint32_t g_binding_material_textures_2d = 8;
static const uint32_t g_binding_material_textures_3d = 9;
static const uint32_t g_binding_ro_data_buffer = 10;

static const uint32_t g_set_ro_data_buffer = 0;
static const uint32_t g_set_material_textures = 0;

// Command line options structure.
struct Options
{
    bool no_window = false;
    std::string output_file = "output.exr";
    uint32_t res_x = 1024;
    uint32_t res_y = 768;
    uint32_t num_images = 3;
    int32_t device_index = -1;
    uint32_t samples_per_pixel = 4096;
    uint32_t samples_per_iteration = 8;
    uint32_t max_path_length = 4;
    float cam_fov = 45.0f;
    mi::Float32_3 cam_pos = { 0.0f, 0.0f, 3.0f };
    mi::Float32_3 cam_lookat = { 0.0f, 0.0f, 0.0f };
    mi::Float32_3 light_pos = { 10.0f, 0.0f, 5.0f };
    mi::Float32_3 light_intensity = { 1.0f, 0.95f, 0.9f };
    bool light_enabled = false;
    std::string hdr_file = "goegap.hdr";
    float hdr_intensity = 1.0f;
    float hdr_rotate = 0.0f;
    bool background_color_enabled = false;
    mi::Float32_3 background_color = { 0.0f, 0.0f, 0.0f };
    bool enable_ro_segment = false;
    bool disable_ssbo = false;
    uint32_t max_const_data = 1024;
    std::string material_name = "";
    bool enable_validation_layers = false;
    bool enable_shader_optimization = true;
    std::string dump_mdl = "";
    std::string dump_glsl = "";
    std::vector<std::string> mdl_search_paths = {};
    std::vector<std::string> mtlx_paths = {};
    std::vector<std::string> mtlx_libraries = {};
    bool materialxtest_mode = false;
    MaterialX::GenMdlOptions::MdlVersion mdl_target_version = MaterialX::GenMdlOptions::MdlVersion::MDL_LATEST;
    mi::examples::log::Level log_level = mi::examples::log::Level::Verbose;
};

using Vulkan_texture = mi::examples::vk::Vulkan_texture;
using Vulkan_buffer = mi::examples::vk::Vulkan_buffer;


//------------------------------------------------------------------------------
// MDL-Vulkan resource interop
//------------------------------------------------------------------------------

// Creates the image and image view for the given texture index.
Vulkan_texture create_material_texture(
    VkDevice device,
    VkPhysicalDevice physical_device,
    VkQueue queue,
    VkCommandPool command_pool,
    mi::neuraylib::ITransaction* transaction,
    mi::neuraylib::IImage_api* image_api,
    const mi::neuraylib::ITarget_code* target_code,
    mi::Size texture_index)
{
    // Get access to the texture data by the texture database name from the target code.
    mi::base::Handle<const mi::neuraylib::ITexture> texture(
        transaction->access<mi::neuraylib::ITexture>(target_code->get_texture(texture_index)));
    mi::base::Handle<const mi::neuraylib::IImage> image(
        transaction->access<mi::neuraylib::IImage>(texture->get_image())); 
    mi::base::Handle<const mi::neuraylib::ICanvas> canvas(image->get_canvas(0, 0, 0));
    mi::Uint32 tex_width = canvas->get_resolution_x();
    mi::Uint32 tex_height = canvas->get_resolution_y();
    mi::Uint32 tex_layers = canvas->get_layers_size();
    char const* image_type = image->get_type(0, 0);

    if (image->is_uvtile() || image->is_animated())
    {
        std::cerr << "The example does not support uvtile and/or animated textures!" << std::endl;
        terminate();
    }

    // For simplicity, the texture access functions are only implemented for float4 and gamma
    // is pre-applied here (all images are converted to linear space).

    // Convert to linear color space if necessary
    if (texture->get_effective_gamma(0, 0) != 1.0f)
    {
        // Copy/convert to float4 canvas and adjust gamma from "effective gamma" to 1.
        mi::base::Handle<mi::neuraylib::ICanvas> gamma_canvas(
            image_api->convert(canvas.get(), "Color"));
        gamma_canvas->set_gamma(texture->get_effective_gamma(0, 0));
        image_api->adjust_gamma(gamma_canvas.get(), 1.0f);
        canvas = gamma_canvas;
    }
    else if (strcmp(image_type, "Color") != 0 && strcmp(image_type, "Float32<4>") != 0)
    {
        // Convert to expected format
        canvas = image_api->convert(canvas.get(), "Color");
    }

    // Create the Vulkan image
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // This example supports only 2D and 3D textures (no PTEX or cube)
    mi::neuraylib::ITarget_code::Texture_shape texture_shape
        = target_code->get_texture_shape(texture_index);

    if (texture_shape == mi::neuraylib::ITarget_code::Texture_shape_2d)
    {
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = tex_width;
        image_create_info.extent.height = tex_height;
        image_create_info.extent.depth = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.mipLevels = 1;
    }
    else if (texture_shape == mi::neuraylib::ITarget_code::Texture_shape_3d
        || texture_shape == mi::neuraylib::ITarget_code::Texture_shape_bsdf_data)
    {
        image_create_info.imageType = VK_IMAGE_TYPE_3D;
        image_create_info.extent.width = tex_width;
        image_create_info.extent.height = tex_height;
        image_create_info.extent.depth = tex_layers;
        image_create_info.arrayLayers = 1;
        image_create_info.mipLevels = 1;
    }
    else
    {
        std::cerr << "Unsupported texture shape!" << std::endl;
        terminate();
    }

    Vulkan_texture material_texture;

    VK_CHECK(vkCreateImage(device, &image_create_info, nullptr,
        &material_texture.image));

    // Allocate device memory for the texture.
    material_texture.device_memory = mi::examples::vk::allocate_and_bind_image_memory(
        device, physical_device, material_texture.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    {
        size_t layer_size = tex_width * tex_height * sizeof(float) * 4; // RGBA32F
        size_t staging_buffer_size = layer_size * tex_layers;
        mi::examples::vk::Staging_buffer staging_buffer(device, physical_device,
            staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        // Memcpy the read-only data into the staging buffer
        uint8_t* mapped_data = static_cast<uint8_t*>(staging_buffer.map_memory());
        for (mi::Uint32 layer = 0; layer < tex_layers; layer++)
        {
            mi::base::Handle<const mi::neuraylib::ITile> tile(canvas->get_tile(layer));
            std::memcpy(mapped_data, tile->get_data(), layer_size);
            mapped_data += layer_size;
        }
        staging_buffer.unmap_memory();

        // Upload the read-only data from the staging buffer into the storage buffer
        mi::examples::vk::Temporary_command_buffer command_buffer(device, command_pool);
        command_buffer.begin();

        mi::examples::vk::transitionImageLayout(command_buffer.get(),
            /*image=*/           material_texture.image,
            /*src_access_mask=*/ 0,
            /*dst_access_mask=*/ VK_ACCESS_TRANSFER_WRITE_BIT,
            /*old_layout=*/      VK_IMAGE_LAYOUT_UNDEFINED,
            /*new_layout=*/      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            /*src_stage_mask=*/  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            /*dst_stage_mask=*/  VK_PIPELINE_STAGE_TRANSFER_BIT);

        VkBufferImageCopy copy_region = {};
        copy_region.bufferOffset = 0;
        copy_region.bufferRowLength = 0;
        copy_region.bufferImageHeight = 0;
        copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_region.imageSubresource.mipLevel = 0;
        copy_region.imageSubresource.baseArrayLayer = 0;
        copy_region.imageSubresource.layerCount = 1;
        copy_region.imageOffset = { 0, 0, 0 };
        copy_region.imageExtent = { tex_width, tex_height, tex_layers };

        vkCmdCopyBufferToImage(command_buffer.get(), staging_buffer.get(),
            material_texture.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

        mi::examples::vk::transitionImageLayout(command_buffer.get(),
            /*image=*/           material_texture.image,
            /*src_access_mask=*/ VK_ACCESS_TRANSFER_WRITE_BIT,
            /*dst_access_mask=*/ VK_ACCESS_SHADER_READ_BIT,
            /*old_layout=*/      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            /*new_layout=*/      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            /*src_stage_mask=*/  VK_PIPELINE_STAGE_TRANSFER_BIT,
            /*dst_stage_mask=*/  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        command_buffer.end_and_submit(queue);
    }

    // Create the image view
    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = material_texture.image;
    image_view_create_info.viewType = (image_create_info.imageType == VK_IMAGE_TYPE_2D)
        ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_3D;
    image_view_create_info.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(
        device, &image_view_create_info, nullptr, &material_texture.image_view));

    return material_texture;
}


//------------------------------------------------------------------------------
// Application and rendering logic
//------------------------------------------------------------------------------
class Df_vulkan_app : public mi::examples::vk::Vulkan_example_app
{
public:
    Df_vulkan_app(
        mi::base::Handle<mi::neuraylib::ITransaction> transaction,
        mi::base::Handle<mi::neuraylib::IMdl_impexp_api> mdl_impexp_api,
        mi::base::Handle<mi::neuraylib::IImage_api> image_api,
        mi::base::Handle<const mi::neuraylib::ITarget_code> target_code,
        mi::base::Handle<const mi::neuraylib::ICompiled_material> compiled_material,
        const Options& options)
    : Vulkan_example_app(mdl_impexp_api.get(), image_api.get())
    , m_transaction(transaction)
    , m_target_code(target_code)
    , m_compiled_material(compiled_material)
    , m_options(options)
    {
    }

    virtual void init_resources() override;
    virtual void cleanup_resources() override;

    // All frame buffer size dependent resources need to be recreated
    // when the swapchain is recreated due to not being optimal anymore
    // or because the window was resized.
    virtual void recreate_size_dependent_resources() override;

    // Updates the application logic. This is called right before the
    // next frame is rendered.
    virtual void update(float elapsed_seconds, uint32_t frame_index) override;

    // Populates the current frame's command buffer. The base application's
    // render pass has already been started at this point.
    virtual void render(VkCommandBuffer command_buffer, uint32_t frame_index, uint32_t image_index) override;

    virtual void resized_callback(uint32_t width, uint32_t height) override;

private:
    struct Camera_state
    {
        float base_distance;
        float theta;
        float phi;
        float zoom;
    };

    struct Render_params
    {
        // Camera
        alignas(16) mi::Float32_3 cam_pos;
        alignas(16) mi::Float32_3 cam_dir;
        alignas(16) mi::Float32_3 cam_right;
        alignas(16) mi::Float32_3 cam_up;
        float cam_focal;

        // Point light
        alignas(16) mi::Float32_3 point_light_pos;
        alignas(16) mi::Float32_3 point_light_color;
        float point_light_intensity;

        // Environment
        float environment_intensity_factor;
        float environment_inv_integral;
        float environment_rotation;
        uint32_t background_color_enabled;
        alignas(16) mi::Float32_3 background_color;

        // Render params
        uint32_t max_path_length;
        uint32_t samples_per_iteration;
        uint32_t progressive_iteration;
        uint32_t flip_texcoord_v;
    };

private:

    void update_camera_render_params(const Camera_state& cam_state);

    void create_accumulation_image();

    // Creates the rendering shader module. The generated GLSL target code,
    // GLSL MDL renderer runtime implementation, and renderer code are compiled
    // and linked together here.
    VkShaderModule create_path_trace_shader_module();

    // Creates the descriptors set layout which is used to create the
    // pipeline layout. Here the number of material resources is declared.
    void create_descriptor_set_layouts();

    // Create the pipeline layout and state for rendering a full screen triangle.
    void create_pipeline_layouts();
    void create_pipelines();

    void create_render_params_buffers();

    void create_environment_map();

    // Creates the descriptor pool and set that hold enough space for all
    // material resources, and are used during rendering to access the
    // the resources.
    void create_descriptor_pool_and_sets();

    void create_query_pool();

    void update_accumulation_image_descriptors();

    void write_accum_images_to_file();

private:
    mi::base::Handle<mi::neuraylib::ITransaction> m_transaction;
    mi::base::Handle<const mi::neuraylib::ITarget_code> m_target_code;
    mi::base::Handle<const mi::neuraylib::ICompiled_material> m_compiled_material;
    Options m_options;

    Vulkan_texture m_accum_image = {};
    VkSampler m_linear_sampler = nullptr;

    VkRenderPass m_path_trace_render_pass = nullptr;
    VkPipelineLayout m_path_trace_pipeline_layout = nullptr;
    VkPipelineLayout m_display_pipeline_layout = nullptr;
    VkPipeline m_path_trace_pipeline = nullptr;
    VkPipeline m_display_pipeline = nullptr;

    VkDescriptorSetLayout m_path_trace_descriptor_set_layout = nullptr;
    VkDescriptorSetLayout m_display_descriptor_set_layout = nullptr;
    VkDescriptorPool m_descriptor_pool = nullptr;
    std::vector<VkDescriptorSet> m_path_trace_descriptor_sets = {};
    VkDescriptorSet m_display_descriptor_set = nullptr;
    std::vector<Vulkan_buffer> m_render_params_buffers = {};
    VkQueryPool m_query_pool = nullptr;

    // Environment Map
    mi::examples::vk::Vulkan_environment_map m_environment_map;

    // Material resources
    Vulkan_buffer m_ro_data_buffer;
    std::vector<Vulkan_texture> m_material_textures_2d;
    std::vector<Vulkan_texture> m_material_textures_3d;

    Render_params m_render_params;
    bool m_camera_moved = true; // Force a clear in first frame
    uint32_t m_display_buffer_index = 0; // Which buffer to display
    bool m_first_stats_update = true;
    double m_last_stats_update = 0.0;
    float m_render_time = 0.0f;
    bool m_vsync_enabled = true;
    
    // Camera movement
    Camera_state m_camera_state = {};
    mi::Float32_2 m_mouse_start = {};
    bool m_camera_moving = false;
};

void Df_vulkan_app::init_resources()
{
    glslang::InitializeProcess();

    m_linear_sampler = mi::examples::vk::create_linear_sampler(m_device);

    // Create the render resources for the material
    //
    // Create the storage buffer for the material's read-only data
    mi::Size num_segments = m_target_code->get_ro_data_segment_count();
    if (num_segments > 0)
    {
        if (num_segments > 1)
        {
            std::cerr << "Multiple data segments are defined for read-only data."
                << " This should not be the case if a read-only segment or SSBO is used.\n";
            terminate();
        }

        m_ro_data_buffer = mi::examples::vk::create_storage_buffer(
            m_device, m_physical_device, m_graphics_queue, m_command_pool,
            m_target_code->get_ro_data_segment_data(0),
            m_target_code->get_ro_data_segment_size(0));
    }

    // Record the indices of each texture in their respective array
    // e.g. the indices of 2D textures in the m_material_textures_2d array
    std::vector<uint32_t> material_textures_indices;

    // Create the textures for the material
    if (m_target_code->get_texture_count() > 0)
    {
        // The first texture (index = 0) is always the invalid texture in MDL
        material_textures_indices.reserve(m_target_code->get_texture_count() - 1);

        for (mi::Size i = 1; i < m_target_code->get_texture_count(); i++)
        {
            Vulkan_texture texture = create_material_texture(
                m_device, m_physical_device, m_graphics_queue, m_command_pool,
                m_transaction.get(), m_image_api.get(), m_target_code.get(), i);

            switch (m_target_code->get_texture_shape(i))
            {
            case mi::neuraylib::ITarget_code::Texture_shape_2d:
                material_textures_indices.push_back(static_cast<uint32_t>(m_material_textures_2d.size()));
                m_material_textures_2d.push_back(texture);
                break;
            case mi::neuraylib::ITarget_code::Texture_shape_3d:
            case mi::neuraylib::ITarget_code::Texture_shape_bsdf_data:
                material_textures_indices.push_back(static_cast<uint32_t>(m_material_textures_3d.size()));
                m_material_textures_3d.push_back(texture);
                break;
            default:
                std::cerr << "Unsupported texture shape!" << std::endl;
                terminate();
                break;
            }
        }
    }

    create_descriptor_set_layouts();
    create_pipeline_layouts();
    create_accumulation_image();
    create_pipelines();
    create_render_params_buffers();
    create_environment_map();
    create_descriptor_pool_and_sets();
    create_query_pool();

    // Initialize render parameters
    m_render_params.progressive_iteration = 0;
    m_render_params.max_path_length = m_options.max_path_length;
    m_render_params.samples_per_iteration = m_options.samples_per_iteration;

    m_render_params.point_light_pos = m_options.light_pos;
    m_render_params.point_light_intensity = m_options.light_enabled
        ? std::max(std::max(m_options.light_intensity.x, m_options.light_intensity.y), m_options.light_intensity.z)
        : 0.0f;
    m_render_params.point_light_color = m_render_params.point_light_intensity > 0.0f
        ? m_options.light_intensity / m_render_params.point_light_intensity
        : m_options.light_intensity;
    m_render_params.environment_intensity_factor = m_options.hdr_intensity;
    m_render_params.environment_rotation = m_options.hdr_rotate;
    m_render_params.background_color = m_options.background_color;
    m_render_params.background_color_enabled = m_options.background_color_enabled ? 1 : 0;
    m_render_params.flip_texcoord_v = m_options.materialxtest_mode ? 1 : 0;

    const float fov = m_options.cam_fov;
    const float to_radians = static_cast<float>(M_PI / 180.0);
    m_render_params.cam_focal = 1.0f / mi::math::tan(fov / 2.0f * to_radians);

    // Setup camera
    const mi::Float32_3 camera_pos = m_options.cam_pos;
    mi::Float32_3 inv_dir = camera_pos - m_options.cam_lookat;
    m_camera_state.base_distance = mi::math::length(inv_dir);
    inv_dir = inv_dir / m_camera_state.base_distance;
    m_camera_state.phi = mi::math::atan2(inv_dir.x, inv_dir.z);
    m_camera_state.theta = mi::math::acos(inv_dir.y);
    m_camera_state.zoom = 0;

    update_camera_render_params(m_camera_state);
}

void Df_vulkan_app::cleanup_resources()
{
    // In headless mode we output the accumulation buffers to files
    if (m_options.no_window)
        write_accum_images_to_file();

    // Cleanup resources
    for (Vulkan_texture& texture : m_material_textures_3d)
        texture.destroy(m_device);
    for (Vulkan_texture& texture : m_material_textures_2d)
        texture.destroy(m_device);
    m_ro_data_buffer.destroy(m_device);

    m_environment_map.destroy(m_device);

    for (Vulkan_buffer& buffer : m_render_params_buffers)
        buffer.destroy(m_device);

    vkDestroyQueryPool(m_device, m_query_pool, nullptr);
    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_display_descriptor_set_layout, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_path_trace_descriptor_set_layout, nullptr);
    vkDestroyPipelineLayout(m_device, m_display_pipeline_layout, nullptr);
    vkDestroyPipelineLayout(m_device, m_path_trace_pipeline_layout, nullptr);
    vkDestroyPipeline(m_device, m_path_trace_pipeline, nullptr);
    vkDestroyPipeline(m_device, m_display_pipeline, nullptr);
    vkDestroyRenderPass(m_device, m_path_trace_render_pass, nullptr);
    vkDestroySampler(m_device, m_linear_sampler, nullptr);

    m_accum_image.destroy(m_device);

    glslang::FinalizeProcess();
}

// All frame buffer size dependent resources need to be recreated
// when the swapchain is recreated due to not being optimal anymore
// or because the window was resized.
void Df_vulkan_app::recreate_size_dependent_resources()
{
    vkDestroyPipeline(m_device, m_path_trace_pipeline, nullptr);
    vkDestroyPipeline(m_device, m_display_pipeline, nullptr);
    vkDestroyRenderPass(m_device, m_path_trace_render_pass, nullptr);
    m_accum_image.destroy(m_device);

    create_accumulation_image();
    create_pipelines();

    update_accumulation_image_descriptors();
}

// Updates the application logic. This is called right before the
// next frame is rendered.
void Df_vulkan_app::update(float /*elapsed_seconds*/, uint32_t frame_index)
{
    uint64_t timestamps[2];
    VkResult result = vkGetQueryPoolResults(m_device, m_query_pool, frame_index * 2, 2,
        sizeof(uint64_t) * 2, timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
    if (result == VK_SUCCESS)
    {
        auto time_now = glfwGetTime();
        if (time_now - m_last_stats_update > 0.5 || m_first_stats_update)
        {
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(m_physical_device, &device_properties);

            m_render_time = (float)((timestamps[1] - timestamps[0])
                * (double)device_properties.limits.timestampPeriod * 1e-6);

            m_first_stats_update = false;
            m_last_stats_update = time_now;
        }
    }

    if (m_camera_moved)
        m_render_params.progressive_iteration = 0;

    // Update current frame's render parameters uniform buffer
    std::memcpy(m_render_params_buffers[frame_index].mapped_data,
        &m_render_params, sizeof(Render_params));

    m_render_params.progressive_iteration += m_options.samples_per_iteration;
}

// Populates the current frame's command buffer.
void Df_vulkan_app::render(VkCommandBuffer command_buffer, uint32_t frame_index, uint32_t image_index)
{
    // Path trace compute pass
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_path_trace_pipeline);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        m_path_trace_pipeline_layout, 0, 1, &m_path_trace_descriptor_sets[frame_index],
        0, nullptr);

    if (m_camera_moved)
    {
        m_camera_moved = false;

        VkClearColorValue clear_color = { 0.0f, 0.0f, 0.0f, 0.0f };

        VkImageSubresourceRange clear_range;
        clear_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        clear_range.baseMipLevel = 0;
        clear_range.levelCount = 1;
        clear_range.baseArrayLayer = 0;
        clear_range.layerCount = 1;

        vkCmdClearColorImage(command_buffer,
            m_accum_image.image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1, &clear_range);
    }

    vkCmdResetQueryPool(command_buffer, m_query_pool, frame_index * 2, 2);
    vkCmdWriteTimestamp(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, m_query_pool, frame_index * 2);

    uint32_t group_count_x = (m_image_width + g_local_size_x - 1) / g_local_size_x;
    uint32_t group_count_y = (m_image_width + g_local_size_y - 1) / g_local_size_y;
    vkCmdDispatch(command_buffer, group_count_x, group_count_y, 1);

    vkCmdWriteTimestamp(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_query_pool, frame_index * 2 + 1);

    mi::examples::vk::transitionImageLayout(command_buffer,
        /*image=*/           m_accum_image.image,
        /*src_access_mask=*/ VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
        /*dst_access_mask=*/ VK_ACCESS_SHADER_READ_BIT,
        /*old_layout=*/      VK_IMAGE_LAYOUT_GENERAL,
        /*new_layout=*/      VK_IMAGE_LAYOUT_GENERAL,
        /*src_stage_mask=*/  VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        /*dst_stage_mask=*/  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    // Display render pass
    VkRenderPassBeginInfo render_pass_begin_info = {};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = m_main_render_pass;
    render_pass_begin_info.framebuffer = m_framebuffers[image_index];
    render_pass_begin_info.renderArea = { {0, 0}, {m_image_width, m_image_height} };

    VkClearValue clear_values[2];
    clear_values[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clear_values[1].depthStencil = { 1.0f, 0 };
    render_pass_begin_info.clearValueCount = std::size(clear_values);
    render_pass_begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(
        command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(
        command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_display_pipeline);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_display_pipeline_layout, 0, 1, &m_display_descriptor_set, 0, nullptr);

    vkCmdPushConstants(command_buffer, m_display_pipeline_layout,
        VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &m_display_buffer_index);

    vkCmdDraw(command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(command_buffer);
}

// Gets called when the window is resized.
void Df_vulkan_app::resized_callback(uint32_t /*width*/, uint32_t /*height*/)
{
    m_camera_moved = true;
}

void Df_vulkan_app::update_camera_render_params(const Camera_state& cam_state)
{
    m_render_params.cam_dir.x = -mi::math::sin(cam_state.phi) * mi::math::sin(cam_state.theta);
    m_render_params.cam_dir.y = -mi::math::cos(cam_state.theta);
    m_render_params.cam_dir.z = -mi::math::cos(cam_state.phi) * mi::math::sin(cam_state.theta);

    m_render_params.cam_right.x = mi::math::cos(cam_state.phi);
    m_render_params.cam_right.y = 0.0f;
    m_render_params.cam_right.z = -mi::math::sin(cam_state.phi);

    m_render_params.cam_up.x = -mi::math::sin(cam_state.phi) * mi::math::cos(cam_state.theta);
    m_render_params.cam_up.y = mi::math::sin(cam_state.theta);
    m_render_params.cam_up.z = -mi::math::cos(cam_state.phi) * mi::math::cos(cam_state.theta);

    const float dist = cam_state.base_distance * mi::math::pow(0.95f, cam_state.zoom);
    m_render_params.cam_pos.x = -m_render_params.cam_dir.x * dist;
    m_render_params.cam_pos.y = -m_render_params.cam_dir.y * dist;
    m_render_params.cam_pos.z = -m_render_params.cam_dir.z * dist;
}

void Df_vulkan_app::create_accumulation_image()
{
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = g_accumulation_texture_format;
    image_create_info.extent.width = m_image_width;
    image_create_info.extent.height = m_image_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_STORAGE_BIT
        | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK(vkCreateImage(m_device, &image_create_info, nullptr, &m_accum_image.image));
    m_accum_image.device_memory = mi::examples::vk::allocate_and_bind_image_memory(
        m_device, m_physical_device, m_accum_image.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    { // Transition image layout
        mi::examples::vk::Temporary_command_buffer command_buffer(m_device, m_command_pool);
        command_buffer.begin();

        mi::examples::vk::transitionImageLayout(command_buffer.get(),
            /*image=*/           m_accum_image.image,
            /*src_access_mask=*/ 0,
            /*dst_access_mask=*/ VK_ACCESS_SHADER_READ_BIT,
            /*old_layout=*/      VK_IMAGE_LAYOUT_UNDEFINED,
            /*new_layout=*/      VK_IMAGE_LAYOUT_GENERAL,
            /*src_stage_mask=*/  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            /*dst_stage_mask=*/  VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        command_buffer.end_and_submit(m_graphics_queue);
    }

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = g_accumulation_texture_format;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    image_view_create_info.image = m_accum_image.image;
    VK_CHECK(vkCreateImageView(
        m_device, &image_view_create_info, nullptr, &m_accum_image.image_view));
}

VkShaderModule Df_vulkan_app::create_path_trace_shader_module()
{
    std::string df_glsl_source = m_target_code->get_code();

    std::string path_trace_shader_source = mi::examples::io::read_text_file(
        mi::examples::io::get_executable_folder() + "/MaterialXRenderMdlStandalone/" + "path_trace.comp");

    std::vector<std::string> defines;
    defines.push_back("LOCAL_SIZE_X=" + std::to_string(g_local_size_x));
    defines.push_back("LOCAL_SIZE_Y=" + std::to_string(g_local_size_y));

    defines.push_back("BINDING_RENDER_PARAMS=" + std::to_string(g_binding_render_params));
    defines.push_back("BINDING_ENV_MAP=" + std::to_string(g_binding_environment_map));
    defines.push_back("BINDING_ENV_MAP_SAMPLING_DATA=" + std::to_string(g_binding_environment_sampling_data));
    defines.push_back("BINDING_BEAUTY_BUFFER=" + std::to_string(g_binding_beauty_buffer));

    defines.push_back("MDL_SET_MATERIAL_TEXTURES_2D=" + std::to_string(g_set_material_textures));
    defines.push_back("MDL_SET_MATERIAL_TEXTURES_3D=" + std::to_string(g_set_material_textures));
    defines.push_back("MDL_SET_MATERIAL_RO_DATA_SEGMENT=" + std::to_string(g_set_ro_data_buffer));
    defines.push_back("MDL_BINDING_MATERIAL_TEXTURES_2D=" + std::to_string(g_binding_material_textures_2d));
    defines.push_back("MDL_BINDING_MATERIAL_TEXTURES_3D=" + std::to_string(g_binding_material_textures_3d));
    defines.push_back("MDL_BINDING_MATERIAL_RO_DATA_SEGMENT=" + std::to_string(g_binding_ro_data_buffer));

    defines.push_back("NUM_TEX_RESULTS=16");

    if (m_options.enable_ro_segment)
        defines.push_back("USE_RO_DATA_SEGMENT");

    // string constants
    for (size_t i = 1, n = m_target_code ? m_target_code->get_string_constant_count() : 0; i < n; ++i)
        defines.push_back("MDL_STRING_CONSTANT_" + std::string(m_target_code->get_string_constant(i)) + "=" + std::to_string(i));

    // Check if functions for backface were generated
    for (mi::Size i = 0; i < m_target_code->get_callable_function_count(); i++)
    {
        const char* fname = m_target_code->get_callable_function(i);

        if (std::strcmp(fname, "mdl_backface_bsdf_sample") == 0)
            defines.push_back("MDL_HAS_BACKFACE_BSDF");
        else if (std::strcmp(fname, "mdl_backface_edf_sample") == 0)
            defines.push_back("MDL_HAS_BACKFACE_EDF");
        else if (std::strcmp(fname, "mdl_backface_emission_intensity") == 0)
            defines.push_back("MDL_HAS_BACKFACE_EMISSION_INTENSITY");
    }

    auto t0 = std::chrono::steady_clock::now();
    VkShaderModule shader_module = mi::examples::vk::create_shader_module_from_sources(
        m_device, { df_glsl_source, path_trace_shader_source }, EShLangCompute,
        defines, m_options.enable_shader_optimization);
    auto t1 = std::chrono::steady_clock::now();
    if (!m_path_trace_pipeline) // Print only the first time
        mi::examples::log::info("Compile GLSL to SPIR-V: %.3fs", std::chrono::duration<float>(t1 - t0).count());

    return shader_module;
}

// Creates the descriptors set layout which is used to create the
// pipeline layout. Here the number of material resources is declared.
void Df_vulkan_app::create_descriptor_set_layouts()
{
    {
        auto make_binding = [](uint32_t binding, VkDescriptorType type, uint32_t count)
        {
            VkDescriptorSetLayoutBinding layout_binding = {};
            layout_binding.binding = binding;
            layout_binding.descriptorType = type;
            layout_binding.descriptorCount = count;
            layout_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            return layout_binding;
        };

        // We reserve enough space for the maximum amount of textures we expect (arbitrary number here)
        // in combination with descriptor indexing. The partially bound flag is used because all textures
        // in MDL share the same index range, but in GLSL we must use separate arrays. This leads to "holes"
        // in the GLSL texture arrays since for each index only one of the texture arrays is populated.
        // We can also leave the descriptor sets empty with the partially bound flag, in case no
        // textures of the corresponding shapes is used.
        // See Df_vulkan_app::create_descriptor_pool_and_sets() for how the descriptor sets are populated.
        const uint32_t max_num_textures = 100;

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.push_back(make_binding(g_binding_render_params, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1));
        bindings.push_back(make_binding(g_binding_environment_map, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1));
        bindings.push_back(make_binding(g_binding_environment_sampling_data, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1));
        bindings.push_back(make_binding(g_binding_material_textures_2d, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, max_num_textures));
        bindings.push_back(make_binding(g_binding_material_textures_3d, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, max_num_textures));
        bindings.push_back(make_binding(g_binding_beauty_buffer, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1));
        bindings.push_back(make_binding(g_binding_ro_data_buffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1));

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
        descriptor_set_layout_create_info.sType
            = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
        descriptor_set_layout_create_info.pBindings = bindings.data();

        std::vector<VkDescriptorBindingFlags> binding_flags(bindings.size(), 0);
        binding_flags[3] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT; // g_binding_material_textures_2d
        binding_flags[4] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT; // g_binding_material_textures_3d

        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT descriptor_set_layout_binding_flags = {};
        descriptor_set_layout_binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        descriptor_set_layout_binding_flags.bindingCount = static_cast<uint32_t>(bindings.size());
        descriptor_set_layout_binding_flags.pBindingFlags = binding_flags.data();
        descriptor_set_layout_create_info.pNext = &descriptor_set_layout_binding_flags;

        VK_CHECK(vkCreateDescriptorSetLayout(
            m_device, &descriptor_set_layout_create_info, nullptr, &m_path_trace_descriptor_set_layout));
    }

    {
        VkDescriptorSetLayoutBinding layout_bindings[5];
        for (uint32_t i = 0; i < 5; i++)
        {
            layout_bindings[i].binding = i;
            layout_bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            layout_bindings[i].descriptorCount = 1;
            layout_bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            layout_bindings[i].pImmutableSamplers = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
        descriptor_set_layout_create_info.sType
            = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.bindingCount = std::size(layout_bindings);
        descriptor_set_layout_create_info.pBindings = layout_bindings;

        VK_CHECK(vkCreateDescriptorSetLayout(
            m_device, &descriptor_set_layout_create_info, nullptr, &m_display_descriptor_set_layout));
    }
}

void Df_vulkan_app::create_pipeline_layouts()
{
    { // Path trace compute pipeline layout
        VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = 1;
        pipeline_layout_create_info.pSetLayouts = &m_path_trace_descriptor_set_layout;
        pipeline_layout_create_info.pushConstantRangeCount = 0;
        pipeline_layout_create_info.pPushConstantRanges = nullptr;

        VK_CHECK(vkCreatePipelineLayout(
            m_device, &pipeline_layout_create_info, nullptr, &m_path_trace_pipeline_layout));
    }

    { // Display graphics pipeline layout
        VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = 1;
        pipeline_layout_create_info.pSetLayouts = &m_display_descriptor_set_layout;
        pipeline_layout_create_info.pushConstantRangeCount = 0;
        pipeline_layout_create_info.pPushConstantRanges = nullptr;

        VK_CHECK(vkCreatePipelineLayout(
            m_device, &pipeline_layout_create_info, nullptr, &m_display_pipeline_layout));
    }
}

void Df_vulkan_app::create_pipelines()
{
    { // Create path trace compute pipeline
        VkShaderModule path_trace_compute_shader = create_path_trace_shader_module();

        VkPipelineShaderStageCreateInfo compute_shader_stage = {};
        compute_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        compute_shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        compute_shader_stage.module = path_trace_compute_shader;
        compute_shader_stage.pName = "main";

        VkComputePipelineCreateInfo pipeline_create_info = {};
        pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipeline_create_info.stage = compute_shader_stage;
        pipeline_create_info.layout = m_path_trace_pipeline_layout;

        vkCreateComputePipelines(
            m_device, nullptr, 1, &pipeline_create_info, nullptr, &m_path_trace_pipeline);

        vkDestroyShaderModule(m_device, path_trace_compute_shader, nullptr);
    }

    { // Create display graphics pipeline
        VkShaderModule fullscreen_triangle_vertex_shader
            = mi::examples::vk::create_shader_module_from_file(
                m_device, "MaterialXRenderMdlStandalone/display.vert", EShLangVertex, {}, m_options.enable_shader_optimization);
        VkShaderModule display_fragment_shader
            = mi::examples::vk::create_shader_module_from_file(
                m_device, "MaterialXRenderMdlStandalone/display.frag", EShLangFragment, {}, m_options.enable_shader_optimization);

        m_display_pipeline = mi::examples::vk::create_fullscreen_triangle_graphics_pipeline(
            m_device, m_display_pipeline_layout, fullscreen_triangle_vertex_shader,
            display_fragment_shader, m_main_render_pass, 0, m_image_width, m_image_height, false);

        vkDestroyShaderModule(m_device, fullscreen_triangle_vertex_shader, nullptr);
        vkDestroyShaderModule(m_device, display_fragment_shader, nullptr);
    }
}

void Df_vulkan_app::create_render_params_buffers()
{
    m_render_params_buffers.resize(m_image_count);

    for (uint32_t i = 0; i < m_image_count; i++)
    {
        VkBufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.size = sizeof(Render_params);
        create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateBuffer(
            m_device, &create_info, nullptr, &m_render_params_buffers[i].buffer));

        m_render_params_buffers[i].device_memory = mi::examples::vk::allocate_and_bind_buffer_memory(
            m_device, m_physical_device, m_render_params_buffers[i].buffer,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VK_CHECK(vkMapMemory(m_device, m_render_params_buffers[i].device_memory,
            0, sizeof(Render_params), 0, &m_render_params_buffers[i].mapped_data));
    }
}

void Df_vulkan_app::create_environment_map()
{
    std::string path = m_options.hdr_file;
    if (!mi::examples::io::is_absolute_path(m_options.hdr_file))
        path = mi::examples::io::get_executable_folder() + "/" + path;

    if (!mi::examples::io::exists(path))
        exit_failure("Failed to load environment map.");

    // Load environment texture
    m_environment_map.create(m_device, m_physical_device, m_command_pool, m_graphics_queue, m_image_api.get(), m_transaction.get(), path);
    m_render_params.environment_inv_integral = m_environment_map.environment_inv_integral;
}

// Creates the descriptor pool and set that hold enough space for all
// material resources, and are used during rendering to access the
// the resources.
void Df_vulkan_app::create_descriptor_pool_and_sets()
{
    // Reserve enough space. This is way too much, but sizing them perfectly
    // would make the code less readable.
    const VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 }
    };

    VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.maxSets = m_image_count + 2; // img_cnt for path_trace + 1 set for display and imgui each
    descriptor_pool_create_info.poolSizeCount = std::size(pool_sizes);
    descriptor_pool_create_info.pPoolSizes = pool_sizes;
    descriptor_pool_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // required for imgui

    VK_CHECK(vkCreateDescriptorPool(
        m_device, &descriptor_pool_create_info, nullptr, &m_descriptor_pool));

    // Allocate descriptor set
    {
        std::vector<VkDescriptorSetLayout> set_layouts(
            m_image_count, m_path_trace_descriptor_set_layout);

        VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {};
        descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_alloc_info.descriptorPool = m_descriptor_pool;
        descriptor_set_alloc_info.descriptorSetCount = m_image_count;
        descriptor_set_alloc_info.pSetLayouts = set_layouts.data();

        m_path_trace_descriptor_sets.resize(m_image_count);
        VK_CHECK(vkAllocateDescriptorSets(
            m_device, &descriptor_set_alloc_info, m_path_trace_descriptor_sets.data()));
    }

    {
        VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {};
        descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_alloc_info.descriptorPool = m_descriptor_pool;
        descriptor_set_alloc_info.descriptorSetCount = 1;
        descriptor_set_alloc_info.pSetLayouts = &m_display_descriptor_set_layout;

        VK_CHECK(vkAllocateDescriptorSets(
            m_device, &descriptor_set_alloc_info, &m_display_descriptor_set));
    }

    // Populate descriptor sets
    std::vector<VkDescriptorBufferInfo> descriptor_buffer_infos;
    std::vector<VkDescriptorImageInfo> descriptor_image_infos;
    std::vector<VkWriteDescriptorSet> descriptor_writes;

    // Reserve enough space. This is way too much, but sizing them perfectly
    // would make the code less readable.
    descriptor_buffer_infos.reserve(100);
    descriptor_image_infos.reserve(1000);

    for (uint32_t i = 0; i < m_image_count; i++)
    {
        VkWriteDescriptorSet descriptor_write = {};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = m_path_trace_descriptor_sets[i];

        { // Render parameter buffer
            VkDescriptorBufferInfo descriptor_buffer_info = {};
            descriptor_buffer_info.buffer = m_render_params_buffers[i].buffer;
            descriptor_buffer_info.range = VK_WHOLE_SIZE;
            descriptor_buffer_infos.push_back(descriptor_buffer_info);

            VkWriteDescriptorSet descriptor_write = {};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = m_path_trace_descriptor_sets[i];
            descriptor_write.dstBinding = g_binding_render_params;
            descriptor_write.descriptorCount = 1;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptor_write.pBufferInfo = &descriptor_buffer_infos.back();
            descriptor_writes.push_back(descriptor_write);
        }

        { // Environment map
            VkDescriptorImageInfo descriptor_image_info = {};
            descriptor_image_info.sampler = m_environment_map.sampler;
            descriptor_image_info.imageView = m_environment_map.texture.image_view;
            descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            descriptor_image_infos.push_back(descriptor_image_info);

            VkWriteDescriptorSet descriptor_write = {};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = m_path_trace_descriptor_sets[i];
            descriptor_write.dstBinding = g_binding_environment_map;
            descriptor_write.descriptorCount = 1;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_write.pImageInfo = &descriptor_image_infos.back();
            descriptor_writes.push_back(descriptor_write);
        }

        { // Environment map sampling data
            VkDescriptorBufferInfo descriptor_buffer_info = {};
            descriptor_buffer_info.buffer = m_environment_map.sampling_data_buffer.buffer;
            descriptor_buffer_info.range = VK_WHOLE_SIZE;
            descriptor_buffer_infos.push_back(descriptor_buffer_info);

            VkWriteDescriptorSet descriptor_write = {};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = m_path_trace_descriptor_sets[i];
            descriptor_write.dstBinding = g_binding_environment_sampling_data;
            descriptor_write.descriptorCount = 1;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptor_write.pBufferInfo = &descriptor_buffer_infos.back();
            descriptor_writes.push_back(descriptor_write);
        }

        // Material textures
        // 
        // We rely on the partially bound bit when creating the descriptor set layout,
        // so we can leave holes in the descriptor sets (or leave them empty).
        // For each MDL texture index only one of the GLSL texture arrays is populated.
        size_t texture_2d_index = 0;
        size_t texture_3d_index = 0;

        for (mi::Size tex = 1; tex < m_target_code->get_texture_count(); tex++)
        {
            VkDescriptorImageInfo descriptor_image_info = {};
            descriptor_image_info.sampler = m_linear_sampler;
            descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkWriteDescriptorSet descriptor_write = {};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = m_path_trace_descriptor_sets[i];
            descriptor_write.dstArrayElement = static_cast<uint32_t>(tex - 1);
            descriptor_write.descriptorCount = 1;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            mi::neuraylib::ITarget_code::Texture_shape shape
                = m_target_code->get_texture_shape(tex);
            if (shape == mi::neuraylib::ITarget_code::Texture_shape_2d)
            {
                descriptor_image_info.imageView = m_material_textures_2d[texture_2d_index++].image_view;
                descriptor_image_infos.push_back(descriptor_image_info);

                descriptor_write.dstBinding = g_binding_material_textures_2d;
                descriptor_write.pImageInfo = &descriptor_image_infos.back();
                descriptor_writes.push_back(descriptor_write);
            }
            else if (shape == mi::neuraylib::ITarget_code::Texture_shape_3d
                || shape == mi::neuraylib::ITarget_code::Texture_shape_bsdf_data)
            {
                descriptor_image_info.imageView = m_material_textures_3d[texture_3d_index++].image_view;
                descriptor_image_infos.push_back(descriptor_image_info);

                descriptor_write.dstBinding = g_binding_material_textures_3d;
                descriptor_write.pImageInfo = &descriptor_image_infos.back();
                descriptor_writes.push_back(descriptor_write);
            }
        }

        // Read-only data buffer
        if (m_ro_data_buffer.buffer)
        {
            VkDescriptorBufferInfo descriptor_buffer_info = {};
            descriptor_buffer_info.buffer = m_ro_data_buffer.buffer;
            descriptor_buffer_info.range = VK_WHOLE_SIZE;
            descriptor_buffer_infos.push_back(descriptor_buffer_info);

            VkWriteDescriptorSet descriptor_write = {};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.dstSet = m_path_trace_descriptor_sets[i];
            descriptor_write.dstBinding = g_binding_ro_data_buffer;
            descriptor_write.descriptorCount = 1;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptor_write.pBufferInfo = &descriptor_buffer_infos.back();
            descriptor_writes.push_back(descriptor_write);
        }
    }

    vkUpdateDescriptorSets(
        m_device, static_cast<uint32_t>(descriptor_writes.size()),
        descriptor_writes.data(), 0, nullptr);

    update_accumulation_image_descriptors();
}

void Df_vulkan_app::create_query_pool()
{
    VkQueryPoolCreateInfo query_pool_create_info = {};
    query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
    query_pool_create_info.queryCount = m_image_count * 2;

    VK_CHECK(vkCreateQueryPool(m_device, &query_pool_create_info, nullptr, &m_query_pool));

    mi::examples::vk::Temporary_command_buffer command_buffer(m_device, m_command_pool);
    command_buffer.begin();
    vkCmdResetQueryPool(command_buffer.get(), m_query_pool, 0, query_pool_create_info.queryCount);
    command_buffer.end_and_submit(m_graphics_queue);
}

void Df_vulkan_app::update_accumulation_image_descriptors()
{
    std::vector<VkWriteDescriptorSet> descriptor_writes;

    std::vector<VkDescriptorImageInfo> descriptor_image_infos;
    descriptor_image_infos.reserve(m_image_count + 1);

    for (uint32_t i = 0; i < m_image_count; i++)
    {
        VkDescriptorImageInfo descriptor_image_info = {};
        descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet descriptor_write = {};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = m_path_trace_descriptor_sets[i];
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

        descriptor_image_info.imageView = m_accum_image.image_view;
        descriptor_image_infos.push_back(descriptor_image_info);

        descriptor_write.dstBinding = g_binding_beauty_buffer;
        descriptor_write.pImageInfo = &descriptor_image_infos.back();
        descriptor_writes.push_back(descriptor_write);
    }

    VkDescriptorImageInfo descriptor_info = {};
    descriptor_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet descriptor_write = {};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = m_display_descriptor_set;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorCount = 1;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    descriptor_info.imageView = m_accum_image.image_view;
    descriptor_image_infos.push_back(descriptor_info);
    descriptor_write.dstBinding = 0;
    descriptor_write.pImageInfo = &descriptor_image_infos.back();
    descriptor_writes.push_back(descriptor_write);

    vkUpdateDescriptorSets(
        m_device, static_cast<uint32_t>(descriptor_writes.size()),
        descriptor_writes.data(), 0, nullptr);
}

void Df_vulkan_app::write_accum_images_to_file()
{
    auto export_pixels_rgba32f = [&](const std::vector<uint8_t>& pixels, const std::string& filename)
    {
        mi::base::Handle<mi::neuraylib::ICanvas> canvas(
            m_image_api->create_canvas("Color", m_image_width, m_image_height));
        mi::base::Handle<mi::neuraylib::ITile> tile(canvas->get_tile());
        std::memcpy(tile->get_data(), pixels.data(), pixels.size());
        canvas = m_image_api->convert(canvas.get(), "Rgb_fp");
        mi::base::Handle<mi::IMap> export_options(m_transaction->create<mi::IMap>("Map<Interface>"));
        mi::base::Handle<mi::IUint32> quality(m_transaction->create<mi::IUint32>("Uint32"));
        quality->set_value(100);
        export_options->insert("jpg:quality", quality.get());
        mi::base::Handle<mi::IBoolean> force_default_gamma(m_transaction->create<mi::IBoolean>("Boolean"));
        force_default_gamma->set_value(true);
        export_options->insert("force_default_gamma", force_default_gamma.get());
        m_mdl_impexp_api->export_canvas(filename.c_str(), canvas.get(), export_options.get());
    };

    uint32_t image_bpp = mi::examples::vk::get_image_format_bpp(g_accumulation_texture_format);
    std::vector<uint8_t> image_pixel = mi::examples::vk::copy_image_to_buffer(
        m_device, m_physical_device, m_command_pool, m_graphics_queue,
        m_accum_image.image, m_image_width, m_image_height, image_bpp,
        VK_IMAGE_LAYOUT_GENERAL, false);

    std::string output_path = m_options.output_file;
    if (!mi::examples::io::is_absolute_path(output_path))
        output_path = mi::examples::io::get_working_directory() + "/" + output_path;

    mi::examples::log::info("Saving output image to: \"%s\"", output_path.c_str());
    export_pixels_rgba32f(image_pixel, output_path);
}


//------------------------------------------------------------------------------
// MDL material compilation helpers
//------------------------------------------------------------------------------
mi::neuraylib::IFunction_call* create_material_instance(
    mi::neuraylib::IMdl_factory* mdl_factory,
    mi::neuraylib::ITransaction* transaction,
    const std::string& qualified_module_name,
    const std::string& material_simple_name)
{
    // Get the database name for the module we loaded and check if
    // the module exists in the database.
    mi::base::Handle<const mi::IString> module_db_name(
        mdl_factory->get_db_definition_name(qualified_module_name.c_str()));
    mi::base::Handle<const mi::neuraylib::IModule> module(
        transaction->access<mi::neuraylib::IModule>(module_db_name->get_c_str()));
    if (!module)
        exit_failure("Failed to access the loaded module.");

    // To access the material in the database we need to know the exact material
    // signature, so we append the arguments to the full name (with module).
    std::string material_db_name
        = std::string(module_db_name->get_c_str()) + "::" + material_simple_name;
    material_db_name = mi::examples::mdl::add_missing_material_signature(
        module.get(), material_db_name.c_str());

    mi::base::Handle<const mi::neuraylib::IFunction_definition> material_definition(
        transaction->access<mi::neuraylib::IFunction_definition>(material_db_name.c_str()));
    if (!material_definition)
        exit_failure("Failed to access material definition '%s'.", material_db_name.c_str());

    // Create material instance
    mi::Sint32 result;
    mi::base::Handle<mi::neuraylib::IFunction_call> material_instance(
        material_definition->create_function_call(nullptr, &result));
    if (result != 0)
        exit_failure("Failed to instantiate material '%s'.", material_db_name.c_str());

    material_instance->retain();
    return material_instance.get();
}

mi::neuraylib::ICompiled_material* compile_material_instance(
    mi::neuraylib::IMdl_factory *mdl_factory,
    mi::neuraylib::ITransaction *transaction,
    mi::neuraylib::IFunction_call* function_call,
    mi::neuraylib::IMdl_execution_context* context)
{
    // a material instance is a special function call
    mi::base::Handle<const mi::neuraylib::IMaterial_instance> material_instance(
        function_call->get_interface<mi::neuraylib::IMaterial_instance>());

    // convert to target type SID_MATERIAL
    // MDL 1.9 supports custom material types for generalized AOV support
    mi::base::Handle<mi::neuraylib::IType_factory> tf(
        mdl_factory->create_type_factory(transaction));
    mi::base::Handle<const mi::neuraylib::IType> standard_material_type(
        tf->get_predefined_struct(mi::neuraylib::IType_struct::SID_MATERIAL));
    context->set_option("target_type", standard_material_type.get());

    // Since this application is not interactive the user cannot change parameters
    // at runtime. In cases where this is needed the MDL SDK offers a class compilation
    // mode that allows to change parameter edits without recompilation.
    // please refer to the MDL SDK examples for this. E.g., example df_vulkan
    mi::Uint32 compile_flags = mi::neuraylib::IMaterial_instance::DEFAULT_OPTIONS;

    // compile the material
    mi::base::Handle<mi::neuraylib::ICompiled_material> compiled_material(
        material_instance->create_compiled_material(compile_flags, context));
    check_success(print_messages(context));

    compiled_material->retain();
    return compiled_material.get();
}

const mi::neuraylib::ITarget_code* generate_glsl_code(
    mi::neuraylib::ICompiled_material* compiled_material,
    mi::neuraylib::IMdl_backend_api* mdl_backend_api,
    mi::neuraylib::ITransaction* transaction,
    mi::neuraylib::IMdl_execution_context* context,
    const Options& options)
{
    // Add compiled material to link unit
    mi::base::Handle<mi::neuraylib::IMdl_backend> be_glsl(
        mdl_backend_api->get_backend(mi::neuraylib::IMdl_backend_api::MB_GLSL));

    check_success(be_glsl->set_option("glsl_version", "450") == 0);
    if (!options.disable_ssbo && !options.enable_ro_segment)
    {
        check_success(be_glsl->set_option("glsl_place_uniforms_into_ssbo", "on") == 0);
        check_success(be_glsl->set_option("glsl_uniform_ssbo_binding",
            std::to_string(g_binding_ro_data_buffer).c_str()) == 0);
        check_success(be_glsl->set_option("glsl_uniform_ssbo_set",
            std::to_string(g_set_ro_data_buffer).c_str()) == 0);
        check_success(be_glsl->set_option("glsl_max_const_data",
            std::to_string(options.max_const_data).c_str()) == 0);
    }
    if (options.enable_ro_segment)
    {
        check_success(be_glsl->set_option("enable_ro_segment", "on") == 0);
        check_success(be_glsl->set_option("max_const_data",
            std::to_string(options.max_const_data).c_str()) == 0);
    }

    // The number of texture coordinate sets that can be queried from the state.
    // This corresponds to the maximum number of texture coordinate sets provided
    // by the geometry in the scene.
    check_success(be_glsl->set_option("num_texture_spaces", "1") == 0);

    // Material code that is shared among different expressions within the
    // material can be precomputed in the beginning of the material shader execution in `mdl_init`.
    // The results are store in a cache and `num_texture_results` defines the size
    // of this cache as number float4s. Shared results that don't fit into the cache
    // are computed on the fly.
    check_success(be_glsl->set_option("num_texture_results", "16") == 0);

    // For Denoising, the MDL SDK can generate auxiliary functions to compute the
    // input buffers of common denoisers. This includes:
    // - an albedo approximation separated into diffuse and glossy 
    // - shading normal
    // - roughness
    check_success(be_glsl->set_option("enable_auxiliary", "off") == 0);

    // For light path expressions (LPEs), the MDL SDK allows to evaluate and sample
    // selected parts of a material for later post render composition.
    check_success(be_glsl->set_option("df_handle_slot_mode", "none") == 0);

    check_success(be_glsl->set_option("libbsdf_flags_in_bsdf_data", "off") == 0);

    mi::base::Handle<mi::neuraylib::ILink_unit> link_unit(
        be_glsl->create_link_unit(transaction, context));

    // Specify which functions to generate
    std::vector<mi::neuraylib::Target_function_description> function_descs;
    function_descs.emplace_back("init", "mdl_init");
    function_descs.emplace_back("thin_walled", "mdl_thin_walled");
    function_descs.emplace_back("surface.scattering", "mdl_bsdf");
    function_descs.emplace_back("surface.emission.emission", "mdl_edf");
    function_descs.emplace_back("surface.emission.intensity", "mdl_emission_intensity");
    function_descs.emplace_back("volume.absorption_coefficient", "mdl_absorption_coefficient");
    
    // Try to determine if the material is thin walled so we can check
    // if backface functions need to be generated.
    bool is_thin_walled_function = true;
    bool thin_walled_value = false;
    mi::base::Handle<const mi::neuraylib::IExpression> thin_walled_expr(
        compiled_material->lookup_sub_expression("thin_walled"));
    if (thin_walled_expr->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
    {
        mi::base::Handle<const mi::neuraylib::IExpression_constant> thin_walled_const(
            thin_walled_expr->get_interface<const mi::neuraylib::IExpression_constant>());
        mi::base::Handle<const mi::neuraylib::IValue_bool> thin_walled_bool(
            thin_walled_const->get_value<mi::neuraylib::IValue_bool>());

        is_thin_walled_function = false;
        thin_walled_value = thin_walled_bool->get_value();
    }

    // Backfaces could be different for thin walled materials
    bool need_backface_bsdf = false;
    bool need_backface_edf = false;
    bool need_backface_emission_intensity = false;

    if (is_thin_walled_function || thin_walled_value)
    {
        // First, backface DFs are only considered for thin_walled materials

        // Second, we only need to generate new code if surface and backface are different
        need_backface_bsdf =
            compiled_material->get_slot_hash(mi::neuraylib::SLOT_SURFACE_SCATTERING)
            != compiled_material->get_slot_hash(mi::neuraylib::SLOT_BACKFACE_SCATTERING);
        need_backface_edf =
            compiled_material->get_slot_hash(mi::neuraylib::SLOT_SURFACE_EMISSION_EDF_EMISSION)
            != compiled_material->get_slot_hash(mi::neuraylib::SLOT_BACKFACE_EMISSION_EDF_EMISSION);
        need_backface_emission_intensity =
            compiled_material->get_slot_hash(mi::neuraylib::SLOT_SURFACE_EMISSION_INTENSITY)
            != compiled_material->get_slot_hash(mi::neuraylib::SLOT_BACKFACE_EMISSION_INTENSITY);

        // Third, either the bsdf or the edf need to be non-default (black)
        mi::base::Handle<const mi::neuraylib::IExpression> scattering_expr(
            compiled_material->lookup_sub_expression("backface.scattering"));
        mi::base::Handle<const mi::neuraylib::IExpression> emission_expr(
            compiled_material->lookup_sub_expression("backface.emission.emission"));

        if (scattering_expr->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT
            && emission_expr->get_kind() == mi::neuraylib::IExpression::EK_CONSTANT)
        {
            mi::base::Handle<const mi::neuraylib::IExpression_constant> scattering_expr_constant(
                scattering_expr->get_interface<mi::neuraylib::IExpression_constant>());
            mi::base::Handle<const mi::neuraylib::IValue> scattering_value(
                scattering_expr_constant->get_value());

            mi::base::Handle<const mi::neuraylib::IExpression_constant> emission_expr_constant(
                emission_expr->get_interface<mi::neuraylib::IExpression_constant>());
            mi::base::Handle<const mi::neuraylib::IValue> emission_value(
                emission_expr_constant->get_value());

            if (scattering_value->get_kind() == mi::neuraylib::IValue::VK_INVALID_DF
                && emission_value->get_kind() == mi::neuraylib::IValue::VK_INVALID_DF)
            {
                need_backface_bsdf = false;
                need_backface_edf = false;
                need_backface_emission_intensity = false;
            }
        }
    }

    if (need_backface_bsdf)
        function_descs.emplace_back("backface.scattering", "mdl_backface_bsdf");

    if (need_backface_edf)
        function_descs.emplace_back("backface.emission.emission", "mdl_backface_edf");

    if (need_backface_emission_intensity)
        function_descs.emplace_back("backface.emission.intensity", "mdl_backface_emission_intensity");

    link_unit->add_material(
        compiled_material, function_descs.data(), function_descs.size(), context);
    check_success(print_messages(context));

    // Compile cutout_opacity also as standalone version to be used in the anyhit programs
    // to avoid costly precalculation of expressions only used by other expressions.
    // This can be especially useful for anyhit shaders in ray tracing pipelines.
    mi::neuraylib::Target_function_description cutout_opacity_function_desc(
        "geometry.cutout_opacity", "mdl_standalone_cutout_opacity");
    link_unit->add_material(
        compiled_material, &cutout_opacity_function_desc, 1, context);
    check_success(print_messages(context));

    // Generate GLSL code
    auto t0 = std::chrono::steady_clock::now();
    mi::base::Handle<const mi::neuraylib::ITarget_code> target_code(
        be_glsl->translate_link_unit(link_unit.get(), context));
    auto t1 = std::chrono::steady_clock::now();
    check_success(print_messages(context));
    check_success(target_code);
    mi::examples::log::info("Generate GLSL target code: %.3fs", std::chrono::duration<float>(t1 - t0).count());

    target_code->retain();
    return target_code.get();
}


//------------------------------------------------------------------------------
// Command line helpers
//------------------------------------------------------------------------------
void print_usage(char const* prog_name)
{
    std::cout
        << "Usage: " << prog_name << " [options] [<material_name|full_mdle_path>]\n"
        << "Options:\n"
        << "  -h|--help                   print this text and exit\n"

        << "  --mtlx_path <path>          Specify an additional absolute search path location\n"
           "                              (e.g. '/projects/MaterialX'). This path will be queried when\n"
           "                              locating standard data libraries, XInclude references, and\n"
           "                              referenced images. Can occur multiple times.\n"

        << "  --mtlx_library <rel_path>   Specify an additional relative path to a custom data\n"
           "                              library folder (e.g. 'libraries/custom'). MaterialX files\n"
           "                              at the root of this folder will be included in all content\n"
           "                              documents. Can occur multiple times.\n"

        << "  --mtlx_to_mdl <version>     Specify the MDL version to generate.\n"
           "                              Supported values are \"1.6\", \"1.7\", \"1.8\", \"1.9\", ... and\n"
           "                              \"latest\". (default: \"latest\")\n"

        << "  -p|--mdl_path <path>        additional MDL search path, can occur multiple times\n"

        << "  --mat <identifier>          Specify the material to render. The are multiple options:\n"
        << "                              - <MaterialX-filepath>\n"
        << "                              - <MaterialX-filepath>?name=<element name>\n"
        << "                              - <MDL qualified material name>\n"

        << "  --nogui                     Don't open interactive display\n"
        << "  --res <res_x> <res_y>       resolution (default: 1024x768)\n"
        << "  --numimg <n>                swapchain image count (default: 3)\n"
        << "  --device <id>               run on supported GPU <id>\n"
        << "  --spp <num>                 samples per pixel, only used for --nowin (default: 4096)\n"
        << "  --spi <num>                 samples per render loop iteration (default: 8)\n"
        << "  --max_path_length <num>     maximum path length (default: 4)\n"

        << "  -f|--fov <fov>              the camera field of view in degrees (default: 96.0)\n"
        << "  --camera <px> <py> <pz> <fx> <fy> <fz>  Overrides the camera pose defined in the\n"
        << "                                          scene as well as the computed one if the scene\n"
        << "                                          has no camera. Parameters specify position and\n"
        << "                                          focus point.\n"

        << "  -l|--light <x> <y> <z>      adds an omni-directional light with the given position\n"
        << "             <r> <g> <b>      and intensity\n"
        << "  --hdr <path>                hdr image file used for the environment map\n"
        << "                              (default: nvidia/sdk_examples/resources/environment.hdr)\n"
        << "  --hdr_intensity <value>     intensity of the environment map (default: 1.0)\n"
        << "  --hdr_rotate <angle>        Environment rotation in degree (default: 0)\n"
        << "  --background <r> <g> <b>    Constant background color to replace the environment only \n"
        << "                              if directly visible to the camera. (default: <empty>).\n"

        << "  --enable_ro_segment         enable the read-only data segment\n"
        << "  --disable_ssbo              disable use of an ssbo for constants\n"
        << "  --max_const_data <size>     set the maximum size of constants in bytes in the\n"
        << "                              generated code (requires read-only data segment or\n"
        << "                              ssbo, default 1024)\n"
        << "  --vkdebug                   enable the Vulkan validation layers\n"
        << "  --no_shader_opt             disables shader SPIR-V optimization\n"

        << "  --materialxtest_mode        setup image and texcoord space to match the test setup\n"
        << "  -o|--output <path>          image file to write result in nowin mode (default: output.exr)\n"
        << "  -g|--generated <path>       outputs the MDL code generated from MaterialX to a file\n"
        << "  --generated_glsl <path>     outputs the generated GLSL target code to a file\n"
        << "  --info                      limit log output to level 'info' and higher\n"
        << "  --warn                      limit log output to level 'warning' and higher\n"
        << "  --error                     limit log output to level 'error'\n"
        << std::endl;

    exit(EXIT_FAILURE);
}

void parse_command_line(int argc, char* argv[], Options& options)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg(argv[i]);
        if (arg[0] == '-')
        {
            if (arg == "--nogui")
                options.no_window = true;

            else if (arg == "--mtlx_path" && i < argc - 1)
            {
                std::string path(argv[++i]);
                options.mtlx_paths.push_back(mi::examples::io::normalize(path));
            }
            else if (arg == "--mtlx_library" && i < argc - 1)
            {
                std::string path(argv[++i]);
                options.mtlx_libraries.push_back(mi::examples::io::normalize(path));
            }
            else if (arg == "--mtlx_to_mdl" && i < argc - 1)
            {
                std::string version(argv[++i]);
                if (version != "1.6")
                    options.mdl_target_version = MaterialX::GenMdlOptions::MdlVersion::MDL_1_6;
                else if (version != "1.7")
                    options.mdl_target_version = MaterialX::GenMdlOptions::MdlVersion::MDL_1_7;
                else if (version != "1.8")
                    options.mdl_target_version = MaterialX::GenMdlOptions::MdlVersion::MDL_1_8;
                else if (version != "1.9")
                    options.mdl_target_version = MaterialX::GenMdlOptions::MdlVersion::MDL_1_9;
                else
                    options.mdl_target_version = MaterialX::GenMdlOptions::MdlVersion::MDL_LATEST;
            }
            else if (arg == "--mat" && i < argc - 1)
            {
                options.material_name = argv[++i];
            }
            else if (arg == "--res" && i < argc - 2)
            {
                options.res_x = std::max(atoi(argv[++i]), 1);
                options.res_y = std::max(atoi(argv[++i]), 1);
            }
            else if (arg == "--numimg" && i < argc - 1)
                options.num_images = std::max(atoi(argv[++i]), 2);
            else if (arg == "--device" && i < argc - 1)
                options.device_index = std::max(atoi(argv[++i]), -1);
            else if (arg == "-o" && i < argc - 1)
                options.output_file = argv[++i];
            else if (arg == "--spp" && i < argc - 1)
                options.samples_per_pixel = std::atoi(argv[++i]);
            else if (arg == "--spi" && i < argc - 1)
                options.samples_per_iteration = std::atoi(argv[++i]);
            else if (arg == "--max_path_length" && i < argc - 1)
                options.max_path_length = std::atoi(argv[++i]);
            else if ((arg == "-f" || arg == "--fov") && i < argc - 1)
                options.cam_fov = static_cast<float>(std::atof(argv[++i]));
            else if (arg == "--camera" && i < argc - 6)
            {
                options.cam_pos.x = static_cast<float>(std::atof(argv[++i]));
                options.cam_pos.y = static_cast<float>(std::atof(argv[++i]));
                options.cam_pos.z = static_cast<float>(std::atof(argv[++i]));
                options.cam_lookat.x = static_cast<float>(std::atof(argv[++i]));
                options.cam_lookat.y = static_cast<float>(std::atof(argv[++i]));
                options.cam_lookat.z = static_cast<float>(std::atof(argv[++i]));
            }
            else if ((arg == "-l" || arg == "--light") && i < argc - 6)
            {
                options.light_pos.x = static_cast<float>(std::atof(argv[++i]));
                options.light_pos.y = static_cast<float>(std::atof(argv[++i]));
                options.light_pos.z = static_cast<float>(std::atof(argv[++i]));
                options.light_intensity.x = static_cast<float>(std::atof(argv[++i]));
                options.light_intensity.y = static_cast<float>(std::atof(argv[++i]));
                options.light_intensity.z = static_cast<float>(std::atof(argv[++i]));
                options.light_enabled = true;
            }
            else if (arg == "--hdr" && i < argc - 1)
                options.hdr_file = argv[++i];
            else if (arg == "--hdr_intensity" && i < argc - 1)
                options.hdr_intensity = static_cast<float>(std::atof(argv[++i]));
            else if (arg == "--hdr_rotate" && i < argc - 1)
                options.hdr_rotate = std::max(0.0f, std::min(static_cast<float>(atof(argv[++i])), 360.0f)) / 360.0f;
            else if (arg == "--background" && i < argc - 3)
            {
                options.background_color.x = static_cast<float>(std::atof(argv[++i]));
                options.background_color.y = static_cast<float>(std::atof(argv[++i]));
                options.background_color.z = static_cast<float>(std::atof(argv[++i]));
                options.background_color_enabled = true;
            }
            else if ((arg == "-p" || arg == "--mdl_path") && i < argc - 1)
                options.mdl_search_paths.push_back(argv[++i]);
            else if (arg == "--enable_ro_segment")
                options.enable_ro_segment = true;
            else if (arg == "--disable_ssbo")
                options.disable_ssbo = true;
            else if (arg == "--max_const_data")
                options.max_const_data = uint32_t(std::atoi(argv[++i]));
            else if (arg == "--vkdebug")
                options.enable_validation_layers = true;
            else if (arg == "--no_shader_opt")
                options.enable_shader_optimization = false;

            else if (arg == "--materialxtest_mode")
                options.materialxtest_mode = true;
            else if ((arg == "-g" || arg == "--generated") && i < argc - 1)
                options.dump_mdl = (argv[++i]);
            else if (arg == "--generated_glsl" && i < argc - 1)
                options.dump_glsl = (argv[++i]);

            else if (arg == "--info")
                options.log_level = mi::examples::log::Level::Info;
            else if (arg == "--warn")
                options.log_level = mi::examples::log::Level::Warning;
            else if (arg == "--error")
                options.log_level = mi::examples::log::Level::Error;
            else
            {
                if (arg != "-h" && arg != "--help")
                    std::cout << "Unknown option: \"" << arg << "\"" << std::endl;

                print_usage(argv[0]);
            }
        }
    }
}


//------------------------------------------------------------------------------
// Main function
//------------------------------------------------------------------------------

namespace mi
{
namespace examples
{

namespace mdl
{
    // required for loading and unloading the SDK
    #ifdef MI_PLATFORM_WINDOWS
        HMODULE g_dso_handle = 0;
    #else
        void* g_dso_handle = 0;
    #endif
} // namespace mdl

namespace log
{
    Level s_Level;
}

} // namespace examples
} // namespace mi

int MAIN_UTF8(int argc, char* argv[])
{
    Options options;
    parse_command_line(argc, argv, options);
    mi::examples::log::s_Level = options.log_level;

    // Access the MDL SDK
    mi::base::Handle<mi::neuraylib::INeuray> neuray(
        mi::examples::mdl::load_and_get_ineuray());
    if (!neuray.is_valid_interface())
        exit_failure("Failed to load the SDK.");

    // install a custom logger to have control over the output
    mi::base::Handle<mi::neuraylib::ILogging_configuration> logging_configuration(
        neuray->get_api_component<mi::neuraylib::ILogging_configuration>());
    std::unique_ptr<mi::examples::log::ExampleLogger> logger = std::make_unique<mi::examples::log::ExampleLogger>();
    logging_configuration->set_receiving_logger(logger.get());

    // Configure the MDL SDK
    mi::base::Handle<mi::neuraylib::IMdl_configuration> mdl_config(
        neuray->get_api_component<mi::neuraylib::IMdl_configuration>());

    // add the search paths for MDL module and resource resolution outside of MDL modules
    // also add the `mdl` folder next the binary as search path
    options.mdl_search_paths.push_back(mi::examples::io::get_executable_folder() + "/mdl");
    for (size_t i = 0, n = options.mdl_search_paths.size(); i < n; ++i)
    {
        mdl_config->add_mdl_path(options.mdl_search_paths[i].c_str());
        mdl_config->add_resource_path(options.mdl_search_paths[i].c_str());
    }

    // load image plugins for texture loading
    if (mi::examples::mdl::load_plugin(neuray.get(), "nv_openimageio" MI_BASE_DLL_FILE_EXT) != 0)
    {
        exit_failure("Failed to load the nv_openimageio plugin.");
    }
    if (mi::examples::mdl::load_plugin(neuray.get(), "dds" MI_BASE_DLL_FILE_EXT) != 0)
    {
        exit_failure("Failed to load the dds plugin.");
    }

    // Start the MDL SDK
    mi::Sint32 result = neuray->start();
    if (result != 0)
        exit_failure("Failed to initialize the SDK. Result code: %d", result);

    {
        // Create a transaction
        mi::base::Handle<mi::neuraylib::IDatabase> database(
            neuray->get_api_component<mi::neuraylib::IDatabase>());
        mi::base::Handle<mi::neuraylib::IScope> scope(database->get_global_scope());
        mi::base::Handle<mi::neuraylib::ITransaction> transaction(scope->create_transaction());

        // Access needed API components
        mi::base::Handle<mi::neuraylib::IMdl_factory> mdl_factory(
            neuray->get_api_component<mi::neuraylib::IMdl_factory>());

        mi::base::Handle<mi::neuraylib::IMdl_impexp_api> mdl_impexp_api(
            neuray->get_api_component<mi::neuraylib::IMdl_impexp_api>());

        mi::base::Handle<mi::neuraylib::IMdl_backend_api> mdl_backend_api(
            neuray->get_api_component<mi::neuraylib::IMdl_backend_api>());

        mi::base::Handle<mi::neuraylib::IImage_api> image_api(
            neuray->get_api_component<mi::neuraylib::IImage_api>());

        mi::base::Handle<mi::neuraylib::IMdl_execution_context> context(
            mdl_factory->create_execution_context());

        mi::base::Handle<mi::neuraylib::IMdl_configuration> mdl_config(
            neuray->get_api_component<mi::neuraylib::IMdl_configuration>());

        {
            // corresponds to module path within an MDL search path 
            std::string qualified_module_name;
            // the function name, i.e., within the given module
            std::string material_simple_name;

            // check if the selected material is a MaterialX material
            if (std::strstr(options.material_name.c_str(), ".mtlx") != nullptr)
            {
                // Note, this section is all that is needed to add MaterialX support to an MDL-based renderer
                MdlGenerator gen;
                gen.SetMdlVersion(options.mdl_target_version);
                gen.SetFileTextureVerticalFlip(options.materialxtest_mode);
                for (const auto& p : options.mtlx_paths)
                    gen.AddMaterialxSearchPath(p);
                for (const auto& l : options.mtlx_libraries)
                    gen.AddMaterialxLibrary(l);

                // When the MaterialX filename has a query parameter called 'name', use it as element selector
                const auto query_arguments = mi::examples::io::parse_url_query(mi::examples::io::get_url_query(options.material_name));
                const auto& nameIt = query_arguments.find("name");
                gen.SetSource(
                    mi::examples::io::drop_url_query(options.material_name),
                    nameIt == query_arguments.end() ? "" : nameIt->second);

                MdlGenerator::Result result;
                if (gen.Generate(mdl_config.get(), result))
                {
                    qualified_module_name = "::app::generated";
                    material_simple_name = result.generatedMdlName;
                    if (mdl_impexp_api->load_module_from_string(
                            transaction.get(), qualified_module_name.c_str(), result.generatedMdlCode.c_str(), context.get()) < 0)
                    {
                        mi::examples::log::error("Failed to load the generated MDL code from: " + options.material_name);
                        mi::examples::log::context_messages(context.get());
                        qualified_module_name.clear();
                        material_simple_name.clear();
                    }
                    else if (!options.dump_mdl.empty())
                    {
                        std::string dump_mdl_path = options.dump_mdl;
                        if (!mi::examples::io::is_absolute_path(dump_mdl_path))
                            dump_mdl_path = mi::examples::io::get_working_directory() + "/" + dump_mdl_path;
                        mi::examples::log::info("Dumping generated MDL module to: \"%s\"", dump_mdl_path.c_str());
                        std::ofstream file_stream(dump_mdl_path);
                        file_stream.write(result.generatedMdlCode.c_str(), result.generatedMdlCode.length());
                        file_stream.close();
                    }
                }
            }
            // fall back to load a regular MDL module
            else
            {
                // Split material name into module and simple material name
                mi::examples::mdl::parse_cmd_argument_material_name(
                    options.material_name, qualified_module_name, material_simple_name);

                // Load the module from the MDL search path
                if (mdl_impexp_api->load_module(transaction.get(), qualified_module_name.c_str(), context.get()) < 0)
                {
                    mi::examples::log::error("Failed to load material: " + options.material_name);
                    mi::examples::log::context_messages(context.get());
                    qualified_module_name.clear();
                    material_simple_name.clear();
                }
            }

            // Load a default material from string
            if (qualified_module_name.empty() || material_simple_name.empty())
            {
                qualified_module_name = "::app";
                material_simple_name = "not_available";
                static const char* module_src =
                    "mdl 1.6;\n"
                    "import ::df::*;\n"
                    "export material not_available() = material(\n"
                    "    surface: material_surface(\n"
                    "        df::diffuse_reflection_bsdf(\n"
                    "            tint: color(0.8, 0.0, 0.8)\n"
                    "        )\n"
                    "    )"
                    ");";

                mdl_impexp_api->load_module_from_string(
                    transaction.get(), qualified_module_name.c_str(), module_src, context.get());
            }

            // Load and compile material, and generate GLSL code
            mi::base::Handle<mi::neuraylib::IFunction_call> material_instance(
                create_material_instance(mdl_factory.get(), transaction.get(),
                    qualified_module_name, material_simple_name));

            mi::base::Handle<mi::neuraylib::ICompiled_material> compiled_material(
                compile_material_instance(mdl_factory.get(), transaction.get(),
                    material_instance.get(), context.get()));

            mi::base::Handle<const mi::neuraylib::ITarget_code> target_code(
                generate_glsl_code(compiled_material.get(), mdl_backend_api.get(),
                    transaction.get(), context.get(), options));

            if (!options.dump_glsl.empty())
            {
                std::string dump_glsl_path = options.dump_glsl;
                if (!mi::examples::io::is_absolute_path(dump_glsl_path))
                    dump_glsl_path = mi::examples::io::get_working_directory() + "/" + dump_glsl_path;
                mi::examples::log::info("Dumping GLSL target code to: \"%s\"", dump_glsl_path.c_str());
                std::ofstream file_stream(dump_glsl_path);
                file_stream.write(target_code->get_code(), target_code->get_code_size());
                file_stream.close();
            }

            // Start application
            mi::examples::vk::Vulkan_example_app::Config app_config;
            app_config.window_title = "MDL Standalone Render for MaterialX";
            app_config.image_width = options.res_x;
            app_config.image_height = options.res_y;
            app_config.image_count = options.num_images;
            app_config.headless = options.no_window;
            app_config.iteration_count = options.samples_per_pixel / options.samples_per_iteration;
            app_config.device_index = options.device_index;
            app_config.enable_validation_layers = options.enable_validation_layers;
            app_config.enable_descriptor_indexing = true;
            app_config.flip_texcoord_v = options.materialxtest_mode;

            Df_vulkan_app app(transaction, mdl_impexp_api, image_api, target_code, compiled_material,options);
            app.run(app_config);
        }

        transaction->commit();
    }

    // remove custom logger
    logging_configuration->set_receiving_logger(nullptr);
    logger.reset();

    // Shut down the MDL SDK
    if (neuray->shutdown() != 0)
        exit_failure("Failed to shutdown the SDK.");

    // Unload the MDL SDK
    neuray = nullptr;
    if (!mi::examples::mdl::unload())
        exit_failure("Failed to unload the SDK.");

    exit_success();
}

// Convert command line arguments to UTF8 on Windows
COMMANDLINE_TO_UTF8
