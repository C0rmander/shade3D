#include "Tex_Loader.h"
#include <string>
//#include "Texture2D.h"
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
vector<Face3D> texFaces;
//vector<Texture2D> texCoords;

using namespace std;
Tex_Loader::Tex_Loader()
{
    //ctor
}

SDL_Surface* Tex_Loader::getTex(string TEXfile)
{
    SDL_Surface* surface = IMG_Load( TEXfile.c_str());
    if (surface == NULL)
    {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
    }
    return surface;
}

//void Tex_Loader::setTexCoords(vector<Texture2D>& texcoords)
//{
//    texCoords = texcoords;
//}

void Tex_Loader::setTexFaces(vector<Face3D>& texfaces)
{
    texFaces.insert(texFaces.end(), texfaces.begin(), texfaces.end());
}
//vector<Texture2D> Tex_Loader::getTexCoords()
//{
//    return texCoords;
//}
vector<Face3D> Tex_Loader::getTexFaces()
{
    return texFaces;
}
