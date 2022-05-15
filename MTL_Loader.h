#ifndef MTL_LOADER_H
#define MTL_LOADER_H
#include <vector>
#include "Vector3D.h"
#include "SDL.h"
#include <string>
#include <unordered_map>
#include "vk_engine.h"
#include "MTL.h"
class MTL_Loader
{
    public:
        MTL_Loader();
        std::vector <MTL> readFile(std::string MTLfile);
};

#endif // MTL_LOADER_H
