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

void Queue::WaitIdle() const
{
    assert(queue_);
    VK_CALL(vkQueueWaitIdle(queue_));
}

void Queue::Submit(const Span<CommandBuffer> &commandBuffers, const Span2<Semaphore> &waitSemaphores,
                   const Span2<Semaphore> &signalSemaphores, const Fence &signalFence) const
{
    SubmitExt(nullptr, commandBuffers, waitSemaphores, signalSemaphores, signalFence);
}

void Queue::SubmitExt(const void *pNext, const Span<CommandBuffer> &commandBuffers, const Span2<Semaphore> &waitSemaphores,
                      const Span2<Semaphore> &signalSemaphores, const Fence &signalFence) const
{
    assert(queue_ && commandBuffers);

    VkSemaphore *pWaitSemaphores = nullptr;
    VkPipelineStageFlags *pWaitDstStageMask = nullptr;
    const auto waitSemaphoreCount = waitSemaphores.Count();
    if (waitSemaphoreCount > 0)
    {
        pWaitSemaphores = static_cast<VkSemaphore*>(alloca(sizeof(VkSemaphore) * waitSemaphoreCount));
        waitSemaphores.Emplace(pWaitSemaphores);

        pWaitDstStageMask = static_cast<VkPipelineStageFlags*>(alloca(sizeof(VkPipelineStageFlags) * waitSemaphoreCount));
        for (uint32_t i = 0; i < waitSemaphoreCount; ++i)
        {
            pWaitDstStageMask[i] = waitSemaphores[i].GetPipeLineStageFlag();
        }
    }

    VkSemaphore *pSignalSemaphores = nullptr;
    const auto signalSemaphoreCount = signalSemaphores.Count();
    if (signalSemaphoreCount > 0)
    {
        pSignalSemaphores = static_cast<VkSemaphore*>(alloca(sizeof(VkSemaphore) * signalSemaphoreCount));
        signalSemaphores.Emplace(pSignalSemaphores);
    }

    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO, pNext, waitSemaphoreCount, pWaitSemaphores, pWaitDstStageMask,
                               commandBuffers.Count(), reinterpret_cast<const VkCommandBuffer*>(commandBuffers.Data()),
                               signalSemaphoreCount, pSignalSemaphores};

    auto vkSignalFence = VkFence(signalFence);

    VK_CALL(vkQueueSubmit(queue_, 1, &submitInfo, vkSignalFence));
}

VkResult Queue::Present(const Swapchain &swapchain, uint32_t imageIndex, const Span2<Semaphore> &waitSemaphores) const
{
    return PresentExt(nullptr, swapchain, imageIndex, waitSemaphores);
}

VkResult Queue::PresentExt(const void *pNext, const Swapchain &swapchain, uint32_t imageIndex, const Span2<Semaphore> &waitSemaphores) const
{
    assert(queue_ && swapchain);

    const auto waitSemaphoreCount = waitSemaphores.Count();
    VkSemaphore *pWaitSemaphores = nullptr;
    if (waitSemaphoreCount > 0)
    {
        pWaitSemaphores = static_cast<VkSemaphore*>(alloca(waitSemaphoreCount * sizeof(VkSemaphore)));
        waitSemaphores.Emplace(pWaitSemaphores);
    }

    auto vkSwapchain = VkSwapchainKHR(swapchain);

    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, pNext,
                                    waitSemaphoreCount, pWaitSemaphores,
                                    1, &vkSwapchain, &imageIndex, nullptr};

    auto result = vkQueuePresentKHR(queue_, &presentInfo);
    VK_CALL(result);

    return result;
}

std::vector<VkResult> Queue::Present(const Span2<Swapchain> &swapchains, const Span<uint32_t> &imageIndices, const Span2<Semaphore> &waitSemaphores) const
{
    return PresentExt(nullptr, swapchains, imageIndices, waitSemaphores);
}

std::vector<VkResult> Queue::PresentExt(const void *pNext, const Span2<Swapchain> &swapchains, const Span<uint32_t> &imageIndices, const Span2<Semaphore> &waitSemaphores) const
{
    assert(queue_ && swapchains && imageIndices);

    const auto waitSemaphoreCount = waitSemaphores.Count();
    VkSemaphore *pWaitSemaphores = nullptr;
    if (waitSemaphoreCount > 0)
    {
        pWaitSemaphores = static_cast<VkSemaphore*>(alloca(waitSemaphoreCount * sizeof(VkSemaphore)));
        waitSemaphores.Emplace(pWaitSemaphores);
    }

    const auto swapchainCount = swapchains.Count();
    auto pSwapchains = static_cast<VkSwapchainKHR*>(alloca(swapchainCount * sizeof(VkSwapchainKHR)));
    swapchains.Emplace(pSwapchains);

    std::vector<VkResult> results(swapchainCount);

    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, pNext,
                                    waitSemaphoreCount, pWaitSemaphores,
                                    swapchainCount, pSwapchains, imageIndices.Data(), results.data()};

    VK_CALL(vkQueuePresentKHR(queue_, &presentInfo));

    return results;
}

} // namespace vkw