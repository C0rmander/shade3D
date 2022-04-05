#include "vulkanTest.h"
#include "vulkanFunctions.h"
#define VK_NO_PROTOTYPES
#include "vulkan/vulkan.h"
#include <iostream>
#include <windef.h>
#include <Windows.h>

namespace shade3D
{
vulkanTest::vulkanTest()
{

}

bool vulkanTest::firstVulkanInst()
{
    HMODULE vulkan_library;
    #if defined _WIN32
    vulkan_library = LoadLibrary( "vulkan-1.dll" );
    #elif defined __linux
    vulkan_library = dlopen( "libvulkan.so.1", RTLD_NOW );
    #endif
    if( vulkan_library == nullptr )
    {
    std::cout << "Could not connect with a Vulkan Runtime library." <<
    std::endl;
    return false;
    }
    return true;

}
bool vulkanTest::loadExtern()
{
    #ifdef _WIN32
        #define LoadProcAddress GetProcAddress
    #elif defined __linux
        #define LoadFunction dlsym
    #endif

    #define EXPORTED_VULKAN_FUNCTION( name ) \
    name = (PFN_##name)LoadFunction( vulkan_library, #name ); \
    if( name == nullptr ) { \
    std::cout << "Could not load exported Vulkan function named: " \
    name << std::endl; \
    return false; \

}
bool vulkanTest::globalLoader()
{
    #define GLOBAL_LEVEL_VULKAN_FUNCTION( name ) \
    name = (PFN_##name)vkGetInstanceProcAddr( nullptr, #name ); \
    if( name == nullptr ) { \
    std::cout << "Could not load global-level function named: " \
    #name << std::endl; \
    return false; \
    }
    #include "ListOfVulkanFunctions.inl"\
    return true;
}
}
