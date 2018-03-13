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

QueryPool::QueryPool(VkDevice device, VkQueryPool queryPool)
    : queryPool_(device, queryPool) {}

VkResult QueryPool::GetResults(uint32_t firstQuery, uint32_t queryCount, void *pData, size_t dataSize, VkDeviceSize stride, VkQueryResultFlags flags) const
{
    assert(queryPool_);
    auto res = vkGetQueryPoolResults(queryPool_.GetCreator(), queryPool_, firstQuery, queryCount, dataSize, pData, stride, flags);
    VK_CALL(res);
    return res;
}

VkResult QueryPool::GetResults(uint32_t firstQuery, uint32_t queryCount, uint32_t *pData, uint32_t elementCountPerResult, VkQueryResultFlags flags) const
{
    return GetResults(firstQuery, queryCount, pData, queryCount * sizeof(uint32_t) * elementCountPerResult, sizeof(uint32_t) * elementCountPerResult, flags);
}

VkResult QueryPool::GetResults(uint32_t firstQuery, uint32_t queryCount, uint64_t *pData, uint32_t elementCountPerResult, VkQueryResultFlags flags) const
{
    return GetResults(firstQuery, queryCount, pData, queryCount * sizeof(uint64_t) * elementCountPerResult, sizeof(uint64_t) * elementCountPerResult, flags & VK_QUERY_RESULT_64_BIT);
}

} // namespace vkw