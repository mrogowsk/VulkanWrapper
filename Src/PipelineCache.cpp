/*
Copyright(c) 2018 Marcus Rogowsky

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "VulkanWrapper.h"

#include "Error.h"

namespace vkw
{

std::vector<char> PipelineCache::GetData() const
{
    assert(pipelineCache_);

    size_t dataSize;
    VK_CALL(vkGetPipelineCacheData(pipelineCache_.GetCreator(), pipelineCache_, &dataSize, nullptr));

    if (dataSize == 0)
    {
        return {};
    }

    std::vector<char> data(dataSize);
    VK_CALL(vkGetPipelineCacheData(pipelineCache_.GetCreator(), pipelineCache_, &dataSize, data.data()));

    return data;
}

void PipelineCache::Merge(const Span2<PipelineCache> &caches) const
{
    assert(pipelineCache_ && caches);

    const auto cacheCount = caches.Count();
    VkPipelineCache* pCaches = static_cast<VkPipelineCache*>(alloca(sizeof(VkPipelineCache) * cacheCount));
    caches.Emplace(pCaches);

    VK_CALL(vkMergePipelineCaches(pipelineCache_.GetCreator(), pipelineCache_, cacheCount, pCaches));
}

} // namespace vkw