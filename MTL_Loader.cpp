#include "MTL_Loader.h"
#include "Tex_Loader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
//#include "Texture2D.h"
//#include "MathsFunc.h"
#include "MTL.h"
#include "vk_initializers.h"
vector <string> matNames, matUrls;
using namespace std;


MTL_Loader::MTL_Loader()
{
}

vector<float> splitff(string str,char delim)
{
    vector<float> split_data;
    stringstream vertStream(str);
    string point_str;
    while(getline(vertStream, point_str, delim)){split_data.push_back(strtof(point_str.c_str(), NULL));}
    return split_data;
}

vector <MTL> MTL_Loader::readFile(string MTLfile)
{
    vector <MTL> Materials;
    MTL material = MTL();
    //MathsFunc math;
    string data;
    ifstream infile;
    infile.open(MTLfile.c_str());
    Tex_Loader texloader;
    if(infile)
    {
    while(getline(infile, data))
    {
        if(data.substr(0,7) == "newmtl ")
        {
            if(material.newMTL.size() != 0)
            {
                Materials.push_back(material);
                material = {};
            }
            material.newMTL = (data.erase(0,7));
            continue;
        }
        if(data.substr(0,3) == "Ka "){material.Ka = splitff(data.erase(0,3),' ');continue;}
        if(data.substr(0,3) == "Kd "){material.Kd = splitff(data.erase(0,3),' ');continue;}
        if(data.substr(0,3) == "Ks "){material.Ks = splitff(data.erase(0,3),' ');continue;}
        if(data.substr(0,7) == "map_Kd ")
        {
            data.erase(0,7);
            while(data.find("\\") != std::string::npos){data.replace(data.find("\\"),2,"/");}
            material.map_Kd = data.c_str();
        }
    }
    }
    if(material.newMTL.size() != 0)
    {
        Materials.push_back(material);
        material = {};
    }
    return Materials;
}
