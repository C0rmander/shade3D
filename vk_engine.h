#ifndef VK_ENGINE_H
#define VK_ENGINE_H
#pragma once
#include "vk_type.h"
#include <vector>
#include <deque>
#include <functional>
#include "vk_mem_alloc.h"
#include "vk_mesh.h"
#include "Vector4D.h"
#include "Matrix4D.h"
struct DeletionQueue
{
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function)
    {
        deletors.push_back(function);
    }

    void flush()
    {
        // reverse loop over the deletion queue to execute all the functions
        for(auto it = deletors.rbegin(); it != deletors.rend(); it++)
        {
            (*it)(); // call the function
        }

        deletors.clear();
    }
};

struct MeshPushConstants
{
    Vector4D data;
    Matrix4D mat;
};

class vk_engine
{
    public:
    vk_engine();
    VmaAllocator _allocator;
    bool _isInitialized{false};
    int _frameNumber {0};
    VkExtent2D _windowExtent{1700,900};

    struct SDL_Window* _window{nullptr};

    VkInstance _instance; // instance for accessing vulkan library
    VkDebugUtilsMessengerEXT _debug_messenger; // used for outputting debug statements
    VkPhysicalDevice _chosenGPU; // this will allow access to the physical GPU used for rendering
    VkDevice _device; // used to direct graphic commands to the hardware
    VkSurfaceKHR _surface; // Vulkan window surface
    VkSwapchainKHR _swapchain; // Vulkan swapchain
    VkFormat _swapchainImageFormat; // an image format must be defined for the window
    // an image is a handle for the image.
    // an image view is a wrapper which allows you to manipulate the image such as changing colours etc..
    std::vector<VkImage> _swapchainImages; // an array of images in the swapchain
    std::vector<VkImageView> _swapchainImageViews; // an array of image views on the swap chain

    VkQueue _graphicsQueue; // this is the queue our commands will be submitted to
    uint32_t _graphicsQueueFamily; // this is the family of queue we are working with, it defines the types of commands that can be put into that queue

    VkCommandPool _commandPool; // the command pool for the commands
    VkCommandBuffer _mainCommandBuffer; // the buffer the commands will be recorded into

    VkRenderPass _renderpass;

    std::vector<VkFramebuffer> _framebuffers;

    VkSemaphore _presentSemaphore, _renderSemaphore;
    VkFence _renderFence;

    VkPipelineLayout _trianglePipelineLayout;
    VkPipeline _trianglePipeline;

    DeletionQueue _mainDeletionQueue;

    VkPipelineLayout _meshPipelineLayout;
    VkPipeline _meshPipeline;
    Mesh _triangleMesh;
    Mesh _testMesh;
    void init_vulkan();
    // used to initialize everything
    void init();

    //cleans up the engine
    void cleanup();

    // main draw loop
    void draw();

    //starts the main loop
    void run();

    bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule);

    private:
        void init_swapchain();
        void init_commands();
        void init_default_renderpass();
        void init_framebuffers();
        void init_sync_structures();
        void init_pipelines();
        void load_meshes();
        void upload_mesh(Mesh& mesh);
};

class PipelineBuilder
{
public:
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
    VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
    VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
    VkViewport _viewport;
    VkRect2D _scissor;
    VkPipelineRasterizationStateCreateInfo _rasterizer;
    VkPipelineColorBlendAttachmentState _colorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo _multisampling;
    VkPipelineLayout _pipelineLayout;

    VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
};

#endif // VK_ENGINE_H
