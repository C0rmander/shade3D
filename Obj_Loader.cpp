#include "Obj_Loader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
using namespace std;

Obj_Loader::Obj_Loader()
{
}
vector<float> splitf(string str,char delim)
{
    vector<float> split_data;
    stringstream vertStream(str);
    string point_str;
    while(getline(vertStream, point_str, delim)){split_data.push_back(strtof(point_str.c_str(), NULL));}
    return split_data;
}
vector<string> split(string str,char delim)
{
    vector<string> split_data;
    stringstream vertStream(str);
    string point_str;
    while(getline(vertStream, point_str, delim)){split_data.push_back(point_str);}
    return split_data;
}
Face3D getFace(string faces, int pos)
{
vector<float> facelist;
vector<string> split_data = split(faces,' ');
for (int i = 0; i< split_data.size(); i++){facelist.push_back(splitf(split_data[i],'/')[pos]);}
return Face3D(facelist);
}
void Obj_Loader::loadFile(const char* OBJfile)
{
    string data;
    //Tex_Loader texloader;
    vector<std::string> temp_mtl;
    //vector<Texture2D> tex2d;
    ifstream infile;
    int offset = 0;
    infile.open(OBJfile);
    while(getline(infile, data))
    {
        if(data.substr(0,2) == "v "){vec3d.push_back(Vector3D(splitf(data.erase(0,2),' ')));continue;}
        //if(data.substr(0,2) == "vt"){tex2d.push_back(splitf(data.erase(0,3),' '));continue;}
        if(data.substr(0,2) == "vn"){norm3d.push_back(splitf(data.erase(0,3),' '));continue;}
        if(data.substr(0,2) == "vt"){uv3d.push_back(splitf(data.erase(0,3),' '));continue;}
//        if(data.substr(0,7) == "usemtl ")
//        {
//                materialNames.insert({f,data.erase(0,7)});
//                continue;
//        }
        if(data.substr(0,2) == "f ")
        {
            offset+=1;
            face3d.push_back(getFace(data.erase(0,2),0));
            texFaces.push_back(getFace(data.erase(0,2),1));
            normFaces.push_back(getFace(data.erase(0,2),2));
            continue;
        }
        if(data.substr(0,2) == "o ")
        {
            for (std::string i: temp_mtl)
                std::cout<<i<<std::endl;
            temp_mtl.clear();
            objects.push_back(OBJobject{data.erase(0,2), offset});
            continue;
        }
        if(data.substr(0,7) == "usemtl ")
        {
            temp_mtl.push_back(data.erase(0,7));
            continue;
        }
    }
    infile.close();
    //texloader.setTexCoords(tex2d);
    //texloader.setTexFaces(texFaces);
    //return vec3d;
}
vector<Vector3D> Obj_Loader::verts(){return vec3d;}
vector<Vector3D> Obj_Loader::normals(){return norm3d;}
vector<Face3D> Obj_Loader::normalsFaces(){return normFaces;}
vector<Face3D> Obj_Loader::faces(){return face3d;}
vector<Vector3D> Obj_Loader::uvs(){return uv3d;}
vector<OBJobject> Obj_Loader::shapes(){return objects;}
//map<int,string> Obj_Loader::getMaterialNames(){return materialNames;}
