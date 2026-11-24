#include "Includes.hpp"
#include "VKWaterSimulation.h"
#include "pipeline/VKComputePipeline.h"
#include "prehistoric/application/Application.h"

namespace Prehistoric
{
    static void CheckWater(VkResult result)
    {
        if (result != VK_SUCCESS) throw std::runtime_error("Vulkan water compute setup failed: " + std::to_string(result));
    }

    struct VKWaterSimulation::Kernel
    {
        VkDevice device;
        VkDescriptorSetLayout bindings{};
        VkDescriptorPool pool{};
        VkDescriptorSet set{};
        VkPipelineLayout layout{};
        VkShaderModule module{};
        std::unique_ptr<VKComputePipeline> pipeline;

        ~Kernel()
        {
            pipeline.reset();
            vkDestroyShaderModule(device, module, nullptr);
            vkDestroyPipelineLayout(device, layout, nullptr);
            vkDestroyDescriptorPool(device, pool, nullptr);
            vkDestroyDescriptorSetLayout(device, bindings, nullptr);
        }
        template<typename T>
        void Dispatch(VkCommandBuffer cmd, const T& params, unsigned x, unsigned y)
        {
            pipeline->BindPipeline(cmd);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, layout, 0, 1, &set, 0, nullptr);
            vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(params), &params);
            vkCmdDispatch(cmd, x, y, 1);
            VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                0, 1, &barrier, 0, nullptr, 0, nullptr);
        }
    };

    std::unique_ptr<VKWaterSimulation::Kernel> VKWaterSimulation::MakeKernel(const char* name,
        std::initializer_list<VKTexture*> textures, unsigned storageCount)
    {
        auto kernel = std::make_unique<Kernel>();
        VkDevice device = kernel->device = gpu->getDevice();
        uint32_t count = static_cast<uint32_t>(textures.size());
        std::vector<VkDescriptorSetLayoutBinding> bindings(count);
        for (uint32_t i=0; i<count; ++i)
            bindings[i] = {i, i < storageCount ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
        VkDescriptorSetLayoutCreateInfo bindInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        bindInfo.bindingCount = count;
        bindInfo.pBindings = bindings.data();
        CheckWater(vkCreateDescriptorSetLayout(device, &bindInfo, nullptr, &kernel->bindings));
        VkDescriptorPoolSize sizes[] = {{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, storageCount},{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, count-storageCount}};
        VkDescriptorPoolCreateInfo pool{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        pool.maxSets = 1;
        pool.poolSizeCount = storageCount == count ? 1 : 2;
        pool.pPoolSizes = sizes;
        CheckWater(vkCreateDescriptorPool(device,&pool,nullptr,&kernel->pool));
        VkDescriptorSetAllocateInfo alloc{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        alloc.descriptorPool = kernel->pool;
        alloc.descriptorSetCount = 1;
        alloc.pSetLayouts = &kernel->bindings;
        CheckWater(vkAllocateDescriptorSets(device,&alloc,&kernel->set));
        uint32_t i = 0;
        for (auto* texture : textures)
        {
            bool sampled = i >= storageCount;
            VkDescriptorImageInfo image{sampled ? texture->getSampler() : VK_NULL_HANDLE, texture->getImage()->getImageView(),
                sampled && texture != outputs[0].get() && texture != initial[0].get() && texture != initial[1].get() ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL};
            VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            write.dstSet = kernel->set;
            write.dstBinding = i;
            write.descriptorCount = 1;
            write.descriptorType = bindings[i++].descriptorType;
            write.pImageInfo = &image;
            vkUpdateDescriptorSets(device,1,&write,0,nullptr);
        }
        VkPushConstantRange range{VK_SHADER_STAGE_COMPUTE_BIT,0,32};
        VkPipelineLayoutCreateInfo layout{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        layout.setLayoutCount = layout.pushConstantRangeCount = 1;
        layout.pSetLayouts = &kernel->bindings;
        layout.pPushConstantRanges = &range;
        CheckWater(vkCreatePipelineLayout(device,&layout,nullptr,&kernel->layout));
        auto code = ResourceLoader::LoadShaderVK(std::string("vulkan/water/")+name+".comp.spv");
        VkShaderModuleCreateInfo module{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        module.codeSize = code.size();
        module.pCode = reinterpret_cast<const uint32_t*>(code.data());
        CheckWater(vkCreateShaderModule(device,&module,nullptr,&kernel->module));
        VkPipelineShaderStageCreateInfo stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stage.module = kernel->module;
        stage.pName = "main";
        kernel->pipeline = std::make_unique<VKComputePipeline>(device,stage,kernel->layout);
        return kernel;
    }

    VKWaterSimulation::VKWaterSimulation(VKDevice* gpu, AssetManager* assets)
        : gpu(gpu), assets(assets), size(__WaterConfig.fftResolution)
    {
        if (size < 16 || (size & (size-1))) throw std::runtime_error("Water FFT resolution must be a power of two >= 16");
        for (int length=size; length>1; length/=2) ++stages;
        auto map = [&](int width) { return VKTexture::CreateStorageMap(gpu,width,size,VK_FORMAT_R32G32B32A32_SFLOAT); };
        for (auto& image : initial) image=map(size);
        for (auto& image : coefficients) image=map(size);
        for (auto& image : outputs) image=map(size);
        normal=map(size);
        scratch=map(size);
        twiddles=map(stages);
        for (unsigned i=0;i<noise.size();++i)
        {
            noise[i]=assets->loadTexture("engine://textures/noise/noise256_"+std::to_string(i)+".jpg").value();
            assets->addReference<Texture>(noise[i].handle);
        }
        spectrum=MakeKernel("h0k",{initial[0].get(),initial[1].get(),static_cast<VKTexture*>(noise[0].pointer),static_cast<VKTexture*>(noise[1].pointer),static_cast<VKTexture*>(noise[2].pointer),static_cast<VKTexture*>(noise[3].pointer)},2);
        factors=MakeKernel("twiddleFactors",{twiddles.get()},1);
        evolve=MakeKernel("hkt",{coefficients[0].get(),coefficients[1].get(),coefficients[2].get(),initial[0].get(),initial[1].get()},3);
        for (int axis=0;axis<3;++axis)
        {
            butterfly[axis]=MakeKernel("butterfly",{twiddles.get(),coefficients[axis].get(),scratch.get()},3);
            inversion[axis]=MakeKernel("inversion",{outputs[axis].get(),coefficients[axis].get(),scratch.get()},3);
        }
        normalPass=MakeKernel("normal",{normal.get(),outputs[0].get()},1);
    }

    VKWaterSimulation::~VKWaterSimulation()
    {
        vkDeviceWaitIdle(gpu->getDevice());
        for (auto texture : noise) assets->removeReference<Texture>(texture.handle);
    }

    void VKWaterSimulation::Record(VkCommandBuffer cmd,float seconds)
    {
        auto transition = [&](VKTexture* image,VkImageLayout before,VkImageLayout after)
        {
            VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            barrier.srcQueueFamilyIndex=barrier.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
            barrier.oldLayout=before;
            barrier.newLayout=after;
            barrier.srcAccessMask=before==VK_IMAGE_LAYOUT_UNDEFINED ? 0 : VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask=VK_ACCESS_SHADER_READ_BIT | (after==VK_IMAGE_LAYOUT_GENERAL ? VK_ACCESS_SHADER_WRITE_BIT : 0);
            barrier.image=image->getImage()->getImage();
            barrier.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};
            vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,0,0,nullptr,0,nullptr,1,&barrier);
        };
        const auto& cfg=__WaterConfig;
        unsigned groups=size/16;
        if (!initialized)
        {
            for (auto& image : initial) transition(image.get(),VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_GENERAL);
            for (auto& image : coefficients) transition(image.get(),VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_GENERAL);
            transition(scratch.get(),VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_GENERAL);
            transition(twiddles.get(),VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_GENERAL);
            struct {int n,l; float amplitude,intensity,windX,windY,suppression,alignment;} init{size,cfg.fftL,cfg.fftAmplitude,cfg.windSpeed,cfg.windDirection.x,cfg.windDirection.y,cfg.fftCapillarWavesSupression,cfg.alignment};
            spectrum->Dispatch(cmd,init,groups,groups);
            factors->Dispatch(cmd,size,stages,groups);
        }
        auto previous=initialized ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
        for (auto& image : outputs) transition(image.get(),previous,VK_IMAGE_LAYOUT_GENERAL);
        transition(normal.get(),previous,VK_IMAGE_LAYOUT_GENERAL);
        struct {int n,l; float t;} evolution{size,cfg.fftL,seconds*1000.0f*cfg.tDelta};
        evolve->Dispatch(cmd,evolution,groups,groups);
        for(int axis=0;axis<(cfg.choppy ? 3 : 1);++axis)
        {
            int ping=0;
            for(int dir=0;dir<2;++dir)
                for(int stage=0;stage<stages;++stage)
                {
                    int params[]={stage,ping,dir};
                    butterfly[axis]->Dispatch(cmd,params,groups,groups);
                    ping^=1;
                }
            int params[]={ping,size};
            inversion[axis]->Dispatch(cmd,params,groups,groups);
        }
        struct {int n;float strength;} normalParams{size,cfg.normalStrength};
        normalPass->Dispatch(cmd,normalParams,groups,groups);
        for (auto& image : outputs) transition(image.get(),VK_IMAGE_LAYOUT_GENERAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        transition(normal.get(),VK_IMAGE_LAYOUT_GENERAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        initialized=true;
    }
}

