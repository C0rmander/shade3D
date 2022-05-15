#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H
#include "Vector3D.h"
#include "Face3D.h"
#include <vector>
#include <map>
using namespace std;

struct OBJobject
{
    std::string name;
    int offset;
};
class Obj_Loader
{
    public:
        Obj_Loader();
        void loadFile(const char* OBJfile);
        vector<Face3D> texFaces;
        vector<struct Vector3D> verts();
        vector<struct Face3D> faces();
        vector<Face3D> normalsFaces();
        vector<struct Vector3D> normals();
        vector<Vector3D> uvs();
        vector<OBJobject> shapes();
       // map<int,string> getMaterialNames();

    private:
        vector <Face3D> face3d;
        vector <Vector3D> norm3d;
        map <int, string> materialNames;
        vector<Face3D> normFaces;
        vector<Vector3D> uv3d;
        vector <Vector3D> vec3d;
        vector<OBJobject> objects;
};

#endif // OBJ_LOADER_H
