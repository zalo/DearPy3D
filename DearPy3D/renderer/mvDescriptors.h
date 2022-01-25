#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include "mvTypes.h"

// forward declarations
struct mvGraphics;
struct mvDescriptorSpec;
struct mvDescriptorSetLayout;
struct mvDescriptor;
struct mvDescriptorSet;
struct mvDescriptorManager;
struct mvBuffer;

// descriptors
mvDescriptor create_uniform_descriptor        (mvGraphics& graphics, mvDescriptorSpec spec, const std::string& name, u64 size, void* data);
mvDescriptor create_dynamic_uniform_descriptor(mvGraphics& graphics, mvDescriptorSpec spec, const std::string& name, u64 count, u64 size, void* data);
mvDescriptor create_texture_descriptor        (mvDescriptorSpec spec);

// descriptor specs
mvDescriptorSpec create_uniform_descriptor_spec        (u32 binding);
mvDescriptorSpec create_dynamic_uniform_descriptor_spec(u32 binding);
mvDescriptorSpec create_texture_descriptor_spec        (u32 binding);

// descriptor sets
mvDescriptorSet       create_descriptor_set       (mvGraphics& graphics, VkDescriptorSetLayout layout, VkPipelineLayout pipelineLayout);
void                  bind_descriptor_set         (mvGraphics& graphics, mvDescriptorSet& descriptorSet, u32 set, u32 dynamicDescriptorCount = 0u, u32* dynamicOffset = nullptr);
mvDescriptorSetLayout create_descriptor_set_layout(mvGraphics& graphics, std::vector<mvDescriptorSpec> specs);

mvDescriptorManager   create_descriptor_manager();
void                  cleanup_descriptor_manager    (mvGraphics& graphics, mvDescriptorManager& manager);
mvAssetID             register_descriptor_set       (mvDescriptorManager& manager, const std::string& tag, mvDescriptorSet pipeline);
mvAssetID             register_descriptor_set_layout(mvDescriptorManager& manager, const std::string& tag, VkDescriptorSetLayout layout);
VkDescriptorSet       get_descriptor_set            (mvDescriptorManager& manager, const std::string& tag, u32 index);
VkDescriptorSetLayout get_descriptor_set_layout     (mvDescriptorManager& manager, const std::string& tag);

struct mvDescriptorSpec
{
    VkDescriptorSetLayoutBinding layoutBinding;
};

struct mvDescriptorSetLayout
{
    std::vector<mvDescriptorSpec> specs;
    VkDescriptorSetLayout         layout = VK_NULL_HANDLE;
};

struct mvDescriptor
{
    std::vector<mvBuffer>  buffers;
    VkDeviceSize           size = 0u;
    mvDescriptorSpec       spec;
    VkWriteDescriptorSet   write;
};

struct mvDescriptorSet
{
    VkDescriptorSet*          descriptorSets;
    VkPipelineLayout          pipelineLayout;
    std::vector<mvDescriptor> descriptors;
};

struct mvDescriptorManager
{

    std::string* descriptorSetKeys = nullptr;
    std::string* layoutKeys = nullptr;

    // descriptor sets	       	  
    u32                   maxDSCount = 50u;
    u32                   dsCount = 0u;
    mvDescriptorSet*      descriptorSets = nullptr;

    // descriptor set layouts       	  
    u32                    maxLayoutCount = 50u;
    u32                    layoutCount = 0u;
    VkDescriptorSetLayout* layouts = nullptr;
};