#ifndef TEX_LOADER_H
#define TEX_LOADER_H
//#include "Texture2D.h"
#include <string>
#include <vector>
#include "Face3D.h"
#include <SDL.h>
using namespace std;
class Tex_Loader
{
    public:
        Tex_Loader();
        SDL_Surface* getTex(string TEXfile);
       // void setTexCoords(vector<Texture2D>& texcoords);
       // vector<Texture2D> getTexCoords();
        void setTexFaces(vector<Face3D>& texfaces);
        vector<Face3D> getTexFaces();
        vector<SDL_Surface*> getMat(string Matfile, string mat);
};

#endif // TEX_LOADER_H
