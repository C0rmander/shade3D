#pragma once

#include "vk_type.h"
#include <vector>
#include "Vector3D.h"

struct VertexInputDescription
{
  std::vector<VkVertexInputBindingDescription> bindings;
  std::vector<VkVertexInputAttributeDescription> attributes;
  VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex
{
    Vector3D position;
    Vector3D normal;
    Vector3D colour;

    static VertexInputDescription get_vertex_description();
};

struct Mesh
{
    std::vector<Vertex> _vertices;

    AllocatedBuffer _vertexBuffer;

    bool load_from_obj(const char* filename);
};


