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

void DescriptorPool::Reset(VkDescriptorPoolResetFlags flags) const
{
    assert(descriptorPool_);
    VK_CALL(vkResetDescriptorPool(descriptorPool_.GetCreator(), descriptorPool_, flags));
}

DescriptorSet DescriptorPool::AllocateDescriptorSet(const DescriptorSetLayout &setLayout) const
{
    return AllocateDescriptorSetExt(nullptr, setLayout);
}
DescriptorSet DescriptorPool::AllocateDescriptorSetExt(const void *pNext, const DescriptorSetLayout &setLayout) const
{
    assert(descriptorPool_ && setLayout);

    auto vkSetLayout = VkDescriptorSetLayout(setLayout);
    VkDescriptorSetAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, pNext, descriptorPool_, 1, &vkSetLayout};

    VkDescriptorSet descriptorSet;
    VK_CALL(vkAllocateDescriptorSets(descriptorPool_.GetCreator(), &allocateInfo, &descriptorSet));

    return DescriptorSet(descriptorSet);
}
std::vector<DescriptorSet> DescriptorPool::AllocateDescriptorSets(const Span2<DescriptorSetLayout> &setLayouts) const
{
    return AllocateDescriptorSetsExt(nullptr, setLayouts);
}
std::vector<DescriptorSet> DescriptorPool::AllocateDescriptorSetsExt(const void *pNext, const Span2<DescriptorSetLayout> &setLayouts) const
{
    assert(descriptorPool_ && setLayouts);

    const auto descriptorSetCount = setLayouts.Count();
    auto pSetLayouts = static_cast<VkDescriptorSetLayout*>(alloca(sizeof(VkDescriptorSetLayout) * descriptorSetCount));
    setLayouts.Emplace(pSetLayouts);

    VkDescriptorSetAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, pNext, descriptorPool_, descriptorSetCount, pSetLayouts};

    std::vector<DescriptorSet> descriptorSets(descriptorSetCount);
    VK_CALL(vkAllocateDescriptorSets(descriptorPool_.GetCreator(), &allocateInfo, reinterpret_cast<VkDescriptorSet*>(descriptorSets.data())));
    return descriptorSets;
}

void DescriptorPool::FreeDescriptorSets(const Span<DescriptorSet> &descriptorSets) const
{
    assert(descriptorPool_ && descriptorSets);
    vkFreeDescriptorSets(descriptorPool_.GetCreator(), descriptorPool_, descriptorSets.Count(), reinterpret_cast<const VkDescriptorSet*>(descriptorSets.Data()));
}

} // namespace vkw