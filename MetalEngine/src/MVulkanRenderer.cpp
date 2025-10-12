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

using namespace engine::vulkan;

static bool vulkan::IsExtensionAvailable(const vector<VkExtensionProperties>& properties, const char* extension)
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

static bool vulkan::IsDeviceSuitable(VkPhysicalDevice device)
{
	MetalQueueFamilyIndices indices = FindQueueFamiles(device);

	bool extensions_supported = CheckDeviceExtensionsSupport(device);

	bool swapchain_adequate = false;
	if (extensions_supported)
	{
		MetalSwapChainSupportDetails swapchain_support = QuerySwapChainSupport(device);
		swapchain_adequate = !swapchain_support.formats.empty() && !swapchain_support.present_modes.empty();
	}

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(device, &features);

	return indices.IsComplete() && extensions_supported && swapchain_adequate && features.samplerAnisotropy;
}

MetalQueueFamilyIndices vulkan::FindQueueFamiles(VkPhysicalDevice device)
{
	MetalQueueFamilyIndices indices;

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

MetalSwapChainSupportDetails vulkan::QuerySwapChainSupport(VkPhysicalDevice device)
{
	MetalSwapChainSupportDetails details;

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

static bool vulkan::CheckDeviceExtensionsSupport(VkPhysicalDevice device)
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

static void vulkan::VulkanRendererShutdown(void)
{
	/*Free memory*/
	vkFreeMemory(m_device, m_devicememory, nullptr);
	m_devicememory = VK_NULL_HANDLE;
}

static bool vulkan::VulkanInitRenderer()
{
	/*Checking if the memory type index is valid*/
	if (m_memorytypeindex == UINT64_MAX)
	{
		return false;
	}

	VkMemoryAllocateInfo mallocinfo = {};
	mallocinfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mallocinfo.allocationSize		= m_size;
	mallocinfo.memoryTypeIndex		= m_memorytypeindex;

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

static int vulkan::VulkanSetupRenderer(vector<const char*> instance_extension)
{
	VkApplicationInfo appinfo	= {};
	appinfo.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appinfo.pApplicationName	= "MetalEditor";
	appinfo.applicationVersion	= VK_MAKE_API_VERSION(0, 0, 1, 0);
	appinfo.pEngineName			= "MetalEngine";
	appinfo.engineVersion		= VK_MAKE_API_VERSION(0, 0, 1, 0);
	appinfo.apiVersion			= VK_API_VERSION_1_4;

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
	MetalQueueFamilyIndices indices = FindQueueFamiles(m_physicaldevice);
	vector<VkDeviceQueueCreateInfo> queue_create_information;
	set<VkUint32> unique_queue_families = { indices.graphics_family, indices.present_family };

	float queue_priority = 1.0f;
	for (VkUint32 queuefamily : unique_queue_families)
	{
		VkDeviceQueueCreateInfo queue_creation_info = {};
		queue_creation_info.sType					= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_creation_info.queueFamilyIndex		= queuefamily;
		queue_creation_info.queueCount				= 1;
		queue_creation_info.pQueuePriorities		= &queue_priority;
		queue_create_information.push_back(queue_creation_info);
	}

	VkPhysicalDeviceFeatures device_features	= {};
	device_features.samplerAnisotropy			= VK_TRUE;

	VkDeviceCreateInfo device_creation_info			= {};
	device_creation_info.sType						= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_creation_info.queueCreateInfoCount		= static_cast<VkUint32>(queue_create_information.size());
	device_creation_info.pQueueCreateInfos			= queue_create_information.data();
	device_creation_info.pEnabledFeatures			= &device_features;
	device_creation_info.enabledExtensionCount		= static_cast<VkUint32>(m_deviceextension.size());
	device_creation_info.ppEnabledExtensionNames	= m_deviceextension.data();

	VK_CHECK(vkCreateDevice(m_physicaldevice, &device_creation_info, nullptr, &m_device));

	vkGetDeviceQueue(m_device, indices.graphics_family, 0, &m_graphicsqueue);
	vkGetDeviceQueue(m_device, indices.present_family, 0, &m_presentqueue);

	/* Creating command pool*/
	MetalQueueFamilyIndices queue_family_indices = FindPhysicalQueueFamilies();

}

vector<char> MetalVulkanPipeline::ReadShaderFile(const string& filepath)
{
	ifstream file{filepath, ios::ate | ios::binary};
	
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
	auto VertexCode		= ReadShaderFile(vertexfilepath);
	auto FragmentCode	= ReadShaderFile(fragmentfilepath);
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