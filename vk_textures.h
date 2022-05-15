#ifndef VK_TEXTURES_H
#define VK_TEXTURES_H
#pragma once

#include "vk_type.h"
#include "vk_engine.h"

namespace vkutil {

	bool load_image_from_file(vk_engine& engine, const char* file, AllocatedImage& outImage);

}

#endif // VK_TEXTURES_H
