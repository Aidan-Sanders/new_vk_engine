#include "vk_mesh.h"

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <vector>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

using namespace tinygltf;

struct buffer_view {
    unsigned char *data;
    uint32_t stride;
    uint32_t count;
};

struct texture_view {
    uint32_t width;
    uint32_t height;
    std::vector<unsigned char> img;
};

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
    color_attr.offset = offsetof(vertex, texcoord);
    description.attributes.push_back(color_attr);

    return description;
}

buffer_view retreive_buffer(Model *model, Primitive *primitive,
                            uint32_t accessor_index = -1, const char *attr = nullptr)
{
    buffer_view buffer_view;
    Accessor *accessor;
    if (attr != nullptr) {
        auto attribute = primitive->attributes.find(attr);
        accessor = &model->accessors[attribute->second];
    } else
        accessor = &model->accessors[accessor_index];
    auto *bufferview = &model->bufferViews[accessor->bufferView];
    auto *buffer = &model->buffers[bufferview->buffer];
    buffer_view.data =
        buffer->data.data() + bufferview->byteOffset + accessor->byteOffset;

    auto componentType = accessor->componentType;
    switch (componentType) {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        buffer_view.stride = sizeof(uint8_t);
        break;
    case TINYGLTF_COMPONENT_TYPE_BYTE:
        buffer_view.stride = sizeof(int8_t);
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        buffer_view.stride = sizeof(uint16_t);
        break;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
        buffer_view.stride = sizeof(int16_t);
        break;
    case TINYGLTF_COMPONENT_TYPE_INT:
        buffer_view.stride = sizeof(int32_t);
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        buffer_view.stride = sizeof(uint32_t);
        break;
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
        buffer_view.stride = sizeof(float);
        break;
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
        buffer_view.stride = sizeof(double);
        break;
    default:
        break;
    }

    auto type = accessor->type;
    switch (type) {
    case TINYGLTF_TYPE_SCALAR:
        buffer_view.stride *= 1;
        break;
    case TINYGLTF_TYPE_VEC2:
        buffer_view.stride *= 2;
        break;
    case TINYGLTF_TYPE_VEC3:
        buffer_view.stride *= 3;
        break;
    case TINYGLTF_TYPE_VEC4:
        buffer_view.stride *= 4;
        break;
    case TINYGLTF_TYPE_MAT2:
        buffer_view.stride *= 4;
        break;
    case TINYGLTF_TYPE_MAT3:
        buffer_view.stride *= 9;
        break;
    case TINYGLTF_TYPE_MAT4:
        buffer_view.stride *= 16;
        break;
    }

    if (bufferview->byteStride != 0)
        buffer_view.stride = bufferview->byteStride;

    buffer_view.count = accessor->count;
    return buffer_view;
}

texture_view retreive_texture(Model *model, uint32_t material_index = -1)
{
    texture_view texture_view;
    auto *material = &model->materials[material_index];
    auto *base_color_texture = &material->pbrMetallicRoughness.baseColorTexture;
    if (base_color_texture->index != -1) {
        auto *texture = &model->textures[base_color_texture->index];
        auto *img = &model->images[texture->source];
        texture_view.width = img->width;
        texture_view.height = img->height;
        texture_view.img = img->image;
    }
    return texture_view;
}

std::vector<mesh> load_from_gltf(const char *filename, std::vector<node> &nodes)
{
    TinyGLTF loader;
    Model model;
    std::string err;
    std::string warn;
    std::vector<mesh> meshes;
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename);

    if (!warn.empty()) {
        std::cerr << "warn: " << warn.c_str() << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "err: " << err.c_str() << std::endl;
    }

    if (!ret) {
        std::cerr << "failed to parse gltf" << std::endl;
        return meshes;
    }

    for (auto n = model.nodes.cbegin(); n != model.nodes.cend(); ++n) {
        node node;
        node.name = n->name;
        node.mesh_id = n->mesh;
        node.children = n->children;

        glm::mat4 t = glm::translate(glm::mat4(1.f), glm::vec3(0.f));
        glm::mat4 r = glm::translate(glm::mat4(1.f), glm::vec3(0.f));
        glm::mat4 s = glm::scale(t, glm::vec3(1.f));

        if (n->translation.size() != 0)
            t = glm::translate(
                glm::mat4(1.f),
                glm::vec3(n->translation[0], n->translation[1], n->translation[2]));

        if (n->rotation.size() != 0)
            r = glm::toMat4(glm::quat(n->rotation[3], n->rotation[0], n->rotation[1],
                                      n->rotation[2]));

        if (n->scale.size() != 0)
            s = glm::scale(glm::mat4(1.f),
                           glm::vec3(n->scale[0], n->scale[1], n->scale[2]));

        node.transform_mat = t * r * s;

        if (n->matrix.size() != 0) {
            node.transform_mat =
                glm::mat4(n->matrix[0], n->matrix[1], n->matrix[2], n->matrix[3],
                          n->matrix[4], n->matrix[5], n->matrix[6], n->matrix[7],
                          n->matrix[8], n->matrix[9], n->matrix[10], n->matrix[11],
                          n->matrix[12], n->matrix[13], n->matrix[14], n->matrix[15]);
        }

        nodes.push_back(node);
    }

    for (auto m = model.meshes.cbegin(); m != model.meshes.cend(); ++m) {
        mesh mesh;
        auto primitive = m->primitives[0];

        /* POSITION */
        buffer_view pos = retreive_buffer(&model, &primitive, -1, "POSITION");
        mesh.vertices.resize(pos.count);
        unsigned char *data = pos.data;
        for (uint32_t i = 0; i < pos.count; ++i) {
            mesh.vertices[i].pos =
                glm::vec3(*(float *)data, *(float *)(data + sizeof(float)),
                          *(float *)(data + 2 * sizeof(float)));
            data += pos.stride;
        }

        /* NORMAL */
        buffer_view normal = retreive_buffer(&model, &primitive, -1, "NORMAL");
        data = normal.data;
        for (uint32_t i = 0; i < normal.count; ++i) {
            mesh.vertices[i].normal =
                glm::vec3(*(float *)data, *(float *)(data + sizeof(float)),
                          *(float *)(data + 2 * sizeof(float)));
            data += normal.stride;
        }

        /* TEXCROOD */
        buffer_view texcrood = retreive_buffer(&model, &primitive, -1, "TEXCOORD_0");
        data = texcrood.data;
        for (uint32_t i = 0; i < texcrood.count; ++i) {
            mesh.vertices[i].texcoord =
                glm::vec2(*(float *)data, *(float *)(data + sizeof(float)));
            data += texcrood.stride;
        }

        /* INDEX */
        if (primitive.indices != -1) {
            buffer_view index = retreive_buffer(&model, &primitive, primitive.indices);
            data = index.data;
            for (uint32_t i = 0; i < index.count; ++i) {
                mesh.indices.push_back(*(uint16_t *)data);
                data += index.stride;
            }
        }

        // /* TEXTURE */
        if (primitive.material != -1) {
            texture_view texture_view = retreive_texture(&model, primitive.material);
            mesh.texture = texture_view.img;
            mesh.texture_buffer.width = texture_view.width;
            mesh.texture_buffer.height = texture_view.height;
            mesh.texture_buffer.format = VK_FORMAT_R8G8B8A8_SRGB;
        }

        meshes.push_back(mesh);
    }

    std::cout << filename << " loaded" << std::endl;

    return meshes;
}
