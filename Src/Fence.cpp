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

Fence::Fence(VkDevice device, VkFence fence)
    : fence_(device, fence)
{
    assert(device && fence);
}

VkResult Fence::Wait(uint64_t timeoutInNanoSeconds) const
{
    assert(fence_);
    VkFence vkFence = fence_;
    auto res = vkWaitForFences(fence_.GetCreator(), 1, &vkFence, true, timeoutInNanoSeconds);
    VK_CALL(res);
    return res;
}

void Fence::Reset() const
{
    assert(fence_);
    VkFence vkFence = fence_;
    VK_CALL(vkResetFences(fence_.GetCreator(), 1, &vkFence));
}

VkResult Fence::GetStatus() const
{
    assert(fence_);
    auto res = vkGetFenceStatus(fence_.GetCreator(), fence_);
    VK_CALL(res);
    return res;
}

} // namespace vkw