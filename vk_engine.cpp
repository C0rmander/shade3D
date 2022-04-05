#include "vk_type.h"
#include "vk_engine.h"
#include <SDL.h>
#include <SDL_vulkan.h>
#include "vk_initializers.h"
#include <cmath>
#include "vk_type.h"
#include <windows.h>
#include <iostream>
#include <fstream>
// bootstrap library created by Charles-lunarg
//https://github.com/charles-lunarg/vk-bootstrap
#include "VkBootstrap.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "transform.h"
#include "Matrix4D.h"
#include "Vector3D.h"
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
    //load the vertex and fragment shaders
    init_pipelines();

    //load meshes
    load_meshes();
    // make sure everything worked correctly
    _isInitialized = true;

    init_sync_structures();
}

void vk_engine::init_vulkan()
{
    std::cout<<"initiating vulkan";
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
    .set_minimum_version(1,1)
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

    _mainDeletionQueue.push_function([=]() {
                                     vkDestroySwapchainKHR(_device, _swapchain, nullptr);
                                     });
}

void vk_engine::init_commands()
{
    std::cout<<"started creating command pool"<<std::endl;
    //create command pool to submit commands to the graphics queue
    VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool));

    //allocate the default command buffer that we will use for rendering
    VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_commandPool, 1);

    VK_CHECK((vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_mainCommandBuffer)));

    _mainDeletionQueue.push_function([=](){
                                     vkDestroyCommandPool(_device, _commandPool, nullptr);
                                     });
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

    // minimum amount of subpasses is 1
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colour_attachment_ref;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    //connect the coliur attachment to the info
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &colour_attachment;
    // create link between subpass and info
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

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
      fb_info.pAttachments = &_swapchainImageViews[i];
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

    VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_renderFence));

    _mainDeletionQueue.push_function([=](){
                                     vkDestroyFence(_device, _renderFence, nullptr);
                                     });

    // no flags needed for semaphores
    VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentSemaphore));
    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderSemaphore));

    _mainDeletionQueue.push_function([=](){
                                     vkDestroySemaphore(_device, _presentSemaphore, nullptr);
                                     vkDestroySemaphore(_device, _renderSemaphore, nullptr);
                                     });
}

void vk_engine::init_pipelines()
{
    VkShaderModule triangleFragShader;
    if(!load_shader_module("frag.spv", &triangleFragShader))
    {
        std::cout<<"Error building triangle fragment shader" << std::endl;
    }
    else
    {
        std::cout<<"triangle fragment shader loaded successfully" << std::endl;
    }
//    VkShaderModule triangleVertShader;
//    if(!load_shader_module("vert.spv", &triangleVertShader))
//    {
//        std::cout<<"Error building triangle vertex shader" << std::endl;
//    }
//    else
//    {
//        std::cout<<"triangle vertex shader loaded successfully" << std::endl;
//    }

    //VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::pipeline_layout_create_info();

    //VK_CHECK(vkCreatePipelineLayout(_device, &pipeline_layout_info, nullptr, &_trianglePipelineLayout));

    // build the stage-create-info for vertex and fragment stages.
    PipelineBuilder pipelineBuilder;

    //add the other shaders
    //pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, triangleVertShader));
    //pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));


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

    //use the triangle layout created
    //pipelineBuilder._pipelineLayout =  _trianglePipelineLayout;



    //build the pipeline
    //_trianglePipeline = pipelineBuilder.build_pipeline(_device, _renderpass);

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

    VK_CHECK(vkCreatePipelineLayout(_device, &mesh_pipeline_layout_info, nullptr, &_meshPipelineLayout));

    VkShaderModule meshVertShader;
    if(!load_shader_module("tri_mesh.vert.spv", &meshVertShader))
    {
        std::cout<<"Error when building the triangle vertex shader module" << std::endl;
    }
    else
    {
        std::cout<<"triangle vertex shader successfully loaded" << std::endl;
    }

    //add the other shaders
    pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));
    pipelineBuilder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));

    //create mesh pipelin layout
    pipelineBuilder._pipelineLayout = _meshPipelineLayout;
    //build the mesh triangle pipeline
    _meshPipeline = pipelineBuilder.build_pipeline(_device, _renderpass);

    //destroy all shader modules, outside of the queue
    vkDestroyShaderModule(_device, meshVertShader, nullptr);
    //vkDestroyShaderModule(_device, triangleVertShader, nullptr);
    vkDestroyShaderModule(_device, triangleFragShader, nullptr);

    _mainDeletionQueue.push_function([=]()
                                     {
                                         //destroy pipeline
                                         vkDestroyPipeline(_device, _trianglePipeline, nullptr);

                                         vkDestroyPipeline(_device, _meshPipeline, nullptr);

                                        //destroy the pipeline layout
                                         vkDestroyPipelineLayout(_device, _trianglePipelineLayout, nullptr);
                                         vkDestroyPipelineLayout(_device, _meshPipelineLayout, nullptr);



                                     });
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

    _testMesh.load_from_obj("E:/codeblockscode/CpunEngine/bin/Release/Objs/example.obj");

    //upload_mesh(_triangleMesh);
    upload_mesh(_testMesh);

}

void vk_engine::upload_mesh(Mesh& mesh)
{
    //allocate vertex buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    //total size in bytes of buffer
    bufferInfo.size = mesh._vertices.size() * sizeof(Vertex);
    //this buffers usage is for a vertex buffer
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    //VMA needs to know that this sata us CPU writeable but GPU readable
    VmaAllocationCreateInfo vmaAllocInfo = {};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    //allocate the buffer
    VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaAllocInfo, &mesh._vertexBuffer._buffer, &mesh._vertexBuffer._allocation, nullptr));

    //add triangle mesh deletion to the deletion queue
    _mainDeletionQueue.push_function([=]()
                                     {
                                       vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
                                     });

    //copy vertex data into GPU readable data
    void *data;
    vmaMapMemory(_allocator, mesh._vertexBuffer._allocation, &data);

    memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(Vertex));

    //unmap the pointer to let the driver know that the write is finished
    vmaUnmapMemory(_allocator,mesh._vertexBuffer._allocation);

}

void vk_engine::draw()
{

    Transform tr;
    //wait until the GPU has finished rendering the last frame
    //then have timeout of one second
    VK_CHECK(vkWaitForFences(_device, 1, &_renderFence, true, 1000000000));
    VK_CHECK(vkResetFences(_device, 1 , &_renderFence));

    //request image from the swapchain
    // then one second timeout
    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, _presentSemaphore, VK_NULL_HANDLE, &swapchainImageIndex));

    VK_CHECK(vkResetCommandBuffer(_mainCommandBuffer, 0));

    VkCommandBuffer cmdBuf = _mainCommandBuffer;

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
    rpInfo.clearValueCount = 1;
    rpInfo.pClearValues = &clearValue;
    vkCmdBeginRenderPass(cmdBuf, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _meshPipeline);
    //vkCmdDraw(cmdBuf,3,1,0,0);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmdBuf, 0,1, &_testMesh._vertexBuffer._buffer, &offset);

    //make model view matrict
    //camera position
    Vector3D camPos = {0.f, 0.f,-2.f};

    Matrix4D view = tr.translate(Matrix4D(1.f), camPos);

    //camera projection
    Matrix4D projection = tr.perspective(45.f, 1700/900, 1.f, 200.0f);
    projection(1,1)*= -1;

    Matrix4D model = tr.rotate(Matrix4D(1.f), _frameNumber * 0.04f, Vector3D(1,0,0));

    Matrix4D mesh_matrix =  projection * view * model;
    //std::cout<<mesh_matrix<<std::endl;

    MeshPushConstants constants;
    constants.mat = mesh_matrix;
    //upload matrix to the GPU using push constants
    vkCmdPushConstants(cmdBuf, _meshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0 , sizeof(MeshPushConstants), &constants);
    vkCmdDraw(cmdBuf, _testMesh._vertices.size(),1,0,0);

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
    submit.pWaitSemaphores = &_presentSemaphore;

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &_renderSemaphore;

    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmdBuf;

    //submit command buffer to queue and execute
    // _renderFence will halt cpu until graphics commands have completed
    VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, _renderFence));

    // this will draw the image to the window
    // wait for render semaphore before doing this
    // drawing commands must finish before displaying image

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;

    presentInfo.pSwapchains = &_swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &_renderSemaphore;
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
    while(!exit)
    {
        //handle events in the queue
        while(SDL_PollEvent(&e) != 0)
        {
            if(e.type == SDL_QUIT) exit = true;
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
        vkDestroyCommandPool(_device, _commandPool, nullptr);
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

        vkWaitForFences(_device, 1, &_renderFence, true, 1000000000);

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
