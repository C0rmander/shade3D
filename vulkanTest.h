#ifndef VULKANTEST_H
#define VULKANTEST_H
#include <vulkan/vulkan.h>
namespace shade3D{
class vulkanTest
{
    public:
        vulkanTest();
        bool firstVulkanInst();
        bool loadExtern();
        bool globalLoader();
};
}
#endif // VULKANTEST_H
