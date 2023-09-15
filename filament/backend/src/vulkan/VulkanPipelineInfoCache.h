/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_BACKEND_VULKANPIPELINEINFOCACHE_H
#define TNT_FILAMENT_BACKEND_VULKANPIPELINEINFOCACHE_H

#include <list>
#include <vector>

#include <bluevk/BlueVK.h>

namespace filament::backend {

struct VulkanVertexBuffer;

// We use this class to cache pipeline metadata that can be computed out of performance-critical
// sections such as draw() or beginRenderPass()
class VulkanPipelineInfoCache {
public:
    struct VertexBufferInfo {
        VulkanPipelineCache::VertexArray varray = {};
        VkBuffer vkbuffers[MAX_VERTEX_ATTRIBUTE_COUNT] = {VK_NULL_HANDLE};
        VkDeviceSize offsets[MAX_VERTEX_ATTRIBUTE_COUNT] = {0};
        uint8_t attribToBufferIndex[MAX_VERTEX_ATTRIBUTE_COUNT] = {0};
    };

    VulkanPipelineInfoCache() {
        increaseCapacity();
    }

    ~VulkanPipelineInfoCache() {
        assert_invariant(mBuffers.size() == (mBacking.size() * CACHE_INCREMENT));
    }

    VertexBufferInfo* get() {
        if (mBuffers.empty()) {
            increaseCapacity();
        }
        auto ret = mBuffers.front();
        mBuffers.pop_front();
        return ret;
    }

    void release(VertexBufferInfo* buffer) {
        mBuffers.push_back(buffer);
    }

private:
    static constexpr size_t CACHE_INCREMENT = 10;

    void increaseCapacity() {
        mBacking.emplace_back(std::make_unique<VertexBufferInfoArray>());
        auto& array = *(*(mBacking.end() - 1));
        for (auto& info: array) {
            mBuffers.push_back(&info);
        }
    }

    using VertexBufferInfoArray = std::array<VertexBufferInfo, CACHE_INCREMENT>;
    std::list<VertexBufferInfo*> mBuffers;
    std::vector<std::unique_ptr<VertexBufferInfoArray>> mBacking;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANPIPELINEINFOCACHE_H
