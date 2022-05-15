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
#include <unordered_map>
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

struct  Texture
{
    AllocatedImage image;
    VkImageView imageView;
};

struct MeshPushConstants
{
    Vector4D data;
    Matrix4D mat;
};

struct Material
{
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

struct RenderObject
{
    Mesh* mesh;
    Material* material;
    Matrix4D transformMatrix;
};

struct FrameData
{
    VkSemaphore _presentSemaphore, _renderSemaphore;
    VkFence _renderFence;

    VkCommandPool _commandPool; // the command pool for the commands
    VkCommandBuffer _mainCommandBuffer; // the buffer the commands will be recorded into

    AllocatedBuffer cameraBuffer;
    AllocatedBuffer objectBuffer;
    VkDescriptorSet objectDescriptor;
    VkDescriptorSet globalDescriptor;
};

struct CameraData
{
    Matrix4D view;
    Matrix4D projection;
    Matrix4D viewproj;
};

struct ObjectData
{
    Matrix4D modelMatrix;
};


struct GPUSceneData
{
    Vector4D fogColour;
    Vector4D fogDistances;
    Vector4D ambientColour;
    Vector4D sunlightDirection;
    Vector4D sunlightColour;
};

struct UploadContext
{
    VkFence _uploadFence;
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;
};

constexpr unsigned int FRAME_OVERLAP = 2;

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



    VkRenderPass _renderpass;

    std::vector<VkFramebuffer> _framebuffers;

    FrameData _frames[FRAME_OVERLAP];

    FrameData& get_current_frame();

    VkPipelineLayout _trianglePipelineLayout;
    VkPipeline _trianglePipeline;

    DeletionQueue _mainDeletionQueue;

    VkPipelineLayout _meshPipelineLayout;
    VkPipeline _meshPipeline;
    Mesh _triangleMesh;
    Mesh _testMesh;
    std::vector<std::string> mesh_names;

    VkImageView _depthImageView;
    AllocatedImage _depthImage;

    //depth image must have a format
    VkFormat _depthFormat;

    Vector3D camMan{0,0,0};
    float deltaX;
    float deltaY;

    VkDescriptorSetLayout _globalSetLayout;
    VkDescriptorSetLayout _objectSetLayout;
    VkDescriptorPool _descriptorPool;

    VkPhysicalDeviceProperties _gpuProperties;
    GPUSceneData _sceneParameters;
    AllocatedBuffer _sceneParameterBuffer;

    std::vector<RenderObject> _renderables;

    std::unordered_map<std::string,Material> _materials;
    std::unordered_map<std::string,Mesh> _meshes;
    std::unordered_map<std::string, Texture> _loadTextures;

    Material* create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);
    Material* get_material(const std::string& name);

    Mesh* get_mesh(const std::string& name);

    UploadContext _uploadContext;

    void immediate_submit(std::function<void(VkCommandBuffer cmdBuf)>&& function);

    size_t pad_uniform_buffer_size(size_t originalSize);

    void draw_objects(VkCommandBuffer cmdBuf, RenderObject* first, int count);

    AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);


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
        void init_scene();
        void init_descriptors();
        void load_images();
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

    VkPipelineDepthStencilStateCreateInfo _depthStencil;
};

#endif // VK_ENGINE_H
