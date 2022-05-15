#include "vk_type.h"
#include "vk_engine.h"
#include "vk_textures.h"
#include <SDL.h>
#include <SDL_vulkan.h>
#include "vk_initializers.h"
#include <cmath>
#include "vk_type.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
// bootstrap library created by Charles-lunarg
//https://github.com/charles-lunarg/vk-bootstrap
#include "VkBootstrap.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "transform.h"
#include "Matrix4D.h"
#include "Vector3D.h"
#include "vk_mesh.h"
#include "MTL_Loader.h"
using namespace std;
#define VK_CHECK(x)                                                 \
    do                                                              \
    {                                                               \
        VkResult err = x;                                           \
        if(err)                                                     \
        {                                                           \
            std::cout << "Detected Vulkan error" << err << std::endl;\
            abort();                                                \
        }                                                           \
    } while(0)

vk_engine::vk_engine()
{
    //ctor
}

void vk_engine::init()
{
    //HINSTANCE hinstLib;
    //hinstLib = LoadLibrary(TEXT("vulkan-1.dll"));
    // initialize sdl and create sdl window

    system("compileShaders.bat");

    SDL_Init(SDL_INIT_VIDEO);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

    // ceate and empty sdl window
    _window = SDL_CreateWindow(
    "Shade3D Engine", // this is the window title
    SDL_WINDOWPOS_CENTERED, // center the window on the screen x
    SDL_WINDOWPOS_CENTERED, // center the window on the screen y
    _windowExtent.width, // the windows width (pixels)
    _windowExtent.height, // the window height (pixels)
    window_flags
    );

    //std::cout<<err<<std::endl;
    //load the core vulkan structures
    init_vulkan();

    //create the swapchain
    init_swapchain();
    //initialise the commands and command buffer
    init_commands();
    // start renderpass
    init_default_renderpass();
    //initialise framebuffers
    init_framebuffers();
    // initialise structures for syncing commands
    init_sync_structures();
    //initialise descriptors and descriptors sets for shaders to use
    init_descriptors();
    //load the vertex and fragment shaders
    init_pipelines();

    load_images();

    //load meshes
    load_meshes();

    init_scene();
    // make sure everything worked correctly
    _isInitialized = true;


}

void vk_engine::init_vulkan()
{
    std::cout<<"initiating vulkan"<<std::endl;
    vkb::InstanceBuilder builder;

   //create a basic Vulkan instance with debug features
    auto inst_ret = builder.set_app_name("Shade3D")
    .request_validation_layers(true)
    .require_api_version(1,3,0)
    .use_default_debug_messenger()
    .build();

    vkb::Instance vkb_inst = inst_ret.value();

    //Store instance to destroy on program end
    _instance = vkb_inst.instance;
    // store the debug messenger for later
    _debug_messenger = vkb_inst.debug_messenger;
    // aquires SDL window surface
   SDL_bool err = SDL_Vulkan_CreateSurface(_window, _instance, &_surface);
    //select a suitable GPU with VKBootstrap
    // gpu must support Vulkan 1.1 and be able to write to SDL surface
    vkb::PhysicalDeviceSelector selector{vkb_inst};
    vkb::PhysicalDevice physicalDevice = selector
    .set_minimum_version(1,3)
    .set_surface(_surface)
    .select()
    .value();

    //create the actual Vulkan device
    vkb::DeviceBuilder deviceBuilder{physicalDevice};

    vkb::Device vkbDevice = deviceBuilder.build().value();

    // this creates the device handle which will be used in the rest of the Vulkan application
    _device = vkbDevice.device;

    _chosenGPU = physicalDevice.physical_device;

    // get graphics queue using VKBootstrap
    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _chosenGPU;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
    vmaCreateAllocator(&allocatorInfo, &_allocator);

    _gpuProperties = vkbDevice.physical_device.properties;

    std::cout<<"the GPU has a minimum buffer alignment of: " << _gpuProperties.limits.minUniformBufferOffsetAlignment << std::endl;

}

void vk_engine::init_swapchain()
{
    std::cout<<"started creating swapchain"<<std::endl;
    vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};
    //uses vsync present mode
    vkb::Swapchain vkbSwapchain = swapchainBuilder
    .use_default_format_selection()
    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
    .set_desired_extent(_windowExtent.width, _windowExtent.height)
    .build()
    .value();

    //store the swapchain and all of its images
    _swapchain = vkbSwapchain.swapchain;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();
    _swapchainImageFormat = vkbSwapchain.image_format;

    VkExtent3D depthImageExtent = {
        _windowExtent.width,
        _windowExtent.height,
        1
    };

    //depth format will be 32 bit float
    _depthFormat = VK_FORMAT_D32_SFLOAT;

    //the depth image will be an image with the selected format and depth attachment usage flags
    VkImageCreateInfo dimg_info = vkinit::image_create_info(_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    vmaCreateImage(_allocator, &dimg_info, &dimg_allocinfo, &_depthImage._image, &_depthImage._allocation, nullptr);

    //build image view for depth image
    VkImageViewCreateInfo dview_info = vkinit::imageview_create_info(_depthFormat, _depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);

    VK_CHECK(vkCreateImageView(_device, &dview_info, nullptr, &_depthImageView));

    _mainDeletionQueue.push_function([=]() {
                                     vkDestroySwapchainKHR(_device, _swapchain, nullptr);
                                     vkDestroyImageView(_device, _depthImageView, nullptr);
                                     vmaDestroyImage(_allocator, _depthImage._image, _depthImage._allocation);
                                     });
}

void vk_engine::init_commands()
{
    std::cout<<"started creating command pool"<<std::endl;
    //create command pool to submit commands to the graphics queue
    VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);


    for(int i=0; i< FRAME_OVERLAP; i++)
    {
        VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));
        VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_frames[i]._commandPool, 1);

        VK_CHECK((vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer)));

    _mainDeletionQueue.push_function([=](){
                                     vkDestroyCommandPool(_device, _frames[i]._commandPool, nullptr);
                                     });


    }

    VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily);

    VK_CHECK(vkCreateCommandPool(_device, &uploadCommandPoolInfo, nullptr, &_uploadContext._commandPool));

     _mainDeletionQueue.push_function([=](){
                                     vkDestroyCommandPool(_device, _uploadContext._commandPool, nullptr);
                                     });

    VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_uploadContext._commandPool, 1);

    VkCommandBuffer cmdBuf;
    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_uploadContext._commandBuffer));



    //allocate the default command buffer that we will use for rendering



}

void vk_engine::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
{
    VkCommandBuffer cmdBuf = _uploadContext._commandBuffer;

    VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VK_CHECK(vkBeginCommandBuffer(cmdBuf, &cmdBeginInfo));

    function(cmdBuf);

    VK_CHECK(vkEndCommandBuffer(cmdBuf));

    VkSubmitInfo submit = vkinit::submit_info(&cmdBuf);

    VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, _uploadContext._uploadFence));

    vkWaitForFences(_device, 1, &_uploadContext._uploadFence, true, 9999999999);
    vkResetFences(_device, 1, &_uploadContext._uploadFence);


    vkResetCommandPool(_device, _uploadContext._commandPool, 0);


}

FrameData& vk_engine::get_current_frame()
{

    return _frames[_frameNumber % FRAME_OVERLAP];
}

void vk_engine::init_default_renderpass()
{
    // the renderpass will use thids colour attachment
    VkAttachmentDescription colour_attachment = {};
    // make colour attachment format the same as the format needed by the swapchain
    colour_attachment.format = _swapchainImageFormat;
    // sample once, we are not using MSAA - ** NOTE: maybe change later **
    colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // clear when this attachment is loaded
    colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // at the end of the render pass the attachment gets stored
    colour_attachment.storeOp= VK_ATTACHMENT_STORE_OP_STORE;
    // not soing anything with stencil
    colour_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colour_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // starting layout of attachment is unknown
    colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    //image must be ready after end of renderpass
    colour_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colour_attachment_ref = {};
    //this will be the index in the pAttachments array in the parent renderpass
    colour_attachment_ref.attachment = 0;
    colour_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment = {};

    //depth attachment
    depth_attachment.flags = 0;
    depth_attachment.format = _depthFormat;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref = {};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // minimum amount of subpasses is 1
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colour_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkAttachmentDescription attachments[2] = {colour_attachment, depth_attachment};

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    //connect the coliur attachment to the info
    render_pass_info.attachmentCount = 2;
    render_pass_info.pAttachments = &attachments[0];
    // create link between subpass and info
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;


    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency depth_dependency = {};
    depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    depth_dependency.dstSubpass = 0;
    depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depth_dependency.srcAccessMask = 0;
    depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency dependencies[2] = {dependency, depth_dependency};

    render_pass_info.dependencyCount = 2;
    render_pass_info.pDependencies = &dependencies[0];

    VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr, &_renderpass));

    _mainDeletionQueue.push_function([=](){
                                     vkDestroyRenderPass(_device, _renderpass, nullptr);
                                     });
}

void vk_engine::init_framebuffers()
{
  // create framebuffers for the swapchain images.
  //Creates link betweeb render pass and the images for rendering
  VkFramebufferCreateInfo fb_info = {};
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = nullptr;

  fb_info.renderPass = _renderpass;
  fb_info.attachmentCount = 1;
  fb_info.width = _windowExtent.width;
  fb_info.height = _windowExtent.height;
  fb_info.layers = 1;

  // get all images in the swapchain
  const uint32_t swapchain_imagecount = _swapchainImages.size();
  _framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

  //create framebuffer for each swapchain image view
  for(int i =0; i<swapchain_imagecount; i++)
  {
      VkImageView attachments[2];
      attachments[0] = _swapchainImageViews[i];
      attachments[1] = _depthImageView;
      fb_info.pAttachments = attachments;
      fb_info.attachmentCount = 2;
      VK_CHECK(vkCreateFramebuffer(_device, &fb_info, nullptr, &_framebuffers[i]));

      _mainDeletionQueue.push_function([=]()
                                       {
                                           vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
                                           vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
                                       });
  }
}

void vk_engine::init_sync_structures()
{
    // create synchronisation structures

    VkFenceCreateInfo fenceCreateInfo = {};

    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;

    // for the first frame create a fence before the GPU command
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFenceCreateInfo uploadFenceCreateInfo = vkinit::fence_create_info();


    for(int i =0; i < FRAME_OVERLAP; i++)
    {
        VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));
        VK_CHECK(vkCreateFence(_device, &uploadFenceCreateInfo, nullptr, &_uploadContext._uploadFence));

    _mainDeletionQueue.push_function([=](){
                                     vkDestroyFence(_device, _frames[i]._renderFence, nullptr);
                                     vkDestroyFence(_device, _uploadContext._uploadFence, nullptr);

                                     });

    // no flags needed for semaphores
    VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._presentSemaphore));
    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore));

    _mainDeletionQueue.push_function([=](){
                                     vkDestroySemaphore(_device, _frames[i]._presentSemaphore, nullptr);
                                     vkDestroySemaphore(_device, _frames[i]._renderSemaphore, nullptr);
                                     });
    }
}

void vk_engine::init_descriptors()
{
    const size_t sceneParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));

    _sceneParameterBuffer = create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    std::vector<VkDescriptorPoolSize> sizes =
    {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10}

    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = 0;
    pool_info.maxSets = 10;
    pool_info.poolSizeCount = (uint32_t)sizes.size();
    pool_info.pPoolSizes = sizes.data();

    vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptorPool);

    VkDescriptorSetLayoutBinding camBufferBinding = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

    VkDescriptorSetLayoutBinding sceneBinding = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT| VK_SHADER_STAGE_FRAGMENT_BIT, 1);

    VkDescriptorSetLayoutBinding objectBind = vkinit::descriptorset_layout_binding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

    VkDescriptorSetLayoutCreateInfo set2info = {};
    set2info.bindingCount = 1;
    set2info.flags = 0;
    set2info.pNext = nullptr;
    set2info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set2info.pBindings = &objectBind;

    vkCreateDescriptorSetLayout(_device, &set2info, nullptr, &_objectSetLayout);

    VkDescriptorSetLayoutBinding bindings[] = {camBufferBinding, sceneBinding};
//    camBufferBinding.binding = 0;
//    camBufferBinding.descriptorCount = 1;
//    //its a uniform buffer
//    camBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//
//    //its used in the vertex shader
//    camBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo setinfo = {};
    setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setinfo.pNext = nullptr;

    //only have 1 binding
    setinfo.bindingCount = 2;
    //no flags
    setinfo.flags = 0;
    //point to the camera buffer binding
    setinfo.pBindings = bindings;

    vkCreateDescriptorSetLayout(_device, &setinfo, nullptr, &_globalSetLayout);

    for(int i = 0; i< FRAME_OVERLAP; i++)
    {
        const int MAX_OBJECTS = 10000;
        _frames[i].objectBuffer = create_buffer(sizeof(ObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,  VMA_MEMORY_USAGE_CPU_TO_GPU);

        _frames[i].cameraBuffer = create_buffer(sizeof(CameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        //allocate one descriptor set for each frame
        VkDescriptorSetAllocateInfo objectSetAlloc = {};
        objectSetAlloc.pNext = nullptr;
        objectSetAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        //using the pool
        objectSetAlloc.descriptorPool = _descriptorPool;
        //only one descriptor
        objectSetAlloc.descriptorSetCount = 1;
        //using the global data layout
        objectSetAlloc.pSetLayouts = &_objectSetLayout;

        vkAllocateDescriptorSets(_device, &objectSetAlloc, &_frames[i].objectDescriptor);

        //allocate one descriptor set for each frame
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.pNext = nullptr;
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        //using the pool
        allocInfo.descriptorPool = _descriptorPool;
        //only one descriptor
        allocInfo.descriptorSetCount = 1;
        //using the global data layout
        allocInfo.pSetLayouts = &_globalSetLayout;

        vkAllocateDescriptorSets(_device, &allocInfo, &_frames[i].globalDescriptor);

        //set information in the descriptor
        VkDescriptorBufferInfo binfo;

        //set it to the camera buffer
        binfo.buffer = _frames[i].cameraBuffer._buffer;

        //at 0 offset
        binfo.offset = 0;
        //must set size to be same size as camera data struct
        binfo.range = sizeof(CameraData);

        VkDescriptorBufferInfo sceneInfo;

        //set it to the camera buffer
        sceneInfo.buffer = _sceneParameterBuffer._buffer;

        //at 0 offset
        sceneInfo.offset = 0;
        //must set size to be same size as camera data struct
        sceneInfo.range = sizeof(GPUSceneData);

        VkDescriptorBufferInfo objectBufferInfo;
        objectBufferInfo.buffer = _frames[i].objectBuffer._buffer;
        objectBufferInfo.offset = 0;
        objectBufferInfo.range = sizeof(ObjectData) * MAX_OBJECTS;

        VkWriteDescriptorSet setWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _frames[i].globalDescriptor, &binfo, 0);
        VkWriteDescriptorSet sceneWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _frames[i].globalDescriptor, &sceneInfo, 1);
        VkWriteDescriptorSet objectWrite = vkinit::write_descriptor_buffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _frames[i].objectDescriptor, &objectBufferInfo, 0);
        setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

        VkWriteDescriptorSet setWrites[] = {setWrite, sceneWrite, objectWrite};

        vkUpdateDescriptorSets(_device, 3, setWrites, 0, nullptr);
    }

    for(int i = 0; i< FRAME_OVERLAP; i++)
    {
        _mainDeletionQueue.push_function([&](){
                                         vmaDestroyBuffer(_allocator, _frames[i].cameraBuffer._buffer, _frames[i].cameraBuffer._allocation);
                                         vmaDestroyBuffer(_allocator, _frames[i].objectBuffer._buffer, _frames[i].objectBuffer._allocation);
                                         });
    }

    _mainDeletionQueue.push_function([&](){
                                         vkDestroyDescriptorSetLayout(_device, _globalSetLayout, nullptr);
                                         vkDestroyDescriptorSetLayout(_device, _objectSetLayout, nullptr);
                                         vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
                                         });
}
void vk_engine::init_pipelines()
{
    VkShaderModule texturedMeshShader;
    if(!load_shader_module("default_lit.frag.spv", &texturedMeshShader))
    {
        std::cout<<"Error building triangle fragment shader" << std::endl;
    }
    else
    {
        std::cout<<"triangle fragment shader loaded successfully" << std::endl;
    }

    // build the stage-create-info for vertex and fragment stages.
    PipelineBuilder pipelineBuilder;


    //vertex input will control how to read vertices from the vertex buffer
    pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();

    //input assembly is the configuration we chose when drawing to use lists, strips or individual points
    // triangle list will be used
    pipelineBuilder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    //build viewport and scissor from the swapchain data
    pipelineBuilder._viewport.x = 0.0f;
    pipelineBuilder._viewport.y = 0.0f;
    pipelineBuilder._viewport.width = (float) _windowExtent.width;
    pipelineBuilder._viewport.height = (float)_windowExtent.width;
    pipelineBuilder._viewport.minDepth = 0.0f;
    pipelineBuilder._viewport.maxDepth = 1.0f;

    pipelineBuilder._scissor.offset = {0,0};
    pipelineBuilder._scissor.extent = _windowExtent;

    //configure the rasterizer to draw filled triangles
    pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

    //don't use multisampling
    pipelineBuilder._multisampling = vkinit::multisampling_state_create_info();

    //single blend attachment with no blending and writing to RGBA
    pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();

    VertexInputDescription vertexDescription = Vertex::get_vertex_description();

    //connect the pipeline builder vertex input info to the one we get from the vertex
    pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
    pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

    pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
    pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();


    //clear the shader stages
    pipelineBuilder._shaderStages.clear();

    //empty pipeline layout info
    VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = vkinit::pipeline_layout_create_info();

    //setup push constants
    VkPushConstantRange push_constant;
    push_constant.offset = 0;


    //this push constant range takes up the size of a MeshPushConstants struct
    push_constant.size = sizeof(MeshPushConstants);
    //push constant is only available in the vertex shader
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    mesh_pipeline_layout_info.pPushConstantRanges = &push_constant;
    mesh_pipeline_layout_info.pushConstantRangeCount = 1;

    VkDescriptorSetLayout setLayouts[] = {_globalSetLayout, _objectSetLayout};

    mesh_pipeline_layout_info.setLayoutCount = 2;
    mesh_pipeline_layout_info.pSetLayouts = setLayouts;

    VkPipelineLayout meshPipLayout;

    VK_CHECK(vkCreatePipelineLayout(_device, &mesh_pipeline_layout_info, nullptr, &meshPipLayout));

    VkShaderModule meshVertShader;
    if(!load_shader_module("tri_mesh.vert.spv", &meshVertShader))
    {
        std::cout<<"Error when building the triangle vertex shader module" << std::endl;
    }
    else
    {
        std::cout<<"triangle vertex shader successfully loaded" << std::endl;
    }

    pipelineBuilder._pipelineLayout = meshPipLayout;
    //build the mesh triangle pipeline
    pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    _meshPipeline = pipelineBuilder.build_pipeline(_device, _renderpass);

    create_material(_meshPipeline, meshPipLayout, "defaultmesh");

    //add the other shaders
    pipelineBuilder._shaderStages.clear();
    pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));
    pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, texturedMeshShader));

    VkPipeline texPipeline = pipelineBuilder.build_pipeline(_device, _renderpass);
	create_material(texPipeline, meshPipLayout, "texturedmesh");

	// other code ....
	vkDestroyShaderModule(_device, texturedMeshShader, nullptr);

    //create mesh pipelin layout


    //destroy all shader modules, outside of the queue
    vkDestroyShaderModule(_device, meshVertShader, nullptr);
    //vkDestroyShaderModule(_device, triangleVertShader, nullptr);
   // vkDestroyShaderModule(_device, triangleFragShader, nullptr);

    _mainDeletionQueue.push_function([=]()
                                     {
                                         //destroy pipeline
                                         vkDestroyPipeline(_device, _trianglePipeline, nullptr);

                                         vkDestroyPipeline(_device, _meshPipeline, nullptr);

                                        //destroy the pipeline layout
                                         vkDestroyPipelineLayout(_device, _trianglePipelineLayout, nullptr);
                                         vkDestroyPipelineLayout(_device, meshPipLayout, nullptr);



                                     });
}

void vk_engine::init_scene()
{
    for(int i = 0; i < mesh_names.size(); i++)
    {
    RenderObject testRender;
    testRender.mesh = get_mesh(mesh_names[i]);
    testRender.material = get_material("defaultmesh");
    testRender.transformMatrix = Matrix4D(1,0,0,0,
                 0,1,0,0,
                 0,0,1,0,
                 0,0,0,1);

    _renderables.push_back(testRender);

    }
}

void vk_engine::load_meshes()
{
    //make vector 3 vertices long
//    _triangleMesh._vertices.resize(3);
//
//    //vertex positions
//    _triangleMesh._vertices[0].position = {1.f, 1.f, 0.0f};
//    _triangleMesh._vertices[1].position = {-1.f, 1.f, 0.0f};
//    _triangleMesh._vertices[2].position = {0.f, -1.f, 0.0f};
//
//    //vertex colours, all green
//    _triangleMesh._vertices[0].colour = {0.f, 1.f, 0.0f};
//    _triangleMesh._vertices[1].colour = {0.f, 1.f, 0.0f};
//    _triangleMesh._vertices[2].colour = {0.f, 1.f, 0.0f};

    const char* objFile = "E:/codeblockscode/shade3D/D20.obj";
    //const char* objFile = "E:/codeblockscode/shade3D/San_Miguel/san-miguel-low-poly.obj";
   // const char* objFile = "E:/codeblockscode/CpunEngine/bin/Release/Objs/chevrolet.obj";
    vk_mesh meshLoader;
    std::vector<Mesh> testmeshes = meshLoader.load_from_obj(objFile);
    //upload_mesh(_triangleMesh);
    for(int i =0; i<testmeshes.size(); i++)
    {
    upload_mesh(testmeshes[i]);
    cout<<testmeshes[i].name<<endl;
    mesh_names.push_back(testmeshes[i].name);
    _meshes[testmeshes[i].name] = testmeshes[i];
    }

}

void vk_engine::load_images()
{

//    MTL_Loader mtl_loader;
//    vector<MTL> materials = mtl_loader.readFile("sponza.mtl");

//    for(int i =0; i < materials.size(); i++)
//    {
    Texture tex;

	vkutil::load_image_from_file(*this, "E:/codeblockscode/shade3D/bin/Debug/white.png", tex.image);

	VkImageViewCreateInfo imageinfo = vkinit::imageview_create_info(VK_FORMAT_R8G8B8A8_SRGB, tex.image._image, VK_IMAGE_ASPECT_COLOR_BIT);
	vkCreateImageView(_device, &imageinfo, nullptr, &tex.imageView);

	//_loadedTextures["empire_diffuse"] = lostEmpire;

    _loadTextures["white"] = tex;
  //}


}
void vk_engine::upload_mesh(Mesh& mesh)
{

    const size_t bufferSize = mesh._vertices.size() * sizeof(Vertex);
    //allocate vertex buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;
    //total size in bytes of buffer
    bufferInfo.size = bufferSize;
    //this buffers usage is for a vertex buffer
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    //VMA needs to know that this sata us CPU writeable but GPU readable
    VmaAllocationCreateInfo vmaAllocInfo = {};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    AllocatedBuffer stagingBuffer;

    //allocate the buffer
    VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaAllocInfo, &stagingBuffer._buffer, &stagingBuffer._allocation, nullptr));

    //copy vertex data into GPU readable data
    void *data;
    vmaMapMemory(_allocator, stagingBuffer._allocation, &data);

    memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(Vertex));

    //unmap the pointer to let the driver know that the write is finished
    vmaUnmapMemory(_allocator,stagingBuffer._allocation);

    VkBufferCreateInfo vertexBufferInfo = {};
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.pNext = nullptr;
    //total size in bytes of buffer
    vertexBufferInfo.size = bufferSize;
    //this buffers usage is for a vertex buffer
    vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    vmaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VK_CHECK(vmaCreateBuffer(_allocator, &vertexBufferInfo, &vmaAllocInfo, &mesh._vertexBuffer._buffer, &mesh._vertexBuffer._allocation, nullptr));

    immediate_submit([=](VkCommandBuffer cmd)
    {
    VkBufferCopy copy;
    copy.dstOffset = 0;
    copy.srcOffset=0;
    copy.size = bufferSize;
    vkCmdCopyBuffer(cmd, stagingBuffer._buffer, mesh._vertexBuffer._buffer,1, &copy);
    });

    _mainDeletionQueue.push_function([=]()
                                     {
                                       vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
                                     });
    vmaDestroyBuffer(_allocator, stagingBuffer._buffer, stagingBuffer._allocation);


}

Material* vk_engine::create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name)
{
    Material mat;
    mat.pipeline = pipeline;
    mat.pipelineLayout = layout;
    _materials[name] = mat;
    return &_materials[name];
}

Material* vk_engine::get_material(const std::string& name)
{
    auto it = _materials.find(name);
    if(it == _materials.end())
    {
        return nullptr;
    }
    else
    {
        return &(*it).second;
    }

}

Mesh* vk_engine::get_mesh(const std::string& name)
{

    auto it = _meshes.find(name);
    if(it == _meshes.end())
    {
        return nullptr;
    }
    else
    {
        return&(*it).second;
    }
}

void vk_engine::draw_objects(VkCommandBuffer cmdBuf, RenderObject* first, int count)
{
    Transform tr;
    Vector3D camPos = {0.f,-2.f,10.9f};
    //camPos = camPos + camMan;

    Matrix4D view = tr.translate(camPos, deltaY, deltaX, camMan);

    //camera projection
    Matrix4D projection = tr.perspective(70.f, 1700/900, 0.1f, 20000.0f);
    projection(1,1)*= -1;

    Matrix4D model = tr.rotate(Matrix4D(1.f), _frameNumber * 0.04f, Vector3D(1,0,0));



    CameraData camData;
    camData.projection = projection;
    camData.view = view;
    camData.viewproj = projection * view;


    void* data;
    vmaMapMemory(_allocator, get_current_frame().cameraBuffer._allocation, &data);

    void* objectData;
    vmaMapMemory(_allocator, get_current_frame().objectBuffer._allocation, &objectData);

    ObjectData* objectSSBO = (ObjectData*)objectData;

    memcpy(data, &camData, sizeof(CameraData));

    vmaUnmapMemory(_allocator, get_current_frame().cameraBuffer._allocation);

    float framed = (_frameNumber/120.f);

    _sceneParameters.ambientColour = {sin(framed),0,cos(framed),1};

    char* sceneData;
    vmaMapMemory(_allocator, _sceneParameterBuffer._allocation, (void**)&sceneData);

    int frameIndex = _frameNumber % FRAME_OVERLAP;

    sceneData += pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;

    memcpy(sceneData, &_sceneParameters, sizeof(GPUSceneData));

    vmaUnmapMemory(_allocator, _sceneParameterBuffer._allocation);

    vmaUnmapMemory(_allocator, get_current_frame().objectBuffer._allocation);

    Mesh* lastMesh = nullptr;
    Material* lastMaterial = nullptr;
    for(int i = 0; i< count ; i++)
    {

        RenderObject& object = first[i];
        objectSSBO[i].modelMatrix = object.transformMatrix;



        MeshPushConstants constants;
        //Matrix4D m = object.transformMatrix;
        //std::cout<<m(0,0)<<","<<m(1,0)<<","<<m(2,0)<<","<<m(3,0)<<"\n"<<m(0,1)<<","<<m(1,1)<<","<<m(2,1)<<","<<m(3,1)<<"\n"<<m(0,2)<<","<<m(1,2)<<","<<m(2,2)<<","<<m(3,2)<<"\n"<<","<<m(0,3)<<","<<m(1,3)<<","<<m(2,3)<<","<<m(3,3)<<std::endl;
        constants.mat = object.transformMatrix;

        //only bind to pipeline if its not the same as last
        if(object.material != lastMaterial)
        {
            vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
            lastMaterial = object.material;

            //bind descriptor set
            uint32_t uniform_offset = pad_uniform_buffer_size(sizeof(GPUSceneData)) * frameIndex;
            vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 0 , 1, &get_current_frame().globalDescriptor, 1, &uniform_offset);
            vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipelineLayout, 1 , 1, &get_current_frame().objectDescriptor, 0, nullptr);
        }

        //Matrix4D model = tr.rotate(Matrix4D(1.f), _frameNumber * 0.04f, Vector3D(1,0,0));;
        //final matrix calculation
        //Matrix4D MVP = projection * (object.transformMatrix*view);



        //upload to GPU using push constants
        vkCmdPushConstants(cmdBuf, object.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

        if(object.mesh != lastMesh)
        {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmdBuf, 0,1, &object.mesh->_vertexBuffer._buffer, &offset);
            lastMesh = object.mesh;
        }

        vkCmdDraw(cmdBuf, object.mesh->_vertices.size(), 1,0,0);
    }

}

void vk_engine::draw()
{

    Transform tr;
    //wait until the GPU has finished rendering the last frame
    //then have timeout of one second
    VK_CHECK(vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000));
    VK_CHECK(vkResetFences(_device, 1 , &get_current_frame()._renderFence));

    //request image from the swapchain
    // then one second timeout
    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, get_current_frame()._presentSemaphore, VK_NULL_HANDLE, &swapchainImageIndex));

    VK_CHECK(vkResetCommandBuffer(get_current_frame()._mainCommandBuffer, 0));

    VkCommandBuffer cmdBuf = get_current_frame()._mainCommandBuffer;

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;

    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cmdBuf, &cmdBeginInfo));

    // make clear colour from frame numver. Will flash with 120*pi frame period
    VkClearValue clearValue;
    float flash = abs(sin(_frameNumber/120.f));
    clearValue.color = {{0.0f, 0.0f, flash, 1.0f}};

    VkClearValue depthClear;
    depthClear.depthStencil.depth = 1.f;

    // start main renderpass
    // use clear colour and the framebuffer got from the index from the swapchain
    VkRenderPassBeginInfo rpInfo = {};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.pNext = nullptr;

    rpInfo.renderPass = _renderpass;
    rpInfo.renderArea.offset.x = 0;
    rpInfo.renderArea.offset.y = 0;
    rpInfo.renderArea.extent = _windowExtent;
    rpInfo.framebuffer = _framebuffers[swapchainImageIndex];


    //connect clear values
    rpInfo.clearValueCount = 2;
    VkClearValue clearValues[] = {clearValue, depthClear};
    rpInfo.pClearValues = &clearValues[0];
    vkCmdBeginRenderPass(cmdBuf, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

//***************************************START*************************************************
//    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _meshPipeline);
//    //std::cout<<"test"<<std::endl;
//    //vkCmdDraw(cmdBuf,3,1,0,0);
//
//    VkDeviceSize offset = 0;
//    vkCmdBindVertexBuffers(cmdBuf, 0,1, &_testMesh._vertexBuffer._buffer, &offset);
//
//    //make model view matrict
//    //camera position
//    Vector3D camPos = {0.f,-2.f,10.9f};
//
//    Matrix4D view = tr.translate(camPos, 0.f, 0.f);
//
//    //camera projection
//    Matrix4D projection = tr.perspective(70.f, 1700/900, 0.1f, 200.0f);
//    projection(1,1)*= -1;
//
//    Matrix4D model = tr.rotate(Matrix4D(1.f), _frameNumber * 0.04f, Vector3D(1,0,0));
//
//   Matrix4D mesh_matrix =  ((projection) * (model*view));
//   // Matrix4D mesh_matrix =  model * view * projection;
//    //std::cout<<mesh_matrix<<std::endl;
//
//    MeshPushConstants constants;
//    constants.mat = mesh_matrix;
//    //upload matrix to the GPU using push constants
//    vkCmdPushConstants(cmdBuf, _meshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0 , sizeof(MeshPushConstants), &constants);
//    vkCmdDraw(cmdBuf, _testMesh._vertices.size(),1,0,0);
//***************************************END****************************************************************

    draw_objects(cmdBuf, _renderables.data(), _renderables.size());

    vkCmdEndRenderPass(cmdBuf);
    VK_CHECK(vkEndCommandBuffer(cmdBuf));

    //prepare submission to queue

    // wait for semaphore to indicate when swapchain is ready
    // _renderSemaphore will be signalled when rendering finished

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = nullptr;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit.pWaitDstStageMask = &waitStage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &get_current_frame()._presentSemaphore;

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &get_current_frame()._renderSemaphore;

    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmdBuf;

    //submit command buffer to queue and execute
    // _renderFence will halt cpu until graphics commands have completed
    VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

    // this will draw the image to the window
    // wait for render semaphore before doing this
    // drawing commands must finish before displaying image

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;

    presentInfo.pSwapchains = &_swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapchainImageIndex;

    VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

    //increase number of frames
    _frameNumber++;

}

void vk_engine::run()
{
    SDL_Event e;
    bool exit = false;

    //main loop
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    SDL_CaptureMouse(SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    while(!exit)
    {
        //handle events in the queue
        while(SDL_PollEvent(&e) != 0)
        {

            if(e.type == SDL_QUIT) exit = true;
            if (e.type == SDL_MOUSEMOTION && e.motion.state & SDL_BUTTON_LMASK)
            {
                deltaX += (e.motion.xrel*_frameNumber) * -0.00001;
                deltaY += (e.motion.yrel*_frameNumber) * -0.00001;
            }
        }
        if (keystate[SDL_SCANCODE_W])
            {
                camMan.z -=0.0008 * _frameNumber;
            }
            if (keystate[SDL_SCANCODE_A])
            {
                camMan.x -=0.0004 * _frameNumber;
            }
            if (keystate[SDL_SCANCODE_S])
            {
                camMan.z +=0.0008 * _frameNumber;
            }
            if (keystate[SDL_SCANCODE_D])
            {
                camMan.x +=0.0004 * _frameNumber;
            }
            if (keystate[SDL_SCANCODE_Q])
            {
                camMan.y -=0.0008 * _frameNumber;
            }
            if (keystate[SDL_SCANCODE_E])
            {
                camMan.y +=0.0008 * _frameNumber;
            }

        draw();
    }
}

bool vk_engine::load_shader_module(const char* filePath, VkShaderModule* outShaderModule)
{

    //open file in binary mode (std::ios::binary)
    // and place cursor at the end (std::ios::ate)
    // cursor at end tells us how big the file is
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if(!file.is_open())
    {
       return false;
    }
    // location of cursor is file size in bytes
    size_t fileSize = (size_t)file.tellg();

    //spirv buffer must be uint32 and space must be reserved for the full file
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    //place cursor at beginning
    file.seekg(0);

    //load the file into the buffer
    file.read((char*)buffer.data(), fileSize);

    //close the file once it has been loaded into buffer
    file.close();

    // use buffer to create new shader module
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    //convert buffer size to bytes by multiplying the buffer size by the bytes size of uint32_t
    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();

    //check creation is successful
    // shader errors are common so VK_CHECK macro is helpful here
    VkShaderModule shaderModule;
    if(vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        return false;
    }

    *outShaderModule = shaderModule;
    return true;

}

void vk_engine::cleanup()
{
    if(_isInitialized)
    {
        vkDestroyCommandPool(_device, get_current_frame()._commandPool, nullptr);
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);

        //destroy main renderpass
        vkDestroyRenderPass(_device, _renderpass, nullptr);

        //destroy swapchain resources
        for(int i = 0; i < _framebuffers.size(); i++)
        {
            vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);

            vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
        }

        //destroy all swapchain resources
        for (int i = 0; i< _swapchainImageViews.size(); i++)
        {
            vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
        }

        vkWaitForFences(_device, 1, &get_current_frame()._renderFence, true, 1000000000);

        _mainDeletionQueue.flush();

        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyDevice(_device, nullptr);
        vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
        vkDestroyInstance(_instance, nullptr);
        SDL_DestroyWindow(_window);
    }
}

VkPipeline PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass pass)
{
    // create viewport state from viewport and scissor
    // only supporting one viewport and scissor
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;

    viewportState.viewportCount = 1;
    viewportState.pViewports = &_viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &_scissor;

    //no transparent objects yet so place holder colour blending will be used
    // blending type is "no blend" , colour attachment does get written to
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &_colorBlendAttachment;

    //build the pipeline
    //all of the info structs get used here
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = nullptr;

    pipelineInfo.stageCount = _shaderStages.size();
    pipelineInfo.pStages = _shaderStages.data();
    pipelineInfo.pVertexInputState = &_vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &_inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &_rasterizer;
    pipelineInfo.pMultisampleState = &_multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &_depthStencil;


    // vk_check is not sufficient to check for errors so we use this instead
    VkPipeline newPipeline;
    if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS)
    {
        std::cout << "failed to create pipeline\n";
                      return VK_NULL_HANDLE;
    }
    else
    {
        return newPipeline;
    }

}

AllocatedBuffer vk_engine::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = nullptr;

    bufferInfo.size = allocSize;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo = {};
    vmaAllocInfo.usage = memoryUsage;

    AllocatedBuffer newBuffer;

    //allocate the buffer
    VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaAllocInfo, &newBuffer._buffer, &newBuffer._allocation, nullptr));

    return newBuffer;
}

size_t vk_engine::pad_uniform_buffer_size(size_t originalSize)
{
    size_t minUboAlignment = _gpuProperties.limits.minUniformBufferOffsetAlignment;
    size_t alignedSize = originalSize;
    if(minUboAlignment > 0)
    {
        alignedSize = (alignedSize + minUboAlignment - 1 & ~(minUboAlignment -1));
    }
    return alignedSize;
}
