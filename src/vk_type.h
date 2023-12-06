﻿#pragma once

#include "vk_mem_alloc.h"
#include <vulkan/vulkan.h>

struct allocated_buffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};