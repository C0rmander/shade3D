#include "vk_engine.h"
#include <SDL_vulkan.h>
int main(int argc, char* args[])
{
    vk_engine engine;
    engine.init();
    engine.run();
    engine.cleanup();
    return 0;
}
