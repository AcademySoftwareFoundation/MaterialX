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

#include "vulkan_base_application.h"

#define VOLK_IMPLEMENTATION
#include <volk.h>

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <spirv-tools/optimizer.hpp>

#include <unordered_set>
#include <algorithm>
#include <numeric>
#define _USE_MATH_DEFINES
#include <math.h>

namespace
{

// Similar to std::remove_copy_if, but returns an iterator that can be used in erase
template <typename Src_iterator, typename Dst_iterator, typename Pred>
Src_iterator splice_if(Src_iterator first, Src_iterator last, Dst_iterator out, Pred p)
{
    Src_iterator result = first;
    for (; first != last; ++first)
    {
        if (p(*first))
            *result++ = *first;
        else
            *out++ = *first;
    }
    return result;
}

std::string decode_driver_version(VkPhysicalDeviceProperties physical_device_props)
{
    std::ostringstream ss;
    if (physical_device_props.vendorID == 4318) // NVIDIA
    {
        ss << ((physical_device_props.driverVersion >> 22) & 0x3ff) << "."
            << ((physical_device_props.driverVersion >> 14) & 0x0ff);
    }
#ifdef MI_PLATFORM_WINDOWS
    else if (physical_device_props.vendorID == 0x8086)
    {
        ss << (physical_device_props.driverVersion >> 14) << "."
            << (physical_device_props.driverVersion & 0x3fff);
    }
#endif
    else
    {
        ss << (physical_device_props.driverVersion >> 22) << "."
            << ((physical_device_props.driverVersion >> 12) & 0x3ff) << "."
            << (physical_device_props.driverVersion & 0xfff);
    }

    return ss.str();
}

} // namespace

namespace mi::examples::vk
{

void Glsl_compiler::add_defines(const std::vector<std::string>& defines)
{
    for (std::string define : defines)
    {
        // Replace first = with a space.
        // E.g. MY_DEFINE=1 -> MY_DEFINE 1
        const size_t equal_pos = define.find_first_of("=");
        if (equal_pos != define.npos)
            define[equal_pos] = ' ';
        
        m_shader_preamble += "#define ";
        m_shader_preamble += define;
        m_shader_preamble += '\n';
    }
}

// Parses the given shader source and adds it to the shader program
// which can be compiled to SPIR-V by link_program.
void Glsl_compiler::add_shader(std::string_view source)
{
    std::unique_ptr<glslang::TShader>& shader =
        m_shaders.emplace_back(std::make_unique<glslang::TShader>(m_shader_type));

    const char* sources[] = { source.data() };
    const int lengths[] = { static_cast<int>(source.length()) };
    shader->setStringsWithLengths(sources, lengths, 1);
    shader->setEntryPoint(m_entry_point.c_str());
    shader->setEnvClient(
        glslang::EShClientVulkan,
        glslang::EshTargetClientVersion::EShTargetVulkan_1_0);
    shader->setEnvTarget(
        glslang::EShTargetLanguage::EShTargetSpv,
        glslang::EShTargetLanguageVersion::EShTargetSpv_1_0);
    shader->setEnvInput(
        glslang::EShSourceGlsl, m_shader_type,
        glslang::EShClient::EShClientVulkan, 100);
    shader->setPreamble(m_shader_preamble.c_str());

    bool success = shader->parse(
        /*builtInResource=*/ GetDefaultResources(),
        /*defaultVersion=*/ 100, // Will be overridden by #version in shader source
        /*forwardCompatible=*/ true,
        /*messages =*/ s_messages,
        /*includer =*/ m_file_includer);

    if (!success)
    {
        std::cerr << "Compilation for shader "
            << (m_shaders.size() - 1) << " failed:\n"
            << shader->getInfoLog()
            << shader->getInfoDebugLog();
        terminate();
    }
}

// Links all previously added shaders and compiles the linked program to SPIR-V.
std::vector<unsigned int> Glsl_compiler::link_program(bool optimize)
{
    glslang::TProgram program;
    for (std::unique_ptr<glslang::TShader>& shader : m_shaders)
        program.addShader(shader.get());

    if (!program.link(s_messages))
    {
        std::cerr << "Shader program linking failed:\n"
            << program.getInfoLog()
            << program.getInfoDebugLog();
        terminate();
    }

    std::vector<unsigned int> spirv;
    glslang::GlslangToSpv(*program.getIntermediate(m_shader_type), spirv);

    if (optimize)
    {
        spvtools::OptimizerOptions opt_options;
        opt_options.set_run_validator(false);

        std::vector<uint32_t> spirv_opt;
        spvtools::Optimizer optimizer(SPV_ENV_VULKAN_1_0);
        optimizer.RegisterPerformancePasses();
        optimizer.Run(spirv.data(), spirv.size(), &spirv_opt, opt_options);
        return spirv_opt;
    }

    return spirv;
}

glslang::TShader::Includer::IncludeResult* Glsl_compiler::Simple_file_includer::includeSystem(
    const char* /*header_name*/, const char* /*includer_name*/, size_t /*inclusion_depth*/)
{
    // Not supported
    return nullptr;
}

glslang::TShader::Includer::IncludeResult* Glsl_compiler::Simple_file_includer::includeLocal(
    const char* header_name, const char* includer_name, size_t inclusion_depth)
{
    std::string filename(header_name);

    // Drop strict relative marker
    if (mi::examples::strings::starts_with(filename, "./"))
        filename = filename.substr(2);

    // Make path absolute if not already
    if (!mi::examples::io::is_absolute_path(filename))
        filename = mi::examples::io::get_executable_folder() + "/" + filename;

    std::ifstream file_stream(filename, std::ios_base::binary | std::ios_base::ate);
    if (!file_stream.is_open())
    {
        std::cerr << "Shader include file \"" << filename <<  "\" not found.";
        terminate();
    }

    size_t length = file_stream.tellg();
    char* content = new char[length];
    file_stream.seekg(0, std::ios::beg);
    file_stream.read(content, length);
    return new IncludeResult(header_name, content, length, content);
}

void Glsl_compiler::Simple_file_includer::releaseInclude(IncludeResult* include_result)
{
    if (include_result)
    {
        delete[] static_cast<char*>(include_result->userData);
        delete include_result;
    }
}


void Vulkan_example_app::run(const Config& config)
{
    init(config);

    double last_frame_time = glfwGetTime();

    if (!m_config.headless)
    {
        uint32_t frame_index = 0;

        while (!glfwWindowShouldClose(m_window))
        {
            glfwPollEvents();

            // Wait for the GPU to finish the current command buffer
            VK_CHECK(vkWaitForFences(
                m_device, 1, &m_frame_inflight_fences[frame_index], true, UINT64_MAX));
            VK_CHECK(vkResetFences(
                m_device, 1, &m_frame_inflight_fences[frame_index]));

            uint32_t image_index;
            VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
                m_image_available_semaphores[frame_index], nullptr, &image_index);
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                recreate_swapchain_or_framebuffer_image();
                continue;
            }
            else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
                VK_CHECK(result); // This will output the error

            render_loop_iteration(frame_index, image_index, last_frame_time);

            if (m_screenshot_requested)
            {
                m_screenshot_requested = false;
                save_screenshot(image_index, "screenshot.png");
            }

            // Present the next image
            VkPresentInfoKHR present_info = {};
            present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            present_info.waitSemaphoreCount = 1;
            present_info.pWaitSemaphores = &m_render_finished_semaphores[frame_index];
            present_info.swapchainCount = 1;
            present_info.pSwapchains = &m_swapchain;
            present_info.pImageIndices = &image_index;

            result = vkQueuePresentKHR(m_present_queue, &present_info);
            if (result == VK_ERROR_OUT_OF_DATE_KHR
                || result == VK_SUBOPTIMAL_KHR
                || m_framebuffer_resized)
            {
                recreate_swapchain_or_framebuffer_image();
                m_framebuffer_resized = false;
                continue;
            }
            else if (result != VK_SUCCESS)
                VK_CHECK(result); // This will output the error

            frame_index = (frame_index + 1) % m_image_count;
        }
    }
    else
    {
        for (uint32_t i = 0; i < m_config.iteration_count; i++)
        {
            uint32_t image_index = i % m_image_count;

            // Wait for the GPU to finish the current command buffer
            VK_CHECK(vkWaitForFences(
                m_device, 1, &m_frame_inflight_fences[image_index], true, UINT64_MAX));
            VK_CHECK(vkResetFences(
                m_device, 1, &m_frame_inflight_fences[image_index]));

            render_loop_iteration(image_index, image_index, last_frame_time);
        }
    }

    // Wait for all resources to be unused
    VK_CHECK(vkDeviceWaitIdle(m_device));

    cleanup();
}

void Vulkan_example_app::save_screenshot(uint32_t image_index, const char* filename) const
{
    uint32_t bpp = mi::examples::vk::get_image_format_bpp(m_image_format);
    VkImageLayout image_layout = m_config.headless
        ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    std::vector<uint8_t> pixels = mi::examples::vk::copy_image_to_buffer(
        m_device, m_physical_device, m_command_pool, m_graphics_queue,
        m_swapchain_images[image_index], m_image_width, m_image_height,
        bpp, image_layout, true);

    // Convert from BGRA to RGBA if needed.
    if (m_image_format == VK_FORMAT_B8G8R8A8_UNORM
        || m_image_format == VK_FORMAT_B8G8R8A8_SRGB)
    {
        for (uint32_t i = 0; i < m_image_width * m_image_height; i++)
            std::swap(pixels[i * 4 + 0], pixels[i * 4 + 2]);
    }

    mi::base::Handle<mi::neuraylib::ICanvas> canvas(
        m_image_api->create_canvas("Rgba", m_image_width, m_image_height));
    mi::base::Handle<mi::neuraylib::ITile> tile(canvas->get_tile());

    std::memcpy(tile->get_data(), pixels.data(), pixels.size());
    m_mdl_impexp_api->export_canvas(filename, canvas.get());
}

void Vulkan_example_app::set_vsync_enabled(bool vsync_enabled)
{
    if (vsync_enabled != m_config.vsync)
    {
        m_config.vsync = vsync_enabled;
        m_framebuffer_resized = true;
    }
}

void Vulkan_example_app::init(const Config& config)
{
    m_config = config;

    VK_CHECK(volkInitialize());

    if (!m_config.headless)
        init_window();

    // Gather extensions and validation layers
    std::vector<const char*> instance_extensions;
    std::vector<const char*> device_extensions;
    std::vector<const char*> validation_layers;

    if (!m_config.headless)
    {
        uint32_t glfw_extensions_count;
        const char** glfw_extensions
            = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);

        instance_extensions.insert(instance_extensions.end(),
            glfw_extensions, glfw_extensions + glfw_extensions_count);

        device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    if (config.enable_validation_layers)
    {
        instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        validation_layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    if (config.enable_descriptor_indexing)
    {
        instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        device_extensions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
        device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    }

    // Determine if all validation layers are supported and present
    if (!validation_layers.empty())
    {
        uint32_t layer_count;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));

        std::vector<VkLayerProperties> available_layers(layer_count);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()));

        // Copy all unsupported layers into a separate vector and remove them from the original
        std::vector<const char*> unsupported_layers;
        auto it = splice_if(validation_layers.begin(), validation_layers.end(),
            std::back_inserter(unsupported_layers),
            [&available_layers](const char* requested_layer)
        {
            for (const VkLayerProperties& available_layer : available_layers)
            {
                if (std::strcmp(requested_layer, available_layer.layerName) == 0)
                    return true;
            }
            return false;
        });
        validation_layers.erase(it, validation_layers.end());

        if (!unsupported_layers.empty())
        {
            std::cerr << "Not all requested instance layers are available. Could not find:\n";
            for (const char* layer : unsupported_layers)
                std::cerr << "  " << layer << "\n";
            std::cerr << "The environment variable VK_LAYER_PATH might need to be set.\n";
        }
    }

    // Initialize Vulkan interfaces
    init_instance(instance_extensions, validation_layers);

    if (!m_config.headless)
        VK_CHECK(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface));

    pick_physical_device(device_extensions);
    init_device(device_extensions, validation_layers);

    vkGetDeviceQueue(m_device, m_graphics_queue_family_index, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, m_present_queue_family_index, 0, &m_present_queue);

    if (config.headless)
        init_swapchain_for_headless();
    else
        init_swapchain_for_window();
    init_depth_stencil_buffer();
    init_render_pass();
    init_framebuffers();
    init_command_pool_and_buffers();
    init_synchronization_objects();

    // Init application resources
    init_resources();
}

void Vulkan_example_app::cleanup()
{
    // Cleanup application resources
    cleanup_resources();

    for (VkFramebuffer framebuffer : m_framebuffers)
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    vkDestroyRenderPass(m_device, m_main_render_pass, nullptr);

    vkDestroyCommandPool(m_device, m_command_pool, nullptr);

    vkDestroyImageView(m_device, m_depth_stencil_image_view, nullptr);
    vkDestroyImage(m_device, m_depth_stencil_image, nullptr);
    vkFreeMemory(m_device, m_depth_stencil_device_memory, nullptr);

    for (VkImageView image_view : m_swapchain_image_views)
        vkDestroyImageView(m_device, image_view, nullptr);
    if (m_config.headless)
    {
        for (VkImage image : m_swapchain_images)
            vkDestroyImage(m_device, image, nullptr);
        for (VkDeviceMemory device_memory : m_swapchain_device_memories)
            vkFreeMemory(m_device, device_memory, nullptr);
    }
    else
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
 
    for (uint32_t i = 0; i < m_image_count; i++)
    {
        vkDestroyFence(m_device, m_frame_inflight_fences[i], nullptr);
        vkDestroySemaphore(m_device, m_image_available_semaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_render_finished_semaphores[i], nullptr);
    }

    vkDestroyDevice(m_device, nullptr);
    if (!m_config.headless)
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    if (m_debug_messenger)
        vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Vulkan_example_app::init_window()
{
    glfwSetErrorCallback(&glfw_error_callback);

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW.\n";
        terminate();
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    m_window = glfwCreateWindow(
        static_cast<int>(m_config.image_width),
        static_cast<int>(m_config.image_height),
        m_config.window_title.c_str(),
        nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "Failed to create GLFW window.\n";
        terminate();
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, &internal_key_callback);
    glfwSetCursorPosCallback(m_window, &internal_mouse_move_callback);
    glfwSetMouseButtonCallback(m_window, &internal_mouse_button_callback);
    glfwSetScrollCallback(m_window, &internal_mouse_scroll_callback);
    glfwSetFramebufferSizeCallback(m_window, &internal_resize_callback);
}

void Vulkan_example_app::init_instance(
    const std::vector<const char*>& instance_extensions,
    const std::vector<const char*>& validation_layers)
{
    // Create Vulkan instance
    VkApplicationInfo application_info = {};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "MDL SDK Vulkan Example";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "MDL SDK";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;

    VkDebugUtilsMessengerCreateInfoEXT debug_utils_create_info = {};
    debug_utils_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_utils_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_utils_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_utils_create_info.pfnUserCallback = &debug_messenger_callback;

    if (!validation_layers.empty())
    {
        // Add debug callback extension
        instance_create_info.pNext = &debug_utils_create_info;
        instance_create_info.ppEnabledLayerNames = validation_layers.data();
        instance_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    }

    instance_create_info.ppEnabledExtensionNames = instance_extensions.data();
    instance_create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());

    VK_CHECK(vkCreateInstance(&instance_create_info, nullptr, &m_instance));

    volkLoadInstanceOnly(m_instance);

    if (!validation_layers.empty())
    {
        VK_CHECK(vkCreateDebugUtilsMessengerEXT(
            m_instance, &debug_utils_create_info, nullptr, &m_debug_messenger));
    }
}

void Vulkan_example_app::pick_physical_device(
    const std::vector<const char*>& device_extensions)
{
    uint32_t physical_device_count;
    VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &physical_device_count, nullptr));

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    VK_CHECK(vkEnumeratePhysicalDevices(
        m_instance, &physical_device_count, physical_devices.data()));

    // Find all physical device that support the requested extensions and queue families
    struct Supported_gpu
    {
        VkPhysicalDevice physical_device;
        uint32_t graphics_queue_family_index;
        uint32_t present_queue_family_index;
    };
    std::vector<Supported_gpu> supported_gpus;

    for (const VkPhysicalDevice& physical_device : physical_devices)
    {
        // Select queue families
        uint32_t queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_family_props(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(
            physical_device, &queue_family_count, queue_family_props.data());

        uint32_t graphics_queue_family_index = ~0u;
        uint32_t present_queue_family_index = ~0u;

        for (uint32_t i = 0; i < queue_family_count; i++)
        {
            if (queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphics_queue_family_index = i;

            VkBool32 has_presentation_support;
            if (!m_config.headless)
            {
                VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(
                    physical_device, i, m_surface, &has_presentation_support));
            }
            else
            {
                // For headless mode just treat every graphics queue as a
                // valid "present" queue to make the setup a lot simpler.
                has_presentation_support = true;
            }

            if (has_presentation_support)
                present_queue_family_index = i;

            if (graphics_queue_family_index != ~0u && present_queue_family_index != ~0u)
                break;
        }

        if (graphics_queue_family_index == ~0u || present_queue_family_index == ~0u)
            continue;

        // The device must support all requested extensions
        if (!check_device_extensions_support(physical_device, device_extensions))
            continue;

        supported_gpus.push_back(
            { physical_device, graphics_queue_family_index, present_queue_family_index });
    }

    if (supported_gpus.empty())
    {
        std::cerr << "No suitable physical device found.\n";
        terminate();
    }

    // Pick the first discrete physical device
    int32_t device_index = -1;

    if (supported_gpus.size() > 1)
    {
        std::string gpu_options = "Multiple GPUs detected, run with option '--device <num>' to select specific one.";

        for (size_t i = 0; i < supported_gpus.size(); i++)
        {
            VkPhysicalDeviceProperties physical_device_props;
            vkGetPhysicalDeviceProperties(
                supported_gpus[i].physical_device, &physical_device_props);

            if (device_index == -1 && physical_device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                device_index = static_cast<int32_t>(i);

            gpu_options += "  [" + std::to_string(i) + "] " + physical_device_props.deviceName;
            switch (physical_device_props.deviceType)
            {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                gpu_options += " [integrated]";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                gpu_options += " [discrete]";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                gpu_options += " [virtual]";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                gpu_options += " [cpu]";
                break;
            default:
                gpu_options += " [unknown]";
                break;
            }
            gpu_options += "\n";
        }
        mi::examples::log::info(gpu_options);
    }
    
    if (m_config.device_index >= 0)
    {
        if (m_config.device_index < supported_gpus.size())
            device_index = m_config.device_index;
        else
            mi::examples::log::info(
                "Requested device index %d is out of bounds. Falling back to the first discrete GPU.",
                m_config.device_index);
    }
    else if (device_index == -1)
    {
        // Either only one supported GPU was found or in the case of multiple GPUs,
        // no discrete one was found. In either case the first one is picked.
        device_index = 0;
    }

    m_physical_device = supported_gpus[device_index].physical_device;
    m_graphics_queue_family_index = supported_gpus[device_index].graphics_queue_family_index;
    m_present_queue_family_index = supported_gpus[device_index].present_queue_family_index;

    VkPhysicalDeviceProperties physical_device_props;
    vkGetPhysicalDeviceProperties(
        m_physical_device, &physical_device_props);
    mi::examples::log::info("Chosen GPU: %  (driver version %s)",
                            physical_device_props.deviceName, decode_driver_version(physical_device_props).c_str());
}

void Vulkan_example_app::init_device(
    const std::vector<const char*>& device_extensions,
    const std::vector<const char*>& validation_layers)
{
    std::unordered_set<uint32_t> unique_queue_families = {
        m_graphics_queue_family_index,
        m_present_queue_family_index
    };
    std::vector< VkDeviceQueueCreateInfo> queue_create_infos;
    for (uint32_t queue_family_index : unique_queue_families)
    {
        float queue_priority = 1.0f;
        VkDeviceQueueCreateInfo queue_create_info = {};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family_index;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    if (!validation_layers.empty())
    {
        device_create_info.ppEnabledLayerNames = validation_layers.data();
        device_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    }
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());

    VkPhysicalDeviceDescriptorIndexingFeaturesEXT physicalDeviceDescriptorIndexingFeatures = {};
    if (m_config.enable_descriptor_indexing)
    {
        physicalDeviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
        physicalDeviceDescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        physicalDeviceDescriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
        physicalDeviceDescriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
        device_create_info.pNext = &physicalDeviceDescriptorIndexingFeatures;
    }

    VK_CHECK(vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_device));

    volkLoadDevice(m_device);
}

void Vulkan_example_app::init_swapchain_for_window()
{
    VkSurfaceCapabilitiesKHR surface_caps;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        m_physical_device, m_surface, &surface_caps));

    // Find correct swapchain image size
    m_image_width = surface_caps.currentExtent.width;
    m_image_height = surface_caps.currentExtent.height;
    if (m_image_width == 0xffffffff|| m_image_height == 0xffffffff)
    {
        // The extents in surface_caps are invalid, take the windows
        // framebuffer size and make them fit the surface caps.
        int framebuffer_width;
        int framebuffer_height;
        glfwGetFramebufferSize(m_window, &framebuffer_width, &framebuffer_height);

        m_image_width = std::clamp(static_cast<uint32_t>(framebuffer_width),
            surface_caps.minImageExtent.width, surface_caps.maxImageExtent.width);
        m_image_height = std::clamp(static_cast<uint32_t>(framebuffer_height),
            surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height);
    }

    // Find a suitable swapchain image format
    uint32_t surface_format_count;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_physical_device, m_surface, &surface_format_count, nullptr));

    std::vector<VkSurfaceFormatKHR> available_surface_formats(surface_format_count);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_physical_device, m_surface, &surface_format_count, available_surface_formats.data()));

    m_image_format = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR image_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    for (VkFormat preferred_format : m_config.preferred_image_formats)
    {
        for (const VkSurfaceFormatKHR& surface_format : available_surface_formats)
        {
            if (surface_format.format == preferred_format)
            {
                m_image_format = surface_format.format;
                image_color_space = surface_format.colorSpace;
            }
        }
    }

    if (m_image_format == VK_FORMAT_UNDEFINED)
    {
        m_image_format = available_surface_formats[0].format;
        image_color_space = available_surface_formats[0].colorSpace;

        std::cerr << "None of the preferred image formats {";
        for (size_t i = 0; i < m_config.preferred_image_formats.size(); i++)
        {
            if (i > 0)
                std::cerr << ", ";
            std::cerr << vkformat_to_str(m_config.preferred_image_formats[i]);
        }
        std::cerr << "} are supported. Choosing the first supported format (";
        std::cerr << vkformat_to_str(m_image_format) << ") instead.\n";
    }

    // Find a compatible alpha channel composite mode
    VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    const VkCompositeAlphaFlagBitsKHR composite_alpha_flags[] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
    };
    for (VkCompositeAlphaFlagBitsKHR flag : composite_alpha_flags)
    {
        if (surface_caps.supportedCompositeAlpha & flag)
        {
            composite_alpha = flag;
            break;
        }
    }

    // Find a present mode
    uint32_t present_mode_count;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_physical_device, m_surface, &present_mode_count, nullptr));

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_physical_device, m_surface, &present_mode_count, present_modes.data()));

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;// Support is guaranteed by spec
    if (!m_config.vsync)
    {
        for (VkPresentModeKHR mode : present_modes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                present_mode = mode;
                break;
            }
        }
    }

    // Requested image count might not be supported, so clamp it
    m_image_count = std::clamp(
        m_config.image_count, surface_caps.minImageCount, surface_caps.maxImageCount);

    // Create swapchain
    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = m_surface;
    swapchain_create_info.minImageCount = m_image_count;
    swapchain_create_info.imageFormat = m_image_format;
    swapchain_create_info.imageColorSpace = image_color_space;
    swapchain_create_info.imageExtent.width = m_image_width;
    swapchain_create_info.imageExtent.height = m_image_height;
    swapchain_create_info.imageArrayLayers = 1; // No stereo/multiview
    swapchain_create_info.imageUsage
        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    const uint32_t queue_family_indices[] = {
        m_graphics_queue_family_index,
        m_present_queue_family_index
    };
    if (queue_family_indices[0] == queue_family_indices[1])
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    else
    {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = std::size(queue_family_indices);
        swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    }
    swapchain_create_info.preTransform = surface_caps.currentTransform;
    swapchain_create_info.compositeAlpha = composite_alpha;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;

    VK_CHECK(vkCreateSwapchainKHR(m_device, &swapchain_create_info, nullptr, &m_swapchain));

    VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_image_count, nullptr));

    m_swapchain_images.resize(m_image_count);
    VK_CHECK(vkGetSwapchainImagesKHR(
        m_device, m_swapchain, &m_image_count, m_swapchain_images.data()));

    m_swapchain_image_views.resize(m_swapchain_images.size());
    for (size_t i = 0; i < m_swapchain_images.size(); i++)
    {
        VkImageViewCreateInfo image_view_create_info = {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = m_swapchain_images[i];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = m_image_format;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.layerCount = 1;
        image_view_create_info.subresourceRange.levelCount = 1;

        VK_CHECK(vkCreateImageView(
            m_device, &image_view_create_info, nullptr, &m_swapchain_image_views[i]));
    }
}

void Vulkan_example_app::init_swapchain_for_headless()
{
    // Find suitable image format
    VkFormatFeatureFlags required_features
        = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT
        | VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;

    m_image_format = find_supported_format(m_physical_device,
        m_config.preferred_image_formats, VK_IMAGE_TILING_OPTIMAL, required_features);

    if (m_image_format == VK_FORMAT_UNDEFINED)
    {
        const std::vector<VkFormat> default_formats = {
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_FORMAT_R16G16B16A16_SFLOAT,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_FORMAT_B8G8R8A8_UNORM
        };

        m_image_format = find_supported_format(m_physical_device,
            default_formats, VK_IMAGE_TILING_OPTIMAL, required_features);

        std::cerr << "None of the preferred image formats {";
        for (size_t i = 0; i < m_config.preferred_image_formats.size(); i++)
        {
            if (i > 0)
                std::cerr << ", ";
            std::cerr << vkformat_to_str(m_config.preferred_image_formats[i]);
        }
        std::cerr << "} ";
        
        if (m_image_format == VK_FORMAT_UNDEFINED)
        {
            std::cerr << " or default formats {";
            for (size_t i = 0; i < default_formats.size(); i++)
            {
                if (i > 0)
                    std::cerr << ", ";
                std::cerr << vkformat_to_str(default_formats[i]);
            }
            std::cerr << "} are supported.";
            terminate();
        }
        
        std::cerr << " are supported. Choosing the first supported default format ("
            << vkformat_to_str(m_image_format) << ") instead.\n\n";
    }

    // Check if the image size is valid, if not clamp it
    VkImageUsageFlags usage_flags
        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    VkImageFormatProperties image_format_props;
    VK_CHECK(vkGetPhysicalDeviceImageFormatProperties(m_physical_device, m_image_format,
        VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usage_flags, 0, &image_format_props));

    m_image_width = std::min(m_config.image_width, image_format_props.maxExtent.width);
    m_image_height = std::min(m_config.image_height, image_format_props.maxExtent.height);

    // No restrictions on how many images we can create
    m_image_count = m_config.image_count;

    // Create the swapchain image resources
    m_swapchain_images.resize(m_image_count);
    m_swapchain_device_memories.resize(m_image_count);
    m_swapchain_image_views.resize(m_image_count);
    
    for (uint32_t i = 0; i < m_image_count; i++)
    {
        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = m_image_format;
        image_create_info.extent = { m_image_width, m_image_height, 1 };
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage = usage_flags;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VK_CHECK(vkCreateImage(
            m_device, &image_create_info, nullptr, &m_swapchain_images[i]));

        m_swapchain_device_memories[i] = allocate_and_bind_image_memory(
            m_device, m_physical_device, m_swapchain_images[i],
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkImageViewCreateInfo image_view_create_info = {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = m_swapchain_images[i];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = m_image_format;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.layerCount = 1;
        image_view_create_info.subresourceRange.levelCount = 1;

        VK_CHECK(vkCreateImageView(
            m_device, &image_view_create_info, nullptr, &m_swapchain_image_views[i]));
    }
}

void Vulkan_example_app::init_depth_stencil_buffer()
{
    m_depth_stencil_format = find_supported_format(m_physical_device,
        { VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    if (m_depth_stencil_format == VK_FORMAT_UNDEFINED)
    {
        std::cerr << "No supported depth stencil format found.\n";
        terminate();
    }

    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = m_depth_stencil_format;
    image_create_info.extent = { m_image_width, m_image_height, 1 };
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK(vkCreateImage(m_device, &image_create_info, nullptr, &m_depth_stencil_image));

    m_depth_stencil_device_memory = allocate_and_bind_image_memory(
        m_device, m_physical_device, m_depth_stencil_image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = m_depth_stencil_image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = m_depth_stencil_format;
    if (has_stencil_component(m_depth_stencil_format))
    {
        image_view_create_info.subresourceRange.aspectMask
            = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    image_view_create_info.subresourceRange.layerCount = 1;
    image_view_create_info.subresourceRange.levelCount = 1;

    VK_CHECK(vkCreateImageView(
        m_device, &image_view_create_info, nullptr, &m_depth_stencil_image_view));
}

void Vulkan_example_app::init_render_pass()
{
    VkImageLayout final_layout = m_config.headless
        ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    m_main_render_pass = create_simple_render_pass(
        m_device, m_image_format, m_depth_stencil_format, final_layout);
}

void Vulkan_example_app::init_framebuffers()
{
    m_framebuffers.resize(m_image_count);
    for (size_t i = 0; i < m_image_count; i++)
    {
        const VkImageView attachments[] = {
            m_swapchain_image_views[i],
            m_depth_stencil_image_view
        };

        VkFramebufferCreateInfo framebuffer_create_info = {};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = m_main_render_pass;
        framebuffer_create_info.attachmentCount = std::size(attachments);
        framebuffer_create_info.pAttachments = attachments;
        framebuffer_create_info.width = m_image_width;
        framebuffer_create_info.height = m_image_height;
        framebuffer_create_info.layers = 1;

        VK_CHECK(vkCreateFramebuffer(
            m_device, &framebuffer_create_info, nullptr, &m_framebuffers[i]));
    }
}

void Vulkan_example_app::init_command_pool_and_buffers()
{
    if (!m_command_pool)
    {
        // Create command pool
        VkCommandPoolCreateInfo command_pool_create_info = {};
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex = m_graphics_queue_family_index;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VK_CHECK(vkCreateCommandPool(
            m_device, &command_pool_create_info, nullptr, &m_command_pool));
    }
    else
    {
        // If called by recreate_swapchain the pool doesn't need to be recreated,
        // only the command buffer need to be freed
        vkFreeCommandBuffers(m_device, m_command_pool,
            static_cast<uint32_t>(m_command_buffers.size()), m_command_buffers.data());
        m_command_buffers.clear();
    }

    // One command buffer per frame
    m_command_buffers.resize(m_image_count, nullptr);

    VkCommandBufferAllocateInfo command_buffer_aloc_info = {};
    command_buffer_aloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_aloc_info.commandPool = m_command_pool;
    command_buffer_aloc_info.commandBufferCount = m_image_count;
    command_buffer_aloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(vkAllocateCommandBuffers(
        m_device, &command_buffer_aloc_info, m_command_buffers.data()));
}

void Vulkan_example_app::init_synchronization_objects()
{
    // If called in recreate_swapchain only the fences need to be created
    if (m_image_available_semaphores.empty())
    {
        m_image_available_semaphores.resize(m_image_count);
        m_render_finished_semaphores.resize(m_image_count);

        VkSemaphoreCreateInfo semaphore_create_info = {};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (size_t i = 0; i < m_image_count; i++)
        {
            VK_CHECK(vkCreateSemaphore(
                m_device, &semaphore_create_info, nullptr, &m_image_available_semaphores[i]));
            VK_CHECK(vkCreateSemaphore(
                m_device, &semaphore_create_info, nullptr, &m_render_finished_semaphores[i]));
        }
    }

    m_frame_inflight_fences.resize(m_image_count);
    for (size_t i = 0; i < m_image_count; i++)
    {
        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VK_CHECK(vkCreateFence(
            m_device, &fence_create_info, nullptr, &m_frame_inflight_fences[i]));
    }
}

void Vulkan_example_app::recreate_swapchain_or_framebuffer_image()
{
    if (!m_config.headless)
    {
        // Wait until the window is unminimized if necessary
        int framebuffer_width = 0;
        int framebuffer_height = 0;
        glfwGetFramebufferSize(m_window, &framebuffer_width, &framebuffer_height);
        while (framebuffer_width == 0 || framebuffer_height == 0)
        {
            glfwGetFramebufferSize(m_window, &framebuffer_width, &framebuffer_height);
            glfwWaitEvents();
        }
    }

    VK_CHECK(vkDeviceWaitIdle(m_device));

    // Cleanup swapchain related resources
    vkDestroyImageView(m_device, m_depth_stencil_image_view, nullptr);
    vkDestroyImage(m_device, m_depth_stencil_image, nullptr);
    vkFreeMemory(m_device, m_depth_stencil_device_memory, nullptr);

    for (VkImageView image_view : m_swapchain_image_views)
        vkDestroyImageView(m_device, image_view, nullptr);
    if (m_config.headless)
    {
        for (VkImage image : m_swapchain_images)
            vkDestroyImage(m_device, image, nullptr);
        for (VkDeviceMemory device_memory : m_swapchain_device_memories)
            vkFreeMemory(m_device, device_memory, nullptr);
    }
    else
        vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    for (VkFramebuffer framebuffer : m_framebuffers)
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    vkDestroyRenderPass(m_device, m_main_render_pass, nullptr);

    for (VkFence fence : m_frame_inflight_fences)
        vkDestroyFence(m_device, fence, nullptr);

    // Recreate swapchain related resources
    if (m_config.headless)
        init_swapchain_for_headless();
    else
        init_swapchain_for_window();
    init_depth_stencil_buffer();
    init_render_pass();
    init_framebuffers();
    init_command_pool_and_buffers();
    init_synchronization_objects();
    
    recreate_size_dependent_resources();
}

void Vulkan_example_app::render_loop_iteration(uint32_t frame_index, uint32_t image_index, double& last_frame_time)
{
    // Measure elapsed seconds since last frame and update application logic
    double current_frame_time = glfwGetTime();
    float elapsed_time = static_cast<float>(current_frame_time - last_frame_time);
    last_frame_time = current_frame_time;

    update(elapsed_time, frame_index);

    // Record command buffer
    VkCommandBuffer command_buffer = m_command_buffers[frame_index];
    VK_CHECK(vkResetCommandBuffer(command_buffer, 0));

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

    // Let application fill the command buffer
    render(command_buffer, frame_index, image_index);

    VK_CHECK(vkEndCommandBuffer(command_buffer));

    // Submit recorded command buffer to GPU
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    
    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    if (!m_config.headless)
    {
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &m_image_available_semaphores[frame_index];
        submit_info.pWaitDstStageMask = &wait_stage;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &m_render_finished_semaphores[frame_index];
    }

    VK_CHECK(vkQueueSubmit(
        m_graphics_queue, 1, &submit_info, m_frame_inflight_fences[frame_index]));
}

void Vulkan_example_app::internal_key_callback(
    GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    auto app = static_cast<Vulkan_example_app*>(glfwGetWindowUserPointer(window));
    app->key_callback(key, action, mods);
}

void Vulkan_example_app::internal_mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods)
{
    auto app = static_cast<Vulkan_example_app*>(glfwGetWindowUserPointer(window));
    app->mouse_button_callback(button, action);
}

void Vulkan_example_app::internal_mouse_move_callback(GLFWwindow* window, double pos_x, double pos_y)
{
    auto app = static_cast<Vulkan_example_app*>(glfwGetWindowUserPointer(window));
    app->mouse_move_callback(static_cast<float>(pos_x), static_cast<float>(pos_y));
}

void Vulkan_example_app::internal_mouse_scroll_callback(
    GLFWwindow* window, double offset_x, double offset_y)
{
    auto app = static_cast<Vulkan_example_app*>(glfwGetWindowUserPointer(window));
    app->mouse_scroll_callback(static_cast<float>(offset_x), static_cast<float>(offset_y));
}

void Vulkan_example_app::internal_resize_callback(GLFWwindow* window, int width, int height)
{
    auto app = static_cast<Vulkan_example_app*>(glfwGetWindowUserPointer(window));
    
    if (app->m_config.headless)
        app->recreate_swapchain_or_framebuffer_image();

    app->resized_callback(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    app->m_framebuffer_resized = true;
}

void Vulkan_example_app::glfw_error_callback(int error_code, const char* description)
{
    std::cerr << "GLFW error (code: " << error_code << "): \"" << description << "\"\n";
}

VKAPI_ATTR VkBool32 VKAPI_PTR Vulkan_example_app::debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    std::cerr << callback_data->pMessage << "\n";
    return VK_FALSE;
}


Staging_buffer::Staging_buffer(VkDevice device, VkPhysicalDevice physical_device,
    VkDeviceSize size, VkBufferUsageFlags usage)
: m_device(device)
{
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = size;
    buffer_create_info.usage = usage;

    VK_CHECK(vkCreateBuffer(m_device, &buffer_create_info, nullptr, &m_buffer));

    m_device_memory = mi::examples::vk::allocate_and_bind_buffer_memory(
        m_device, physical_device, m_buffer,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

Staging_buffer::~Staging_buffer()
{
    vkDestroyBuffer(m_device, m_buffer, nullptr);
    vkFreeMemory(m_device, m_device_memory, nullptr);
}

void* Staging_buffer::map_memory() const
{
    void* mapped_data;
    VK_CHECK(vkMapMemory(m_device, m_device_memory, 0, VK_WHOLE_SIZE, 0, &mapped_data));

    return mapped_data;
}

void Staging_buffer::unmap_memory() const
{
    vkUnmapMemory(m_device, m_device_memory);
}


Temporary_command_buffer::Temporary_command_buffer(VkDevice device, VkCommandPool command_pool)
: m_device(device)
, m_command_pool(command_pool)
{
    VkCommandBufferAllocateInfo command_buffer_alloc_info= {};
    command_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_alloc_info.commandPool = m_command_pool;
    command_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_alloc_info.commandBufferCount = 1;

    VK_CHECK(vkAllocateCommandBuffers(
        m_device, &command_buffer_alloc_info, &m_command_buffer));
}

Temporary_command_buffer::~Temporary_command_buffer()
{
    vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_command_buffer);
}

void Temporary_command_buffer::begin()
{
    VkCommandBufferBeginInfo command_buffer_begin_info = {};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(m_command_buffer, &command_buffer_begin_info));
}

void Temporary_command_buffer::end_and_submit(VkQueue queue, bool wait)
{
    VK_CHECK(vkEndCommandBuffer(m_command_buffer));

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffer;

    VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, nullptr));

    if (wait)
        VK_CHECK(vkQueueWaitIdle(queue));
}


bool check_instance_extensions_support(
    const std::vector<const char*>& requested_extensions)
{
    uint32_t extension_count;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(
        nullptr, &extension_count, available_extensions.data()));

    for (const char* requested_extension : requested_extensions)
    {
        bool found = false;
        for (const VkExtensionProperties& available_extension : available_extensions)
        {
            if (std::strcmp(requested_extension, available_extension.extensionName) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return false;
        }
    }

    return true;
}

bool check_device_extensions_support(VkPhysicalDevice device,
    const std::vector<const char*>& requested_extensions)
{
    uint32_t extension_count;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr));

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    VK_CHECK(vkEnumerateDeviceExtensionProperties(
        device, nullptr, &extension_count, available_extensions.data()));

    for (const char* requested_extension : requested_extensions)
    {
        bool found = false;
        for (const VkExtensionProperties& available_extension : available_extensions)
        {
            if (std::strcmp(requested_extension, available_extension.extensionName) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return false;
        }
    }

    return true;
}

bool check_validation_layers_support(
    const std::vector<const char*>& requested_layers)
{
    uint32_t layer_count;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));

    std::vector<VkLayerProperties> available_layers(layer_count);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()));

    for (const char* requested_layer : requested_layers)
    {
        bool found = false;
        for (const VkLayerProperties& available_layer : available_layers)
        {
            if (std::strcmp(requested_layer, available_layer.layerName) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            return false;
        }
    }

    return true;
}

VkShaderModule create_shader_module_from_file(
    VkDevice device, const char* shader_filename, EShLanguage shader_type,
    const std::vector<std::string>& defines, bool optimize)
{
    std::string shader_source = mi::examples::io::read_text_file(
        mi::examples::io::get_executable_folder() + "/" + shader_filename);

    return create_shader_module_from_sources(device, { shader_source }, shader_type, defines, optimize);
}

VkShaderModule create_shader_module_from_sources(
    VkDevice device, const std::vector<std::string_view> shader_sources,
    EShLanguage shader_type, const std::vector<std::string>& defines, bool optimize)
{
    mi::examples::vk::Glsl_compiler glsl_compiler(shader_type, "main");
    glsl_compiler.add_defines(defines);
    for (const std::string_view& source : shader_sources)
        glsl_compiler.add_shader(source);
    std::vector<unsigned int> compiled_shader = glsl_compiler.link_program(optimize);

    VkShaderModuleCreateInfo shader_module_create_info = {};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.pCode = compiled_shader.data();
    shader_module_create_info.codeSize = compiled_shader.size() * sizeof(unsigned int);

    VkShaderModule shader_module;
    VK_CHECK(vkCreateShaderModule(device,
        &shader_module_create_info, nullptr, &shader_module));

    return shader_module;
}

// NOTE: A more sophisticated should be used in a real application.
//       This example simply allocates dedicated memory for each resource.
//       For best practices on Vulkan memory management see:
//       https://developer.nvidia.com/blog/vulkan-dos-donts/
//       https://developer.nvidia.com/vulkan-memory-management
//       https://developer.nvidia.com/what%E2%80%99s-your-vulkan-memory-type
VkDeviceMemory allocate_and_bind_buffer_memory(
    VkDevice device, VkPhysicalDevice physical_device, VkBuffer buffer,
    VkMemoryPropertyFlags memory_property_flags)
{
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);

    VkMemoryAllocateInfo memory_alloc_info = {};
    memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_alloc_info.allocationSize = memory_requirements.size;
    memory_alloc_info.memoryTypeIndex = mi::examples::vk::find_memory_type(physical_device,
        memory_requirements.memoryTypeBits, memory_property_flags);

    VkDeviceMemory device_memory;
    VK_CHECK(vkAllocateMemory(device, &memory_alloc_info, nullptr, &device_memory));
    VK_CHECK(vkBindBufferMemory(device, buffer, device_memory, 0));

    return device_memory;
}

// NOTE: A more sophisticated should be used in a real application.
//       This example simply allocates dedicated memory for each resource.
//       For best practices on Vulkan memory management see:
//       https://developer.nvidia.com/blog/vulkan-dos-donts/
//       https://developer.nvidia.com/vulkan-memory-management
//       https://developer.nvidia.com/what%E2%80%99s-your-vulkan-memory-type
VkDeviceMemory allocate_and_bind_image_memory(
    VkDevice device, VkPhysicalDevice physical_device, VkImage image,
    VkMemoryPropertyFlags memory_property_flags)
{
    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(device, image, &memory_requirements);

    VkMemoryAllocateInfo memory_alloc_info = {};
    memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_alloc_info.allocationSize = memory_requirements.size;
    memory_alloc_info.memoryTypeIndex = mi::examples::vk::find_memory_type(physical_device,
        memory_requirements.memoryTypeBits, memory_property_flags);

    VkDeviceMemory device_memory;
    VK_CHECK(vkAllocateMemory(device, &memory_alloc_info, nullptr, &device_memory));
    VK_CHECK(vkBindImageMemory(device, image, device_memory, 0));

    return device_memory;
}

uint32_t find_memory_type(
    VkPhysicalDevice physical_device,
    uint32_t memory_type_bits_requirement,
    VkMemoryPropertyFlags required_properties)
{
    VkPhysicalDeviceMemoryProperties device_memory_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &device_memory_props);

    const uint32_t memory_count = device_memory_props.memoryTypeCount;
    for (uint32_t memory_index = 0; memory_index < memory_count; memory_index++)
    {
        uint32_t memory_type_bits = (1 << memory_index);
        bool is_required_memory_type = (memory_type_bits_requirement & memory_type_bits);

        VkMemoryPropertyFlags property_flags
            = device_memory_props.memoryTypes[memory_index].propertyFlags;
        bool has_required_properties
            = (property_flags & required_properties) == required_properties;

        if (is_required_memory_type && has_required_properties)
            return memory_index;
    }

    return 0xffffffff;
}

VkFormat find_supported_format(
    VkPhysicalDevice physical_device,
    const std::vector<VkFormat>& formats,
    VkImageTiling tiling,
    VkFormatFeatureFlags feature_flags)
{
    for (VkFormat format : formats)
    {
        VkFormatProperties format_properties;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);

        if (tiling == VK_IMAGE_TILING_LINEAR
            && (format_properties.linearTilingFeatures & feature_flags) == feature_flags)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL
            && (format_properties.optimalTilingFeatures & feature_flags) == feature_flags)
        {
            return format;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

bool has_stencil_component(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

uint32_t get_image_format_bpp(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_R8_SNORM:
    case VK_FORMAT_R8_USCALED:
    case VK_FORMAT_R8_SSCALED:
    case VK_FORMAT_R8_UINT:
    case VK_FORMAT_R8_SINT:
    case VK_FORMAT_R8_SRGB:
    case VK_FORMAT_S8_UINT:
        return 1;

    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
    case VK_FORMAT_R8G8_UNORM:
    case VK_FORMAT_R8G8_SNORM:
    case VK_FORMAT_R8G8_USCALED:
    case VK_FORMAT_R8G8_SSCALED:
    case VK_FORMAT_R8G8_UINT:
    case VK_FORMAT_R8G8_SINT:
    case VK_FORMAT_R8G8_SRGB:
    case VK_FORMAT_R16_UNORM:
    case VK_FORMAT_R16_SNORM:
    case VK_FORMAT_R16_USCALED:
    case VK_FORMAT_R16_SSCALED:
    case VK_FORMAT_R16_UINT:
    case VK_FORMAT_R16_SINT:
    case VK_FORMAT_R16_SFLOAT:
    case VK_FORMAT_D16_UNORM:
        return 2;

    case VK_FORMAT_R8G8B8_UNORM:
    case VK_FORMAT_R8G8B8_SNORM:
    case VK_FORMAT_R8G8B8_USCALED:
    case VK_FORMAT_R8G8B8_SSCALED:
    case VK_FORMAT_R8G8B8_UINT:
    case VK_FORMAT_R8G8B8_SINT:
    case VK_FORMAT_R8G8B8_SRGB:
    case VK_FORMAT_B8G8R8_UNORM:
    case VK_FORMAT_B8G8R8_SNORM:
    case VK_FORMAT_B8G8R8_USCALED:
    case VK_FORMAT_B8G8R8_SSCALED:
    case VK_FORMAT_B8G8R8_UINT:
    case VK_FORMAT_B8G8R8_SINT:
    case VK_FORMAT_B8G8R8_SRGB:
    case VK_FORMAT_D16_UNORM_S8_UINT:
        return 3;

    case VK_FORMAT_R8G8B8A8_UNORM:
    case VK_FORMAT_R8G8B8A8_SNORM:
    case VK_FORMAT_R8G8B8A8_USCALED:
    case VK_FORMAT_R8G8B8A8_SSCALED:
    case VK_FORMAT_R8G8B8A8_UINT:
    case VK_FORMAT_R8G8B8A8_SINT:
    case VK_FORMAT_R8G8B8A8_SRGB:
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_B8G8R8A8_SNORM:
    case VK_FORMAT_B8G8R8A8_USCALED:
    case VK_FORMAT_B8G8R8A8_SSCALED:
    case VK_FORMAT_B8G8R8A8_UINT:
    case VK_FORMAT_B8G8R8A8_SINT:
    case VK_FORMAT_B8G8R8A8_SRGB:
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
    case VK_FORMAT_R16G16_UNORM:
    case VK_FORMAT_R16G16_SNORM:
    case VK_FORMAT_R16G16_USCALED:
    case VK_FORMAT_R16G16_SSCALED:
    case VK_FORMAT_R16G16_UINT:
    case VK_FORMAT_R16G16_SINT:
    case VK_FORMAT_R16G16_SFLOAT:
    case VK_FORMAT_R32_UINT:
    case VK_FORMAT_R32_SINT:
    case VK_FORMAT_R32_SFLOAT:
    case VK_FORMAT_D32_SFLOAT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    case VK_FORMAT_G8B8G8R8_422_UNORM:
    case VK_FORMAT_B8G8R8G8_422_UNORM:
        return 4;

    case VK_FORMAT_R16G16B16_UNORM:
    case VK_FORMAT_R16G16B16_SNORM:
    case VK_FORMAT_R16G16B16_USCALED:
    case VK_FORMAT_R16G16B16_SSCALED:
    case VK_FORMAT_R16G16B16_UINT:
    case VK_FORMAT_R16G16B16_SINT:
    case VK_FORMAT_R16G16B16_SFLOAT:
        return 6;

    case VK_FORMAT_R16G16B16A16_UNORM:
    case VK_FORMAT_R16G16B16A16_SNORM:
    case VK_FORMAT_R16G16B16A16_USCALED:
    case VK_FORMAT_R16G16B16A16_SSCALED:
    case VK_FORMAT_R16G16B16A16_UINT:
    case VK_FORMAT_R16G16B16A16_SINT:
    case VK_FORMAT_R16G16B16A16_SFLOAT:
    case VK_FORMAT_R32G32_UINT:
    case VK_FORMAT_R32G32_SINT:
    case VK_FORMAT_R32G32_SFLOAT:
    case VK_FORMAT_R64_UINT:
    case VK_FORMAT_R64_SINT:
    case VK_FORMAT_R64_SFLOAT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return 8;

    case VK_FORMAT_R32G32B32_UINT:
    case VK_FORMAT_R32G32B32_SINT:
    case VK_FORMAT_R32G32B32_SFLOAT:
        return 12;

    case VK_FORMAT_R32G32B32A32_UINT:
    case VK_FORMAT_R32G32B32A32_SINT:
    case VK_FORMAT_R32G32B32A32_SFLOAT:
    case VK_FORMAT_R64G64_UINT:
    case VK_FORMAT_R64G64_SINT:
    case VK_FORMAT_R64G64_SFLOAT:
        return 16;

    case VK_FORMAT_R64G64B64_UINT:
    case VK_FORMAT_R64G64B64_SINT:
    case VK_FORMAT_R64G64B64_SFLOAT:
        return 24;

    case VK_FORMAT_R64G64B64A64_UINT:
    case VK_FORMAT_R64G64B64A64_SINT:
    case VK_FORMAT_R64G64B64A64_SFLOAT:
        return 32;
    
    default:
        return 0;
    }
}

VkPipeline create_fullscreen_triangle_graphics_pipeline(
    VkDevice device, VkPipelineLayout pipeline_layout,
    VkShaderModule vertex_shader, VkShaderModule fragment_shader,
    VkRenderPass render_pass, uint32_t subpass,
    uint32_t image_width, uint32_t image_height, bool cull_ccw)
{
    VkPipelineVertexInputStateCreateInfo vertex_input_state = {};
    vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {};
    input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state.primitiveRestartEnable = false;

    VkViewport viewport = { 0.0f, 0.0f, (float)image_width, (float)image_height, 0.0f, 1.0f };
    VkRect2D scissor_rect = { {0, 0}, {image_width, image_height} };

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor_rect;

    VkPipelineRasterizationStateCreateInfo rasterization_state = {};
    rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state.depthClampEnable = false;
    rasterization_state.rasterizerDiscardEnable = false;
    rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_state.frontFace = cull_ccw ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterization_state.depthBiasEnable = false;
    rasterization_state.lineWidth = 1.0f;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};
    depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state.depthTestEnable = false;
    depth_stencil_state.depthWriteEnable = false;
    depth_stencil_state.depthBoundsTestEnable = false;
    depth_stencil_state.stencilTestEnable = false;

    VkPipelineMultisampleStateCreateInfo multisample_state = {};
    multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineShaderStageCreateInfo vertex_shader_stage_info = {};
    vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_info.module = vertex_shader;
    vertex_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_stage_info = {};
    fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_info.module = fragment_shader;
    fragment_shader_stage_info.pName = "main";

    const VkPipelineShaderStageCreateInfo shader_stages[] = {
        vertex_shader_stage_info,
        fragment_shader_stage_info
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.blendEnable = false;
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend_state = {};
    color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state.logicOpEnable = false;
    color_blend_state.attachmentCount = 1;
    color_blend_state.pAttachments = &color_blend_attachment;

    VkGraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = std::size(shader_stages);
    pipeline_create_info.pStages = shader_stages;
    pipeline_create_info.pVertexInputState = &vertex_input_state;
    pipeline_create_info.pInputAssemblyState = &input_assembly_state;
    pipeline_create_info.pViewportState = &viewport_state;
    pipeline_create_info.pRasterizationState = &rasterization_state;
    pipeline_create_info.pDepthStencilState = &depth_stencil_state;
    pipeline_create_info.pMultisampleState = &multisample_state;
    pipeline_create_info.pColorBlendState = &color_blend_state;
    pipeline_create_info.layout = pipeline_layout;
    pipeline_create_info.renderPass = render_pass;
    pipeline_create_info.subpass = 0;

    VkPipeline graphics_pipeline;
    VK_CHECK(vkCreateGraphicsPipelines(
        device, nullptr, 1, &pipeline_create_info, nullptr, &graphics_pipeline));

    return graphics_pipeline;
}

VkRenderPass create_simple_color_only_render_pass(
    VkDevice device, VkFormat image_format, VkImageLayout final_layout)
{
    VkAttachmentDescription attachment_desc = {};
    attachment_desc.format = image_format;
    attachment_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_desc.finalLayout = final_layout;

    VkAttachmentReference attachment_reference = {};
    attachment_reference.attachment = 0;
    attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &attachment_reference;

    VkSubpassDependency subpass_dependency = {};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_create_info = {};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &attachment_desc;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    VkRenderPass render_pass;
    VK_CHECK(vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_pass));

    return render_pass;
}

VkRenderPass create_simple_render_pass(
    VkDevice device, VkFormat image_format,
    VkFormat depth_stencil_format, VkImageLayout final_layout)
{
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = final_layout;

    VkAttachmentDescription depth_attachment = {};
    depth_attachment.format = depth_stencil_format;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_reference = {};
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_reference = {};
    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;
    subpass.pDepthStencilAttachment = &depth_attachment_reference;

    VkSubpassDependency subpass_dependency = {};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    const VkAttachmentDescription attachments[]
        = { color_attachment, depth_attachment };

    VkRenderPassCreateInfo render_pass_create_info = {};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = std::size(attachments);
    render_pass_create_info.pAttachments = attachments;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    VkRenderPass render_pass;
    VK_CHECK(vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_pass));

    return render_pass;
}

VkSampler create_linear_sampler(VkDevice device)
{
    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.unnormalizedCoordinates = false;

    VkSampler sampler;
    VK_CHECK(vkCreateSampler(device, &sampler_create_info, nullptr, &sampler));

    return sampler;
}

void transitionImageLayout(
    VkCommandBuffer command_buffer, VkImage image,
    VkAccessFlags src_access_mask, VkAccessFlags dst_access_mask,
    VkImageLayout old_layout, VkImageLayout new_layout,
    VkPipelineStageFlags src_stage_mask, VkPipelineStageFlags dst_stage_mask,
    VkImageAspectFlags aspect_mask, uint32_t base_mip_level, uint32_t mip_level_count)
{
    VkImageMemoryBarrier image_memory_barrier = {};
    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.srcAccessMask = src_access_mask;
    image_memory_barrier.dstAccessMask = dst_access_mask;
    image_memory_barrier.oldLayout = old_layout;
    image_memory_barrier.newLayout = new_layout;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.image = image;
    image_memory_barrier.subresourceRange.aspectMask = aspect_mask;
    image_memory_barrier.subresourceRange.baseMipLevel = base_mip_level;
    image_memory_barrier.subresourceRange.levelCount = mip_level_count;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(command_buffer,
        src_stage_mask,
        dst_stage_mask,
        0,
        0, nullptr,
        0, nullptr,
        1, &image_memory_barrier);
}

std::vector<uint8_t> copy_image_to_buffer(
    VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue queue,
    VkImage image, uint32_t image_width, uint32_t image_height, uint32_t image_bpp,
    VkImageLayout image_layout, bool flip)
{
    size_t staging_buffer_size = image_width * image_height * image_bpp;
    Staging_buffer staging_buffer(device, physical_device,
        staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    Temporary_command_buffer command_buffer(device, command_pool);
    command_buffer.begin();

    // Determine access and stage mask for image based on current layout
    VkAccessFlags from_access_mask;
    VkPipelineStageFlags from_stage_mask;

    switch (image_layout)
    {
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        from_access_mask = VK_ACCESS_SHADER_READ_BIT;
        from_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        break;
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        from_access_mask = VK_ACCESS_MEMORY_READ_BIT;
        from_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        from_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        from_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        from_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        from_stage_mask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        break;
    
    default:
        // TODO: add more specific cases as needed.
        from_access_mask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        from_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }

    VkImageAspectFlags aspect_mask;
    if (image_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
        aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
    else if (image_layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL)
        aspect_mask = VK_IMAGE_ASPECT_STENCIL_BIT;
    else if (image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    else
        aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;

    transitionImageLayout(command_buffer.get(),
        /*image=*/           image,
        /*src_access_mask=*/ from_access_mask,
        /*dst_access_mask=*/ VK_ACCESS_TRANSFER_READ_BIT,
        /*old_layout=*/      image_layout,
        /*new_layout=*/      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        /*src_stage_mask=*/  from_stage_mask,
        /*dst_stage_mask=*/  VK_PIPELINE_STAGE_TRANSFER_BIT,
        /*aspect_mask=*/     aspect_mask);

    // Copy image to buffer
    VkBufferImageCopy copy_region = {};
    copy_region.bufferOffset = 0;
    copy_region.bufferRowLength = 0;
    copy_region.bufferImageHeight = 0;
    copy_region.imageSubresource.aspectMask = aspect_mask;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;
    copy_region.imageOffset = { 0, 0, 0 };
    copy_region.imageExtent = { image_width, image_height, 1 };

    vkCmdCopyImageToBuffer(command_buffer.get(),
        image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        staging_buffer.get(),
        1, &copy_region);

    transitionImageLayout(command_buffer.get(),
        /*image=*/           image,
        /*src_access_mask=*/ VK_ACCESS_TRANSFER_READ_BIT,
        /*dst_access_mask=*/ from_access_mask,
        /*old_layout=*/      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        /*new_layout=*/      image_layout,
        /*src_stage_mask=*/  VK_PIPELINE_STAGE_TRANSFER_BIT,
        /*dst_stage_mask=*/  from_stage_mask,
        /*aspect_mask=*/     aspect_mask);

    command_buffer.end_and_submit(queue);

    // Fill buffer
    std::vector<uint8_t> buffer(staging_buffer_size);
    void* mapped_data = staging_buffer.map_memory();

    if (flip)
    {
        for (uint32_t y = 0; y < image_height; y++)
        {
            // Copy rows in reverse order
            size_t src_offset = ((image_height - y - 1) * image_width) * image_bpp;
            size_t dst_offset = (y * image_width) * image_bpp;
            uint8_t* src_pixel_ptr = &static_cast<uint8_t*>(mapped_data)[src_offset];
            uint8_t* dst_pixel_ptr = &buffer[dst_offset];
            std::memcpy(dst_pixel_ptr, src_pixel_ptr, image_width * image_bpp);
        }
    }
    else
        std::memcpy(buffer.data(), mapped_data, staging_buffer_size);

    staging_buffer.unmap_memory();
    return buffer;
}

const char* vkresult_to_str(VkResult result)
{
    switch (result)
    {
    case VK_SUCCESS:
        return "VK_SUCCESS";
    case VK_NOT_READY:
        return "VK_NOT_READY";
    case VK_TIMEOUT:
        return "VK_TIMEOUT";
    case VK_EVENT_SET:
        return "VK_EVENT_SET";
    case VK_EVENT_RESET:
        return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
        return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
        return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
        return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION:
        return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:
        return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:
        return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV:
        return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
        return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_NOT_PERMITTED_EXT:
        return "VK_ERROR_NOT_PERMITTED_EXT";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR:
        return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR:
        return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR:
        return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR:
        return "VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_PIPELINE_COMPILE_REQUIRED_EXT:
        return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
    case VK_ERROR_UNKNOWN:
    default:
        return "VK_ERROR_UNKNOWN";
    }
}

// Convert some of the common formats to string.
const char* vkformat_to_str(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_UNDEFINED:
        return "VK_FORMAT_UNDEFINED";
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
        return "VK_FORMAT_R5G6B5_UNORM_PACK16";
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
        return "VK_FORMAT_B5G6R5_UNORM_PACK16";
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
    case VK_FORMAT_R8_UNORM:
        return "VK_FORMAT_R8_UNORM";
    case VK_FORMAT_R8_SNORM:
        return "VK_FORMAT_R8_SNORM";
    case VK_FORMAT_R8_USCALED:
        return "VK_FORMAT_R8_USCALED";
    case VK_FORMAT_R8_SSCALED:
        return "VK_FORMAT_R8_SSCALED";
    case VK_FORMAT_R8_UINT:
        return "VK_FORMAT_R8_UINT";
    case VK_FORMAT_R8_SINT:
        return "VK_FORMAT_R8_SINT";
    case VK_FORMAT_R8_SRGB:
        return "VK_FORMAT_R8_SRGB";
    case VK_FORMAT_R8G8_UNORM:
        return "VK_FORMAT_R8G8_UNORM";
    case VK_FORMAT_R8G8_SNORM:
        return "VK_FORMAT_R8G8_SNORM";
    case VK_FORMAT_R8G8_USCALED:
        return "VK_FORMAT_R8G8_USCALED";
    case VK_FORMAT_R8G8_SSCALED:
        return "VK_FORMAT_R8G8_SSCALED";
    case VK_FORMAT_R8G8_UINT:
        return "VK_FORMAT_R8G8_UINT";
    case VK_FORMAT_R8G8_SINT:
        return "VK_FORMAT_R8G8_SINT";
    case VK_FORMAT_R8G8_SRGB:
        return "VK_FORMAT_R8G8_SRGB";
    case VK_FORMAT_R8G8B8_UNORM:
        return "VK_FORMAT_R8G8B8_UNORM";
    case VK_FORMAT_R8G8B8_SNORM:
        return "VK_FORMAT_R8G8B8_SNORM";
    case VK_FORMAT_R8G8B8_USCALED:
        return "VK_FORMAT_R8G8B8_USCALED";
    case VK_FORMAT_R8G8B8_SSCALED:
        return "VK_FORMAT_R8G8B8_SSCALED";
    case VK_FORMAT_R8G8B8_UINT:
        return "VK_FORMAT_R8G8B8_UINT";
    case VK_FORMAT_R8G8B8_SINT:
        return "VK_FORMAT_R8G8B8_SINT";
    case VK_FORMAT_R8G8B8_SRGB:
        return "VK_FORMAT_R8G8B8_SRGB";
    case VK_FORMAT_B8G8R8_UNORM:
        return "VK_FORMAT_B8G8R8_UNORM";
    case VK_FORMAT_B8G8R8_SNORM:
        return "VK_FORMAT_B8G8R8_SNORM";
    case VK_FORMAT_B8G8R8_USCALED:
        return "VK_FORMAT_B8G8R8_USCALED";
    case VK_FORMAT_B8G8R8_SSCALED:
        return "VK_FORMAT_B8G8R8_SSCALED";
    case VK_FORMAT_B8G8R8_UINT:
        return "VK_FORMAT_B8G8R8_UINT";
    case VK_FORMAT_B8G8R8_SINT:
        return "VK_FORMAT_B8G8R8_SINT";
    case VK_FORMAT_B8G8R8_SRGB:
        return "VK_FORMAT_B8G8R8_SRGB";
    case VK_FORMAT_R8G8B8A8_UNORM:
        return "VK_FORMAT_R8G8B8A8_UNORM";
    case VK_FORMAT_R8G8B8A8_SNORM:
        return "VK_FORMAT_R8G8B8A8_SNORM";
    case VK_FORMAT_R8G8B8A8_USCALED:
        return "VK_FORMAT_R8G8B8A8_USCALED";
    case VK_FORMAT_R8G8B8A8_SSCALED:
        return "VK_FORMAT_R8G8B8A8_SSCALED";
    case VK_FORMAT_R8G8B8A8_UINT:
        return "VK_FORMAT_R8G8B8A8_UINT";
    case VK_FORMAT_R8G8B8A8_SINT:
        return "VK_FORMAT_R8G8B8A8_SINT";
    case VK_FORMAT_R8G8B8A8_SRGB:
        return "VK_FORMAT_R8G8B8A8_SRGB";
    case VK_FORMAT_B8G8R8A8_UNORM:
        return "VK_FORMAT_B8G8R8A8_UNORM";
    case VK_FORMAT_B8G8R8A8_SNORM:
        return "VK_FORMAT_B8G8R8A8_SNORM";
    case VK_FORMAT_B8G8R8A8_USCALED:
        return "VK_FORMAT_B8G8R8A8_USCALED";
    case VK_FORMAT_B8G8R8A8_SSCALED:
        return "VK_FORMAT_B8G8R8A8_SSCALED";
    case VK_FORMAT_B8G8R8A8_UINT:
        return "VK_FORMAT_B8G8R8A8_UINT";
    case VK_FORMAT_B8G8R8A8_SINT:
        return "VK_FORMAT_B8G8R8A8_SINT";
    case VK_FORMAT_B8G8R8A8_SRGB:
        return "VK_FORMAT_B8G8R8A8_SRGB";
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
    case VK_FORMAT_R16_UNORM:
        return "VK_FORMAT_R16_UNORM";
    case VK_FORMAT_R16_SNORM:
        return "VK_FORMAT_R16_SNORM";
    case VK_FORMAT_R16_USCALED:
        return "VK_FORMAT_R16_USCALED";
    case VK_FORMAT_R16_SSCALED:
        return "VK_FORMAT_R16_SSCALED";
    case VK_FORMAT_R16_UINT:
        return "VK_FORMAT_R16_UINT";
    case VK_FORMAT_R16_SINT:
        return "VK_FORMAT_R16_SINT";
    case VK_FORMAT_R16_SFLOAT:
        return "VK_FORMAT_R16_SFLOAT";
    case VK_FORMAT_R16G16_UNORM:
        return "VK_FORMAT_R16G16_UNORM";
    case VK_FORMAT_R16G16_SNORM:
        return "VK_FORMAT_R16G16_SNORM";
    case VK_FORMAT_R16G16_USCALED:
        return "VK_FORMAT_R16G16_USCALED";
    case VK_FORMAT_R16G16_SSCALED:
        return "VK_FORMAT_R16G16_SSCALED";
    case VK_FORMAT_R16G16_UINT:
        return "VK_FORMAT_R16G16_UINT";
    case VK_FORMAT_R16G16_SINT:
        return "VK_FORMAT_R16G16_SINT";
    case VK_FORMAT_R16G16_SFLOAT:
        return "VK_FORMAT_R16G16_SFLOAT";
    case VK_FORMAT_R16G16B16_UNORM:
        return "VK_FORMAT_R16G16B16_UNORM";
    case VK_FORMAT_R16G16B16_SNORM:
        return "VK_FORMAT_R16G16B16_SNORM";
    case VK_FORMAT_R16G16B16_USCALED:
        return "VK_FORMAT_R16G16B16_USCALED";
    case VK_FORMAT_R16G16B16_SSCALED:
        return "VK_FORMAT_R16G16B16_SSCALED";
    case VK_FORMAT_R16G16B16_UINT:
        return "VK_FORMAT_R16G16B16_UINT";
    case VK_FORMAT_R16G16B16_SINT:
        return "VK_FORMAT_R16G16B16_SINT";
    case VK_FORMAT_R16G16B16_SFLOAT:
        return "VK_FORMAT_R16G16B16_SFLOAT";
    case VK_FORMAT_R16G16B16A16_UNORM:
        return "VK_FORMAT_R16G16B16A16_UNORM";
    case VK_FORMAT_R16G16B16A16_SNORM:
        return "VK_FORMAT_R16G16B16A16_SNORM";
    case VK_FORMAT_R16G16B16A16_USCALED:
        return "VK_FORMAT_R16G16B16A16_USCALED";
    case VK_FORMAT_R16G16B16A16_SSCALED:
        return "VK_FORMAT_R16G16B16A16_SSCALED";
    case VK_FORMAT_R16G16B16A16_UINT:
        return "VK_FORMAT_R16G16B16A16_UINT";
    case VK_FORMAT_R16G16B16A16_SINT:
        return "VK_FORMAT_R16G16B16A16_SINT";
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return "VK_FORMAT_R16G16B16A16_SFLOAT";
    case VK_FORMAT_R32_UINT:
        return "VK_FORMAT_R32_UINT";
    case VK_FORMAT_R32_SINT:
        return "VK_FORMAT_R32_SINT";
    case VK_FORMAT_R32_SFLOAT:
        return "VK_FORMAT_R32_SFLOAT";
    case VK_FORMAT_R32G32_UINT:
        return "VK_FORMAT_R32G32_UINT";
    case VK_FORMAT_R32G32_SINT:
        return "VK_FORMAT_R32G32_SINT";
    case VK_FORMAT_R32G32_SFLOAT:
        return "VK_FORMAT_R32G32_SFLOAT";
    case VK_FORMAT_R32G32B32_UINT:
        return "VK_FORMAT_R32G32B32_UINT";
    case VK_FORMAT_R32G32B32_SINT:
        return "VK_FORMAT_R32G32B32_SINT";
    case VK_FORMAT_R32G32B32_SFLOAT:
        return "VK_FORMAT_R32G32B32_SFLOAT";
    case VK_FORMAT_R32G32B32A32_UINT:
        return "VK_FORMAT_R32G32B32A32_UINT";
    case VK_FORMAT_R32G32B32A32_SINT:
        return "VK_FORMAT_R32G32B32A32_SINT";
    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return "VK_FORMAT_R32G32B32A32_SFLOAT";
    case VK_FORMAT_R64_UINT:
        return "VK_FORMAT_R64_UINT";
    case VK_FORMAT_R64_SINT:
        return "VK_FORMAT_R64_SINT";
    case VK_FORMAT_R64_SFLOAT:
        return "VK_FORMAT_R64_SFLOAT";
    case VK_FORMAT_R64G64_UINT:
        return "VK_FORMAT_R64G64_UINT";
    case VK_FORMAT_R64G64_SINT:
        return "VK_FORMAT_R64G64_SINT";
    case VK_FORMAT_R64G64_SFLOAT:
        return "VK_FORMAT_R64G64_SFLOAT";
    case VK_FORMAT_R64G64B64_UINT:
        return "VK_FORMAT_R64G64B64_UINT";
    case VK_FORMAT_R64G64B64_SINT:
        return "VK_FORMAT_R64G64B64_SINT";
    case VK_FORMAT_R64G64B64_SFLOAT:
        return "VK_FORMAT_R64G64B64_SFLOAT";
    case VK_FORMAT_R64G64B64A64_UINT:
        return "VK_FORMAT_R64G64B64A64_UINT";
    case VK_FORMAT_R64G64B64A64_SINT:
        return "VK_FORMAT_R64G64B64A64_SINT";
    case VK_FORMAT_R64G64B64A64_SFLOAT:
        return "VK_FORMAT_R64G64B64A64_SFLOAT";
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
    case VK_FORMAT_D16_UNORM:
        return "VK_FORMAT_D16_UNORM";
    case VK_FORMAT_X8_D24_UNORM_PACK32:
        return "VK_FORMAT_X8_D24_UNORM_PACK32";
    case VK_FORMAT_D32_SFLOAT:
        return "VK_FORMAT_D32_SFLOAT";
    case VK_FORMAT_S8_UINT:
        return "VK_FORMAT_S8_UINT";
    case VK_FORMAT_D16_UNORM_S8_UINT:
        return "VK_FORMAT_D16_UNORM_S8_UINT";
    case VK_FORMAT_D24_UNORM_S8_UINT:
        return "VK_FORMAT_D24_UNORM_S8_UINT";
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return "VK_FORMAT_D32_SFLOAT_S8_UINT";
    case VK_FORMAT_G8B8G8R8_422_UNORM:
        return "VK_FORMAT_G8B8G8R8_422_UNORM";
    case VK_FORMAT_B8G8R8G8_422_UNORM:
        return "VK_FORMAT_B8G8R8G8_422_UNORM";
    default:
        return "Unknown Format";
    }
}

Vulkan_buffer create_storage_buffer(
    VkDevice device,
    VkPhysicalDevice physical_device,
    VkQueue queue,
    VkCommandPool command_pool,
    const void* buffer_data,
    mi::Size buffer_size)
{
    Vulkan_buffer storage_buffer;

    { // Create the storage buffer in device local memory (VRAM)
        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = buffer_size;
        buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VK_CHECK(vkCreateBuffer(
            device, &buffer_create_info, nullptr, &storage_buffer.buffer));

        // Allocate device memory for the buffer.
        storage_buffer.device_memory = mi::examples::vk::allocate_and_bind_buffer_memory(
            device, physical_device, storage_buffer.buffer,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    {
        mi::examples::vk::Staging_buffer staging_buffer(
            device, physical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        // Memcpy the data into the staging buffer
        void* mapped_data = staging_buffer.map_memory();
        std::memcpy(mapped_data, buffer_data, buffer_size);
        staging_buffer.unmap_memory();

        // Upload the data from the staging buffer into the storage buffer
        mi::examples::vk::Temporary_command_buffer command_buffer(device, command_pool);
        command_buffer.begin();

        VkBufferCopy copy_region = {};
        copy_region.size = buffer_size;

        vkCmdCopyBuffer(command_buffer.get(),
                        staging_buffer.get(), storage_buffer.buffer, 1, &copy_region);

        command_buffer.end_and_submit(queue);
    }

    return storage_buffer;
}

void Vulkan_environment_map::create(
    VkDevice device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphics_queue,
    mi::neuraylib::IImage_api* image_api, mi::neuraylib::ITransaction* transaction,
    const std::string& filePath)
{
    mi::base::Handle<mi::neuraylib::IImage> image(
        transaction->create<mi::neuraylib::IImage>("Image"));

    check_success(image->reset_file(filePath.c_str()) == 0);

    mi::base::Handle<const mi::neuraylib::ICanvas> canvas(image->get_canvas(0, 0, 0));
    const mi::Uint32 res_x = canvas->get_resolution_x();
    const mi::Uint32 res_y = canvas->get_resolution_y();

    // Check, whether we need to convert the image
    char const* image_type = image->get_type(0, 0);
    if (strcmp(image_type, "Color") != 0 && strcmp(image_type, "Float32<4>") != 0)
        canvas = image_api->convert(canvas.get(), "Color");

    // Create the Vulkan texture
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    image_create_info.extent = { res_x, res_y, 1 };
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VK_CHECK(vkCreateImage(device, &image_create_info, nullptr, &texture.image));

    texture.device_memory = mi::examples::vk::allocate_and_bind_image_memory(
        device, physical_device, texture.image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = texture.image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = image_create_info.format;
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = 1;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(device, &image_view_create_info, nullptr, &texture.image_view));

    {                                                                   // Upload image data to the GPU
        size_t staging_buffer_size = res_x * res_y * sizeof(float) * 4; // RGBA32F
        mi::examples::vk::Staging_buffer staging_buffer(device, physical_device,
                                                        staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        // Memcpy the read-only data into the staging buffer
        mi::base::Handle<const mi::neuraylib::ITile> tile(canvas->get_tile());
        void* mapped_data = staging_buffer.map_memory();
        std::memcpy(mapped_data, tile->get_data(), staging_buffer_size);
        staging_buffer.unmap_memory();

        // Upload the read-only data from the staging buffer into the storage buffer
        mi::examples::vk::Temporary_command_buffer command_buffer(device, command_pool);
        command_buffer.begin();

        mi::examples::vk::transitionImageLayout(command_buffer.get(),
                                                /*image=*/texture.image,
                                                /*src_access_mask=*/0,
                                                /*dst_access_mask=*/VK_ACCESS_TRANSFER_WRITE_BIT,
                                                /*old_layout=*/VK_IMAGE_LAYOUT_UNDEFINED,
                                                /*new_layout=*/VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                /*src_stage_mask=*/VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                /*dst_stage_mask=*/VK_PIPELINE_STAGE_TRANSFER_BIT);

        VkBufferImageCopy copy_region = {};
        copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_region.imageSubresource.layerCount = 1;
        copy_region.imageExtent = { res_x, res_y, 1 };

        vkCmdCopyBufferToImage(
            command_buffer.get(), staging_buffer.get(), texture.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

        mi::examples::vk::transitionImageLayout(command_buffer.get(),
                                                /*image=*/texture.image,
                                                /*src_access_mask=*/VK_ACCESS_TRANSFER_WRITE_BIT,
                                                /*dst_access_mask=*/VK_ACCESS_SHADER_READ_BIT,
                                                /*old_layout=*/VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                /*new_layout=*/VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                /*src_stage_mask=*/VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                /*dst_stage_mask=*/VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        command_buffer.end_and_submit(graphics_queue);
    }

    // Create alias map
    struct Env_accel
    {
        uint32_t alias;
        float q;
    };

    auto build_alias_map = [](
                               const std::vector<float>& data,
                               std::vector<Env_accel>& accel) -> float
    {
        // Create qs (normalized)
        float sum = std::accumulate(data.begin(), data.end(), 0.0f);
        uint32_t size = static_cast<uint32_t>(data.size());

        for (uint32_t i = 0; i < size; i++)
            accel[i].q = static_cast<float>(size) * data[i] / sum;

        // Create partition table
        std::vector<uint32_t> partition_table(size);
        uint32_t s = 0;
        uint32_t large = size;
        for (uint32_t i = 0; i < size; i++)
            partition_table[(accel[i].q < 1.0f) ? (s++) : (--large)] = accel[i].alias = i;

        // Create alias map
        for (s = 0; s < large && large < size; ++s)
        {
            uint32_t j = partition_table[s];
            uint32_t k = partition_table[large];
            accel[j].alias = k;
            accel[k].q += accel[j].q - 1.0f;
            large = (accel[k].q < 1.0f) ? (large + 1) : large;
        }

        return sum;
    };

    // Create importance sampling data
    mi::base::Handle<const mi::neuraylib::ITile> tile(canvas->get_tile());
    const float* pixels = static_cast<const float*>(tile->get_data());

    std::vector<Env_accel> env_accel_data(res_x * res_y);
    std::vector<float> importance_data(res_x * res_y);
    float cos_theta0 = 1.0f;
    const float step_phi = static_cast<float>(2.0 * M_PI / res_x);
    const float step_theta = static_cast<float>(M_PI / res_y);
    for (uint32_t y = 0; y < res_y; y++)
    {
        const float theta1 = static_cast<float>(y + 1) * step_theta;
        const float cos_theta1 = std::cos(theta1);
        const float area = (cos_theta0 - cos_theta1) * step_phi;
        cos_theta0 = cos_theta1;

        for (uint32_t x = 0; x < res_x; x++)
        {
            const uint32_t idx = y * res_x + x;
            const uint32_t idx4 = idx * 4;
            const float max_channel = std::max(pixels[idx4], std::max(pixels[idx4 + 1], pixels[idx4 + 2]));
            importance_data[idx] = area * max_channel;
        }
    }
    float integral = build_alias_map(importance_data, env_accel_data);
    environment_inv_integral = 1.0f / integral;

    // Create Vulkan buffer for importance sampling data
    sampling_data_buffer = mi::examples::vk::create_storage_buffer(
        device, physical_device, graphics_queue, command_pool, env_accel_data.data(), env_accel_data.size() * sizeof(Env_accel));

    // Create sampler
    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.unnormalizedCoordinates = false;

    VK_CHECK(vkCreateSampler(device, &sampler_create_info, nullptr, &sampler));
}

} // namespace mi::examples::vk
