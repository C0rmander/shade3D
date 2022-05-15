#include "vk_mesh.h"
#include <iostream>
#include "Obj_Loader.h"

vk_mesh::vk_mesh()
{
    //ctor
}

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

    VkVertexInputAttributeDescription uvAttribute = {};
    positionAttribute.binding = 0;
    positionAttribute.location = 3;
    positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    positionAttribute.offset = offsetof(Vertex, uv);

    description.attributes.push_back(positionAttribute);
    description.attributes.push_back(normalAttribute);
    description.attributes.push_back(colourAttribute);
    description.attributes.push_back(uvAttribute);

    return description;
}

std::vector<Mesh> vk_mesh::load_from_obj(const char* filename)
{

    Obj_Loader loadOBJ;
    loadOBJ.loadFile(filename);
    std::vector<Face3D> faces = loadOBJ.faces();
    std::vector <Vector3D> vertices = loadOBJ.verts();
    std::vector <Face3D> normalFaces = loadOBJ.normalsFaces();
    std::vector <Vector3D> normals = loadOBJ.normals();
    std::vector <OBJobject> objects = loadOBJ.shapes();
    std::vector <Mesh> meshes;
    std::cout<<"the vertices size is:  "<<vertices.size()<<std::endl;
    std::cout<<"the normalFaces size is:  "<<normalFaces.size()<<std::endl;
    std::cout<<"the faces size is:  "<<faces.size()<<std::endl;
    for (int o = 0; o < objects.size(); o++)
    {
        Mesh mesh;
        mesh.name = objects[o].name;
        //cout<<objects[o].offset<<endl;
        int offsetStop = 0;
        if(o < objects.size()-1)
        {
            offsetStop = objects[o+1].offset;
        }
        else
        {
            offsetStop = faces.size();
        }
       // cout<<mesh.name<<": ";
        for(int i = objects[o].offset; i < offsetStop; i++)
        {
            Vector3D vec1, vec2, vec3;
            Vertex ObjMesh;
            vec1 = vertices[faces[i].f1-1];
            vec2 = vertices[faces[i].f2-1];
            vec3 = vertices[faces[i].f3-1];


            Vector3D norm1 = normals[normalFaces[i].f1-1];
            Vector3D norm2 = normals[normalFaces[i].f2-1];
            Vector3D norm3 = normals[normalFaces[i].f3-1];


            ObjMesh.position.x = vec1.x;
            ObjMesh.position.y = vec1.y;
            ObjMesh.position.z = vec1.z;

            ObjMesh.normal.x = norm1.x;
            ObjMesh.normal.y = norm1.y;
            ObjMesh.normal.z = norm1.z;
            ObjMesh.colour = ObjMesh.normal;

            mesh._vertices.push_back(ObjMesh);

            ObjMesh.position.x = vec2.x;
            ObjMesh.position.y = vec2.y;
            ObjMesh.position.z = vec2.z;

            ObjMesh.normal.x = norm2.x;
            ObjMesh.normal.y = norm2.y;
            ObjMesh.normal.z = norm2.z;

            ObjMesh.colour = ObjMesh.normal;

            mesh._vertices.push_back(ObjMesh);

            ObjMesh.position.x = vec3.x;
            ObjMesh.position.y = vec3.y;
            ObjMesh.position.z = vec3.z;

            ObjMesh.normal.x = norm3.x;
            ObjMesh.normal.y = norm3.y;
            ObjMesh.normal.z = norm3.z;

            ObjMesh.colour = ObjMesh.normal;

            mesh._vertices.push_back(ObjMesh);

        }

        meshes.push_back(mesh);
       // cout<<"vertices size    "<<meshes[0]._vertices.size()<<endl;

    }


    return meshes;

}
