// ------------------------------------------------------
//
//	ANDREW CONNER SKATZES (c) 2025
//
//	MIT License
// 
//	Description:
//		Engine Vulkan renderer functions and definitions
// ------------------------------------------------------

#include "headers/MVulkanRenderer.hpp"
#include "headers/MError.h"

namespace engine::vulkan
{
	static bool IsExtensionAvailable(const vector<VkExtensionProperties>& properties, const char* extension)
	{
		for (const VkExtensionProperties& p : properties)
		{
			if (strcmp(p.extensionName, extension) == 0)
			{
				return true;
			}
		}

		return false;
	}

	static bool IsDeviceSuitable(VkPhysicalDevice device)
	{
		MetalVulkanQueueFamilyIndices indices = FindQueueFamiles(device);

		bool extensions_supported = CheckDeviceExtensionsSupport(device);

		bool swapchain_adequate = false;
		if (extensions_supported)
		{
			MetalVulkanSwapChainSupportDetails swapchain_support = QuerySwapChainSupport(device);
			swapchain_adequate = !swapchain_support.formats.empty() && !swapchain_support.present_modes.empty();
		}

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);

		return indices.IsComplete() && extensions_supported && swapchain_adequate && features.samplerAnisotropy;
	}

	MetalVulkanQueueFamilyIndices FindQueueFamiles(VkPhysicalDevice device)
	{
		MetalVulkanQueueFamilyIndices indices;

		VkUint32 queuefamilycount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamilycount, nullptr);

		vector<VkQueueFamilyProperties> queuefamilies(queuefamilycount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queuefamilycount, queuefamilies.data());

		int i = 0;
		for (const auto& queuefamily : queuefamilies)
		{
			if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphics_family = i;
				indices.does_graphics_has_value = true;
			}

			VkBool32 presentsupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentsupport);
			if (queuefamily.queueCount > 0 && presentsupport)
			{
				indices.present_family = i;
				indices.does_present_family_has_value = true;
			}

			if (indices.IsComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	MetalVulkanSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device)
	{
		MetalVulkanSwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

		VkUint32 formatcount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatcount, nullptr);

		if (formatcount != 0)
		{
			details.formats.resize(formatcount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatcount, details.formats.data());
		}

		VkUint32 presentmodecount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentmodecount, nullptr);

		if (presentmodecount != 0)
		{
			details.present_modes.resize(presentmodecount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentmodecount, details.present_modes.data());
		}

		return details;
	}

	static bool CheckDeviceExtensionsSupport(VkPhysicalDevice device)
	{
		VkUint32 extensioncount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensioncount, nullptr);

		vector<VkExtensionProperties> available_extensions(extensioncount);
		VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensioncount, available_extensions.data()));

		set<string> required_extensions(m_instance_extensions.begin(), m_instance_extensions.end());

		for (const auto& extensions : available_extensions)
		{
			required_extensions.erase(extensions.extensionName);
		}

		return required_extensions.empty();
	}

	static void VulkanRendererShutdown(void)
	{
		/*Free memory*/
		vkFreeMemory(m_device, m_devicememory, nullptr);
		m_devicememory = VK_NULL_HANDLE;
	}

	static bool VulkanInitRenderer()
	{
		/*Checking if the memory type index is valid*/
		if (m_memorytypeindex == UINT64_MAX)
		{
			return false;
		}

		VkMemoryAllocateInfo mallocinfo = {};
		mallocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mallocinfo.allocationSize = m_size;
		mallocinfo.memoryTypeIndex = m_memorytypeindex;

		/*Reserving a chunk of GPU memory*/
		VK_CHECK(vkAllocateMemory(m_device, &mallocinfo, nullptr, &m_devicememory));

		/*Checking if the device's memory hasn't been allocated, if so we have an error*/
		if (m_devicememory == VK_NULL_HANDLE)
		{
			return false;
		}

		/*Checking if the GPU's memory is visible to the CPU's memory, so the CPU can write directly to it*/
		if (IsHostVisible())
		{
			VK_CHECK(vkMapMemory(m_device, m_devicememory, 0, m_size, 0, (void**)&m_data));
		}

		m_head = new VkChunk();
		m_head->id = m_nextblockid++;
		m_head->size = m_size;
		m_head->offset = 0;
		m_head->previous = nullptr;
		m_head->next = nullptr;
		m_head->type = VAT_FREE;

		return true;
	}

	static int VulkanSetupRenderer(vector<const char*> instance_extension)
	{
		VkApplicationInfo appinfo = {};
		appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appinfo.pApplicationName = "MetalEditor";
		appinfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
		appinfo.pEngineName = "MetalEngine";
		appinfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
		appinfo.apiVersion = VK_API_VERSION_1_4;

		/* Creating Vulkan instance */
		VkInstanceCreateInfo creationinfo = {};
		creationinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		creationinfo.pApplicationInfo = &appinfo;

		/* Getting SDL3 extensions for Vulkan */
		VkUint32 properties_count;
		vector<VkExtensionProperties> properties;
		vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
		properties.resize(properties_count);
		VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.data()));	// Error checking :)

		if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
		{
			instance_extension.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}

		/* Continuing to create Vulkan instance */
		creationinfo.enabledExtensionCount = (VkUint32)instance_extension.size();
		creationinfo.ppEnabledExtensionNames = instance_extension.data();
		VK_CHECK(vkCreateInstance(&creationinfo, m_allocator, &m_instance));

		/* Selecting Physical Device (GPU) */
		VkUint32 devicecount = 0;
		vkEnumeratePhysicalDevices(m_instance, &devicecount, nullptr);
		if (devicecount == 0)
		{
			FatalError("Vulkan Physical Device ERROR", "Failed to find a Vulkan compatible GPU(s)");
			return 1;
		}
		fmt::print("Device count: {}\n", devicecount);
		vector<VkPhysicalDevice> devices(devicecount);
		VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &devicecount, devices.data()));

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_physicaldevice = device;
				break;
			}
		}

		if (m_physicaldevice == VK_NULL_HANDLE)
		{
			FatalError("Vulkan Physical Device ERROR", "Failed to find a suitable GPU(s)");
			return 1;
		}

		vkGetPhysicalDeviceProperties(m_physicaldevice, &m_properties);
		fmt::print("Physical Device: {}\n", m_properties.deviceName);

		/* Creating logical device*/
		MetalVulkanQueueFamilyIndices indices = FindQueueFamiles(m_physicaldevice);
		vector<VkDeviceQueueCreateInfo> queue_create_information;
		set<VkUint32> unique_queue_families = { indices.graphics_family, indices.present_family };

		float queue_priority = 1.0f;
		for (VkUint32 queuefamily : unique_queue_families)
		{
			VkDeviceQueueCreateInfo queue_creation_info = {};
			queue_creation_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_creation_info.queueFamilyIndex = queuefamily;
			queue_creation_info.queueCount = 1;
			queue_creation_info.pQueuePriorities = &queue_priority;
			queue_create_information.push_back(queue_creation_info);
		}

		VkPhysicalDeviceFeatures device_features = {};
		device_features.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo device_creation_info = {};
		device_creation_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_creation_info.queueCreateInfoCount = static_cast<VkUint32>(queue_create_information.size());
		device_creation_info.pQueueCreateInfos = queue_create_information.data();
		device_creation_info.pEnabledFeatures = &device_features;
		device_creation_info.enabledExtensionCount = static_cast<VkUint32>(m_deviceextension.size());
		device_creation_info.ppEnabledExtensionNames = m_deviceextension.data();

		VK_CHECK(vkCreateDevice(m_physicaldevice, &device_creation_info, nullptr, &m_device));

		vkGetDeviceQueue(m_device, indices.graphics_family, 0, &m_graphicsqueue);
		vkGetDeviceQueue(m_device, indices.present_family, 0, &m_presentqueue);

		/* Creating command pool*/
		MetalVulkanQueueFamilyIndices queue_family_indices = FindPhysicalQueueFamilies();

		VkCommandPoolCreateInfo poolinfo = {};
		poolinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolinfo.queueFamilyIndex = queue_family_indices.graphics_family;
		poolinfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK(vkCreateCommandPool(m_device, &poolinfo, nullptr, &m_commandpool));
	}

	MetalVulkanSwapchain::MetalVulkanSwapchain()
	{

	}

	MetalVulkanSwapchain::~MetalVulkanSwapchain()
	{
		for (auto imageview : m_swapchain_image_views)
		{
			vkDestroyImageView(m_device, imageview, nullptr);
		}
		m_swapchain_image_views.clear();

		if (m_swapchain != nullptr)
		{
			vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
			m_swapchain = nullptr;
		}

		for (int i = 0; i < m_depthimages.size(); i++)
		{
			vkDestroyImageView(m_device, m_depthimage_views[i], nullptr);
			vkDestroyImage(m_device, m_depthimages[i], nullptr);
			vkFreeMemory(m_device, m_depthimages_memories[i], nullptr);
		}

		for (auto framebuffer : m_swapchain_framebuffers)
		{
			vkDestroyFramebuffer(m_device, framebuffer, nullptr);
		}

		vkDestroyRenderPass(m_device, m_renderpass, nullptr);

		for (VkUsize i = 0; i < MAXIMUM_FRAMES_IN_FLIGHTS; i++)
		{
			vkDestroySemaphore(m_device, m_render_finished_semaphores[i], nullptr);
			vkDestroySemaphore(m_device, m_image_available_semaphores[i], nullptr);
			vkDestroyFence(m_device, m_in_flight_fences[i], nullptr);
		}
	}


	void VulkanCreateSwapchain()
	{
		MetalVulkanSwapChainSupportDetails SwapchainSupport = GetSwapchainSupport();
		VkSurfaceFormatKHR SurfaceFormat = ChooseSwapSurfaceFormat(SwapchainSupport.formats);
		VkPresentModeKHR PresentMode = ChooseSwapPresentMode(SwapchainSupport.present_modes);
		VkExtent2D Extent = ChooseSwapExtent(SwapchainSupport.capabilities);

		VkUint32 ImageCount = SwapchainSupport.capabilities.minImageCount + 1;
		if (SwapchainSupport.capabilities.maxImageCount > 0 && ImageCount > SwapchainSupport.capabilities.maxImageCount)
		{
			ImageCount = SwapchainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR CreationInfo = {};
		CreationInfo.sType				= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		CreationInfo.surface			= m_surface;
		CreationInfo.minImageCount		= ImageCount;
		CreationInfo.imageFormat		= SurfaceFormat.format;
		CreationInfo.imageColorSpace	= SurfaceFormat.colorSpace;
		CreationInfo.imageExtent		= Extent;
		CreationInfo.imageArrayLayers	= 1;
		CreationInfo.imageUsage			= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		MetalVulkanQueueFamilyIndices Indices = FindPhysicalQueueFamilies();
		VkUint32 QueueFamilyIndices[] = { Indices.graphics_family, Indices.present_family };

		if (Indices.graphics_family != Indices.present_family)
		{
			CreationInfo.imageSharingMode		= VK_SHARING_MODE_CONCURRENT;
			CreationInfo.queueFamilyIndexCount	= 2;
			CreationInfo.pQueueFamilyIndices	= QueueFamilyIndices;
		}
		else
		{
			CreationInfo.imageSharingMode		= VK_SHARING_MODE_EXCLUSIVE;
			CreationInfo.queueFamilyIndexCount	= 0;
			CreationInfo.pQueueFamilyIndices	= nullptr;
		}

		CreationInfo.preTransform	= SwapchainSupport.capabilities.currentTransform;
		CreationInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		CreationInfo.presentMode	= PresentMode;
		CreationInfo.clipped		= VK_TRUE;
		CreationInfo.oldSwapchain	= VK_NULL_HANDLE;

		VK_CHECK(vkCreateSwapchainKHR(m_device, &CreationInfo, nullptr, &m_swapchain));

		vkGetSwapchainImagesKHR(m_device, m_swapchain, &ImageCount, nullptr);
		m_swapchain_images.resize(ImageCount);
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &ImageCount, m_swapchain_images.data());
	}

	void VulkanCreateImageViews()
	{
		m_swapchain_image_views.resize(m_swapchain_images.size());
		for (VkUsize i = 0; i < m_swapchain_images.size(); i++)
		{
			VkImageViewCreateInfo CreationInfo{};
			CreationInfo.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			CreationInfo.image								= m_swapchain_images[i];
			CreationInfo.viewType							= VK_IMAGE_VIEW_TYPE_2D;
			CreationInfo.format								= m_swapchain_image_format;
			CreationInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			CreationInfo.subresourceRange.baseMipLevel		= 0;
			CreationInfo.subresourceRange.levelCount		= 1;
			CreationInfo.subresourceRange.baseArrayLayer	= 0;
			CreationInfo.subresourceRange.layerCount		= 1;

			VK_CHECK(vkCreateImageView(m_device, &CreationInfo, nullptr, &m_swapchain_image_views[i]));
		}
	}

	void VulkanCreateDepthResources()
	{
		VkFormat DepthFormat = FindDepthFormat();
		VkExtent2D SwapchainExtent = m_swapchain_extent;

		m_depthimages.resize(m_swapchain_images.size());
		m_depthimages_memories.resize(m_swapchain_images.size());
		m_depthimage_views.resize(m_swapchain_images.size());

		for (VkUint32 i = 0; i < m_depthimages.size(); i++)
		{
			VkImageCreateInfo CreationInfo{};
			CreationInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			CreationInfo.imageType		= VK_IMAGE_TYPE_2D;
			CreationInfo.extent.width	= m_swapchain_extent.width;
			CreationInfo.extent.height	= m_swapchain_extent.width;
			CreationInfo.extent.depth	= 1;
			CreationInfo.mipLevels		= 1;
			CreationInfo.arrayLayers	= 1;
			CreationInfo.format			= DepthFormat;
			CreationInfo.tiling			= VK_IMAGE_TILING_OPTIMAL;
			CreationInfo.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
			CreationInfo.usage			= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			CreationInfo.samples		= VK_SAMPLE_COUNT_1_BIT;
			CreationInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;
			CreationInfo.flags			= 0;

			CreateImageWithInfo(CreationInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthimages[i], m_depthimages_memories[i]);

			/* It's french for the view -> La Vue*/
			VkImageViewCreateInfo LaVueInfo{};
			LaVueInfo.sType		= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			LaVueInfo.image		= m_depthimages[i];
			LaVueInfo.viewType	= VK_IMAGE_VIEW_TYPE_2D;
			LaVueInfo.format	= DepthFormat;

			LaVueInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_DEPTH_BIT;
			LaVueInfo.subresourceRange.baseMipLevel		= 0;
			LaVueInfo.subresourceRange.levelCount		= 1;
			LaVueInfo.subresourceRange.baseArrayLayer	= 0;
			LaVueInfo.subresourceRange.layerCount		= 1;

			VK_CHECK(vkCreateImageView(m_device, &LaVueInfo, nullptr, &m_depthimage_views[i]));
		}
	}

	void VulkanCreateRenderPass()
	{
		VkAttachmentDescription DepthAttachment{};
		DepthAttachment.format			= FindDepthFormat();
		DepthAttachment.samples			= VK_SAMPLE_COUNT_1_BIT;
		DepthAttachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		DepthAttachment.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		DepthAttachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		DepthAttachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		DepthAttachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
		DepthAttachment.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference DepthRef{};
		DepthRef.attachment	= 1;
		DepthRef.layout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription ColorAttachment = {};
		ColorAttachment.format			= m_swapchain_image_format;
		ColorAttachment.samples			= VK_SAMPLE_COUNT_1_BIT;
		ColorAttachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		ColorAttachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
		ColorAttachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		ColorAttachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		ColorAttachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
		ColorAttachment.finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
		VkAttachmentReference ColorRef = {};
		ColorRef.attachment		= 0;
		ColorRef.layout			= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription Subpass	= {};
		Subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		Subpass.colorAttachmentCount	= 1;
		Subpass.pColorAttachments		= &ColorRef;
		Subpass.pDepthStencilAttachment = &DepthRef;

		/* ONLY PEOPLE IN THE MILITARY WILL GET THIS
			- Platoon
		*/
		VkSubpassDependency SubpassPlatoon = {};
		SubpassPlatoon.srcSubpass		= VK_SUBPASS_EXTERNAL; 
		SubpassPlatoon.srcAccessMask	= 0;
		SubpassPlatoon.srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		SubpassPlatoon.dstSubpass		= 0;
		SubpassPlatoon.dstAccessMask	= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		SubpassPlatoon.dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		array<VkAttachmentDescription, 2> Attachments = { ColorAttachment, DepthAttachment };
		VkRenderPassCreateInfo CreationInfo = {};
		CreationInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		CreationInfo.attachmentCount	= static_cast<VkUint32>(Attachments.size());
		CreationInfo.pAttachments		= Attachments.data();
		CreationInfo.subpassCount		= 1;
		CreationInfo.pSubpasses			= &Subpass;
		CreationInfo.dependencyCount	= 1;
		CreationInfo.pDependencies		= &SubpassPlatoon;

		VK_CHECK(vkCreateRenderPass(m_device, &CreationInfo, nullptr, &m_renderpass));
	}

	void VulkanCreateFramebuffers()
	{
		m_swapchain_framebuffers.resize(m_swapchain_image_format);
		for (VkUsize i = 0; i < m_swapchain_image_format; i++)
		{
			array<VkImageView, 2> Attachments = { m_swapchain_image_views[i], m_depthimage_views[i] };
			VkExtent2D SwapchainExtent = m_swapchain_extent;
			VkFramebufferCreateInfo CreationInfo = {};
			CreationInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			CreationInfo.renderPass = m_renderpass;
			CreationInfo.attachmentCount = static_cast<VkUint32>(Attachments.size());
			CreationInfo.pAttachments = Attachments.data();
			CreationInfo.width = m_swapchain_extent.width;
			CreationInfo.height = m_swapchain_extent.height;
			CreationInfo.layers = 1;
			VK_CHECK(vkCreateFramebuffer(m_device, &CreationInfo, nullptr, &m_swapchain_framebuffers[i]));
		}
	}

	void VulkanCreateSyncObjects()
	{
		m_image_available_semaphores.resize(MAXIMUM_FRAMES_IN_FLIGHTS);
		m_render_finished_semaphores.resize(MAXIMUM_FRAMES_IN_FLIGHTS);
		m_in_flight_fences.resize(MAXIMUM_FRAMES_IN_FLIGHTS);
		m_images_in_flight.resize(m_swapchain_images.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo SemaphoreInfo = {};
		SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
		VkFenceCreateInfo FenceInfo = {};
		FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (VkUsize i = 0; i < MAXIMUM_FRAMES_IN_FLIGHTS; i++)
		{
			if (vkCreateSemaphore(m_device, &SemaphoreInfo, nullptr, &m_image_available_semaphores[i]) != VK_SUCCESS
				|| vkCreateSemaphore(m_device, &SemaphoreInfo, nullptr, &m_render_finished_semaphores[i]) != VK_SUCCESS
				|| vkCreateFence(m_device, &FenceInfo, nullptr, &m_in_flight_fences[i]) != VK_SUCCESS)
			{
				fmt::print("ENGINE: VULKAN Failed to create synchronization objects for a frame");
			}
		}
	}

	void CreateImageWithInfo(const VkImageCreateInfo& imageinfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imagememory)
	{
		VK_CHECK(vkCreateImage(m_device, &imageinfo, nullptr, &image));

		VkMemoryRequirements memoryrequiem;
		vkGetImageMemoryRequirements(m_device, image, &memoryrequiem);

		VkMemoryAllocateInfo mallocinfo = {};
		mallocinfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mallocinfo.allocationSize = memoryrequiem.size;
		mallocinfo.memoryTypeIndex = FindMemoryType(memoryrequiem.memoryTypeBits, properties);
		
		VK_CHECK(vkAllocateMemory(m_device, &mallocinfo, nullptr, &imagememory));
		VK_CHECK(vkBindImageMemory(m_device, image, imagememory, 0));
	}

	VkFormat FindSupportedFormat(const vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(m_physicaldevice, format, &properties);

			if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		FatalError("Vulkan ERROR", "Failed to find supported format!");
	}

	VkUint32 FindMemoryType(VkUint32 typefilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memoryprops;

		vkGetPhysicalDeviceMemoryProperties(m_physicaldevice, &memoryprops);
		for (VkUint32 i = 0; i < memoryprops.memoryTypeCount; i++)
		{
			if ((typefilter & (1 << i)) && (memoryprops.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		/* If we failed doing this then we most halt due to illegal instructor or something like this*/
		FatalError("Vulkan ERROR", "Failed to find suitable memory type!");
	}

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& usableformats)
	{
		for (const auto& uformat : usableformats)
		{
			if (uformat.format == VK_FORMAT_B8G8R8A8_UNORM && uformat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return uformat;
			}
		}

		return usableformats[0];
	}

	VkPresentModeKHR ChooseSwapPresentMode(const vector<VkPresentModeKHR>& usable_present_modes)
	{
		for (const auto& upmode : usable_present_modes)
		{
			if (upmode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				/* Cout is too much for something so simple as this*/
				fmt::print("ENGINE: Vulkan present mode = Mailbox\n");
				return upmode;
			}
		}

		fmt::print("ENGINE: Vulkan present mode = V-Sync\n");
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
		/* Got to do that illegal sin again!*/
		VkUint32 requiem = (numeric_limits<VkUint32>::max)();	/* I AM THE DARK MESSIAH... or I mostly call myself that*/
		if (capabilities.currentExtent.width != requiem)
		{
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D ForetoldExtent = m_window_extent;
			ForetoldExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, ForetoldExtent.width));
			ForetoldExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, ForetoldExtent.height));
			return ForetoldExtent;
		}
	}

	VkResult AcquireNextImage(VkUint32* imageindex)
	{
		/* We have a little macros causing major problems so we're going to commit a sin in order to get this to work*/
		VkUint32 requiem = (numeric_limits<VkUint32>::max)(); /* The name is ironic!!*/
		vkWaitForFences(m_device, 1, &m_in_flight_fences[CurrentFrame], VK_TRUE, requiem);

		VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, requiem, m_image_available_semaphores[CurrentFrame], VK_NULL_HANDLE, imageindex);

		return result;
	}

	VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, VkUint32* imageindex)
	{
		if (m_images_in_flight[*imageindex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(m_device, 1, &m_images_in_flight[*imageindex], VK_TRUE, UINT64_MAX);
		}

		m_images_in_flight[*imageindex] = m_in_flight_fences[CurrentFrame];

		VkSubmitInfo SubmitInfo = {};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore RequiemSemphores[] = { m_image_available_semaphores[CurrentFrame] };
		VkPipelineStageFlags RequiemStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		SubmitInfo.waitSemaphoreCount = 1; /* We're not waiting 2 Requiems*/
		SubmitInfo.pWaitSemaphores = RequiemSemphores;
		SubmitInfo.pWaitDstStageMask = RequiemStages;
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = buffers;

		VkSemaphore SignalSemaphores[] = { m_render_finished_semaphores[CurrentFrame] };
		SubmitInfo.signalSemaphoreCount = 1;
		SubmitInfo.pSignalSemaphores = SignalSemaphores;

		vkResetFences(m_device, 1, &m_in_flight_fences[CurrentFrame]);
		VK_CHECK(vkQueueSubmit(m_graphicsqueue, 1, &SubmitInfo, m_in_flight_fences[CurrentFrame]));

		VkSwapchainKHR Swapchains[] = { m_swapchain };
		VkPresentInfoKHR PresentInfo = {};
		PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		PresentInfo.waitSemaphoreCount = 1;
		PresentInfo.pWaitSemaphores = SignalSemaphores;
		PresentInfo.swapchainCount = 1;
		PresentInfo.pSwapchains = Swapchains;
		PresentInfo.pImageIndices = imageindex;

		auto result = vkQueuePresentKHR(m_presentqueue, &PresentInfo);

		/* This black magic adds 1 to the current frame index
			but if divides only if current frame index equals 2 to wrap back to zero
			just in cause you don't know what '%' means... it's just the remainder after division
		*/
		CurrentFrame = (CurrentFrame + 1) % MAXIMUM_FRAMES_IN_FLIGHTS;
	}

	MetalVulkanPipeline::MetalVulkanPipeline(const string& vertexfilepath, const string& fragmentfilepath)
	{

	}

	MetalVulkanPipeline::~MetalVulkanPipeline()
	{
		vkDestroyShaderModule(m_device, VertexShaderModule, nullptr);
		vkDestroyShaderModule(m_device, FragmentShaderModule, nullptr);
		vkDestroyPipeline(m_device, m_graphicspipeline, nullptr);
	}

	vector<char> MetalVulkanPipeline::ReadShaderFile(const string& filepath)
	{
		ifstream file{ filepath, ios::ate | ios::binary };

		if (!file.is_open())
		{
			FatalError("Vulkan Pipeline ERROR", "Failed to open shader file: %s", filepath.c_str());
			return EMPTY;
		}

		usize filesize = static_cast<usize>(file.tellg());
		vector<char> buffer(filesize);

		file.seekg(0);
		file.read(buffer.data(), filesize);

		file.close();
		return buffer;
	}

	void MetalVulkanPipeline::CreateGraphicsPipeline(const string& vertexfilepath, const string& fragmentfilepath)
	{
		auto VertexCode = ReadShaderFile(vertexfilepath);
		auto FragmentCode = ReadShaderFile(fragmentfilepath);

		CreateShaderModule(VertexCode, &VertexShaderModule);
		CreateShaderModule(FragmentCode, &FragmentShaderModule);

		VkPipelineShaderStageCreateInfo shader_stages[2];

		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader_stages[0].module = VertexShaderModule;
		shader_stages[0].pName = "main";
		shader_stages[0].flags = 0;
		shader_stages[0].pNext = nullptr;
		shader_stages[0].pSpecializationInfo = nullptr;

		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stages[1].module = FragmentShaderModule;
		shader_stages[1].pName = "main";
		shader_stages[1].flags = 0;
		shader_stages[1].pNext = nullptr;
		shader_stages[1].pSpecializationInfo = nullptr;

		VkPipelineVertexInputStateCreateInfo VertexInputInfo{};
		VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VertexInputInfo.vertexAttributeDescriptionCount = 0;
		VertexInputInfo.vertexBindingDescriptionCount = 0;
		VertexInputInfo.pVertexAttributeDescriptions = nullptr;
		VertexInputInfo.pVertexBindingDescriptions = nullptr;

		VkGraphicsPipelineCreateInfo PipelineInfo{};
		PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		PipelineInfo.stageCount = 2;
		PipelineInfo.pStages = shader_stages;
		PipelineInfo.pVertexInputState = &VertexInputInfo;
		PipelineInfo.pInputAssemblyState = &MetalPipelineInfo.inputassemblyinfo;
		PipelineInfo.pViewportState = &MetalPipelineInfo.viewportinfo;
		PipelineInfo.pRasterizationState = &MetalPipelineInfo.rasterizationinfo;
		PipelineInfo.pMultisampleState = &MetalPipelineInfo.multisampleinfo;
		PipelineInfo.pColorBlendState = &MetalPipelineInfo.colorblendinfo;
		PipelineInfo.pDepthStencilState = &MetalPipelineInfo.depth_stencil_info;
		PipelineInfo.pDynamicState = nullptr;
		PipelineInfo.layout = MetalPipelineInfo.pipeline_layout;
		PipelineInfo.renderPass = MetalPipelineInfo.renderpass;
		PipelineInfo.subpass = MetalPipelineInfo.subpass;
		PipelineInfo.basePipelineIndex = -1;
		PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VK_CHECK(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &m_graphicspipeline));
	}

	MetalVulkanPipelineConfigInfo MetalVulkanPipeline::DefaultPipelineConfigInfo(VkUint32 width, VkUint32 height)
	{
		MetalVulkanPipelineConfigInfo config_info = {};

		/*
			These lines of code are to configure the input assembly stage of the Vulkan Graphics Pipeline.
			The stage will tell Vulkan how to interpret and assemble the vertex data—that is provided by vertex buffers (and optionally an index buffer)
			into geometric primitives.

			A deeper explain in this case:
			- sType -> tells Vulkan to define this as an input assembly state structure
			- topology -> 'VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST' this tells Vulkan every 3 vertices to form a independent triangle
			- primitiveRestartEnable -> Disables primitive restart (only useful for stripping topologies)
		*/
		config_info.inputassemblyinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		config_info.inputassemblyinfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		config_info.inputassemblyinfo.primitiveRestartEnable = VK_FALSE;


		/*

			An explanation of these lines:
			- x -> Tells Vulkan to start drawing at the top of the screen
			- y -> Tells Vulkan to start drawing at the left of the screen
			- width -> width of the rendering area (casting a unsigned 32-bit integer to a float)
			- height -> height of the rendering area (casting a unsigned 32-bit integer to a float)
			- minDepth -> this defines the minimum Z depth value (a.k.a closest distance to a plane Vulkan is allowed to render)
			- maxDepth -> this defines the maximum Z depth value (a.k.a farthest distance to a plane Vulkan is allowed to render)
		*/
		config_info.viewport.x = 0.0f;
		config_info.viewport.y = 0.0f;
		config_info.viewport.width = static_cast<float>(width);
		config_info.viewport.height = static_cast<float>(height);
		config_info.viewport.minDepth = 0.0f;
		config_info.viewport.maxDepth = 1.0f;

		config_info.scissor.offset = { 0, 0 };
		config_info.scissor.extent = { width, height };

		config_info.viewportinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		config_info.viewportinfo.viewportCount = 1;
		config_info.viewportinfo.pViewports = &config_info.viewport;
		config_info.viewportinfo.scissorCount = 1;
		config_info.viewportinfo.pScissors = &config_info.scissor;

		config_info.rasterizationinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		config_info.rasterizationinfo.depthClampEnable = VK_FALSE;
		config_info.rasterizationinfo.rasterizerDiscardEnable = VK_FALSE;
		config_info.rasterizationinfo.polygonMode = VK_POLYGON_MODE_FILL;
		config_info.rasterizationinfo.lineWidth = 1.0f;
		config_info.rasterizationinfo.cullMode = VK_CULL_MODE_NONE;
		config_info.rasterizationinfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		config_info.rasterizationinfo.depthBiasEnable = VK_FALSE;
		config_info.rasterizationinfo.depthBiasConstantFactor = 0.0f;
		config_info.rasterizationinfo.depthBiasClamp = 0.0f;
		config_info.rasterizationinfo.depthBiasSlopeFactor = 0.0f;

		config_info.multisampleinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		config_info.multisampleinfo.sampleShadingEnable = VK_FALSE;
		config_info.multisampleinfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		config_info.multisampleinfo.minSampleShading = 1.0f;
		config_info.multisampleinfo.pSampleMask = nullptr;
		config_info.multisampleinfo.alphaToCoverageEnable = VK_FALSE;
		config_info.multisampleinfo.alphaToOneEnable = VK_FALSE;

		config_info.colorblend_attachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		config_info.colorblend_attachment.blendEnable = VK_FALSE;
		config_info.colorblend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		config_info.colorblend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		config_info.colorblend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		config_info.colorblend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		config_info.colorblend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		config_info.colorblend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		config_info.colorblendinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		config_info.colorblendinfo.logicOpEnable = VK_FALSE;
		config_info.colorblendinfo.logicOp = VK_LOGIC_OP_COPY;
		config_info.colorblendinfo.attachmentCount = 1;
		config_info.colorblendinfo.pAttachments = &config_info.colorblend_attachment;
		config_info.colorblendinfo.blendConstants[0] = 0.0f;
		config_info.colorblendinfo.blendConstants[1] = 0.0f;
		config_info.colorblendinfo.blendConstants[2] = 0.0f;
		config_info.colorblendinfo.blendConstants[3] = 0.0f;

		return config_info;
	}

	void MetalVulkanPipeline::CreateShaderModule(const vector<char>& code, VkShaderModule* shadermodule)
	{
		VkShaderModuleCreateInfo shader_creation_info = {};
		shader_creation_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_creation_info.codeSize = code.size();

		/* We're converting one pointer type to another without changing the numerical value of the pointer in question
			We don't want any bit-shuffling, no runtime conversion! We want the pointer bytes unchanged
			SPIR-V expects pCode to be a sequence of constant 32-bit words
		*/
		shader_creation_info.pCode = reinterpret_cast<const VkUint32*>(code.data());

		VK_CHECK(vkCreateShaderModule(m_device, &shader_creation_info, nullptr, shadermodule));


	}

	MetalVulkanWindow::~MetalVulkanWindow()
	{
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	SDL_Window* MetalVulkanWindow::GetWindow() const
	{
		return window;
	}

	SDL_WindowFlags MetalVulkanWindow::GetWindowFlags() const
	{
		return winflags;
	}

	void MetalVulkanWindow::SetWindow(SDL_Window* dest)
	{
		window = dest;
	}

	void MetalVulkanWindow::SetWindowsFlags(SDL_WindowFlags dest)
	{
		winflags = dest;
	}

	int MetalVulkanWindow::CreateSDLWindow(int height, int width, const char* windowtitle)
	{
		fmt::print("Initializing SDL3\n");
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			FatalError("SDL Initializion ERROR", "Failed to Initialize SDL3: %s", SDL_GetError());
			return 1;
		}

		main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
		winflags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
		window = SDL_CreateWindow(windowtitle, (int)(1280 * main_scale), (int)(720 * main_scale), winflags);

		if (window == nullptr)
		{
			FatalError("SDL Initializion ERROR", "SDL window initialzing error: %s", SDL_GetError());
			return 1;
		}

		SDL_GetWindowSize(window, &width, &height);
		SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

		/*Show us our masterpiece*/
		SDL_ShowWindow(window);
		return 0;
	}

}