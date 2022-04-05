#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H
#include "Vector3D.h"
#include "Face3D.h"
#include <vector>
#include <map>
using namespace std;
class Obj_Loader
{
    public:
        Obj_Loader();
        void loadFile(const char* OBJfile);
        vector<struct Vector3D> verts();
        vector<struct Face3D> faces();
        vector<Face3D> normalsFaces();
        vector<struct Vector3D> normals();
       // map<int,string> getMaterialNames();

    private:
        vector <Face3D> face3d;
        vector <Vector3D> norm3d;
        map <int, string> materialNames;
        vector<Face3D> normFaces;
        vector <Vector3D> vec3d;
};

#endif // OBJ_LOADER_H
