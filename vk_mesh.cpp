#include "vk_mesh.h"
#include <iostream>
#include "Obj_Loader.h"
VertexInputDescription Vertex::get_vertex_description()
{
    VertexInputDescription description;

    //one vertex buffer binding with a per-vertex rate
    VkVertexInputBindingDescription mainBinding = {};
    mainBinding.binding = 0;
    mainBinding.stride = sizeof(Vertex);
    mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    description.bindings.push_back(mainBinding);

    //Position will be stored at Location 0
    VkVertexInputAttributeDescription positionAttribute = {};
    positionAttribute.binding = 0;
    positionAttribute.location = 0;
    positionAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    positionAttribute.offset = offsetof(Vertex, position);

    //Normal stored at Location 1
    VkVertexInputAttributeDescription normalAttribute = {};
    normalAttribute.binding = 0;
    normalAttribute.location = 1;
    normalAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    normalAttribute.offset = offsetof(Vertex, normal);

    //colour stored at position 2
    VkVertexInputAttributeDescription colourAttribute = {};
    colourAttribute.binding = 0;
    colourAttribute.location = 2;
    colourAttribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    colourAttribute.offset = offsetof(Vertex, colour);

    description.attributes.push_back(positionAttribute);
    description.attributes.push_back(normalAttribute);
    description.attributes.push_back(colourAttribute);

    return description;
}

bool Mesh::load_from_obj(const char* filename)
{
    Obj_Loader loadOBJ;
    loadOBJ.loadFile(filename);
    std::vector<Face3D> faces = loadOBJ.faces();
    std::vector <Vector3D> vertices = loadOBJ.verts();
    std::vector <Face3D> normalFaces = loadOBJ.normalsFaces();
    std::vector <Vector3D> normals = loadOBJ.normals();
    std::cout<<"the vertices size is:  "<<vertices.size()<<std::endl;
    std::cout<<"the normals size is:  "<<normals.size()<<std::endl;
    std::cout<<"the faces size *3 is:  "<<faces.size()*3<<std::endl;
    for(int i = 0; i < faces.size(); i++)
    {

        Vector3D vec1 = vertices[faces[i].f1-1];
        Vector3D vec2 = vertices[faces[i].f2-1];
        Vector3D vec3 = vertices[faces[i].f3-1];
       // std::cout<<i<<","<<vertices.size()<<std::endl;

        Vector3D norm1 = normals[normalFaces[i].f1-1];
        //std::cout<<faces[i].f1-1<<std::endl;
        Vector3D norm2 = normals[normalFaces[i].f2-1];
        Vector3D norm3 = normals[normalFaces[i].f3-1];
       // std::cout<<i<<","<<normals.size()<<std::endl;
        Vertex ObjMesh;

        ObjMesh.position.x = vec1.x;
        ObjMesh.position.y = vec1.y;
        ObjMesh.position.z = vec1.z;

        ObjMesh.normal.x = norm1.x;
        ObjMesh.normal.y = norm1.y;
        ObjMesh.normal.z = norm1.z;
        ObjMesh.colour = ObjMesh.normal;

        _vertices.push_back(ObjMesh);

        ObjMesh.position.x = vec2.x;
        ObjMesh.position.y = vec2.y;
        ObjMesh.position.z = vec2.z;

        ObjMesh.normal.x = norm2.x;
        ObjMesh.normal.y = norm2.y;
        ObjMesh.normal.z = norm2.z;

        ObjMesh.colour = ObjMesh.normal;

        _vertices.push_back(ObjMesh);

        ObjMesh.position.x = vec3.x;
        ObjMesh.position.y = vec3.y;
        ObjMesh.position.z = vec3.z;

        ObjMesh.normal.x = norm3.x;
        ObjMesh.normal.y = norm3.y;
        ObjMesh.normal.z = norm3.z;

        ObjMesh.colour = ObjMesh.normal;

        _vertices.push_back(ObjMesh);

    }


    return true;

}
