#include "vk_mesh.h"
#include <cstddef>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

vertex_input_description vertex::get_vertex_input_description()
{
    vertex_input_description description;

    VkVertexInputBindingDescription main_binding = {};
    main_binding.binding = 0;
    main_binding.stride = sizeof(vertex);
    main_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    description.bindings.push_back(main_binding);

    VkVertexInputAttributeDescription pos_attr = {};
    pos_attr.location = 0;
    pos_attr.binding = 0;
    pos_attr.format = VK_FORMAT_R32G32B32_SFLOAT;
    pos_attr.offset = offsetof(vertex, pos);
    description.attributes.push_back(pos_attr);

    VkVertexInputAttributeDescription normal_attr = {};
    normal_attr.location = 1;
    normal_attr.binding = 0;
    normal_attr.format = VK_FORMAT_R32G32B32_SFLOAT;
    normal_attr.offset = offsetof(vertex, normal);
    description.attributes.push_back(normal_attr);

    VkVertexInputAttributeDescription color_attr = {};
    color_attr.location = 2;
    color_attr.binding = 0;
    color_attr.format = VK_FORMAT_R32G32B32_SFLOAT;
    color_attr.offset = offsetof(vertex, color);
    description.attributes.push_back(color_attr);

    return description;
}

bool mesh::load_from_gltf(const char *filename) { return true; }