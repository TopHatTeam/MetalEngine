// ------------------------------------------------------
//
//	ANDREW CONNER SKATZES (c) 2025
//
//	MIT License
// 
//	Description:
//		Engine Vulkan renderer functions and definitions
// ------------------------------------------------------

#pragma once

#include <stdio.h>
#include <fmt/std.h>
#include <vulkan/vulkan.hpp>
#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <string>
#include <vector>
#include <fstream>
#include <set>

#include "MTypes.hpp"

/*Defining things without the need for stdint.h*/

typedef unsigned char		VkUint8;		/* 8-bit unsigned integer specifically for Vulkan*/
typedef unsigned short      VkUint16;		/* 16-bit unsigned integer specifically for Vulkan*/
typedef unsigned int		VkUint32;		/* 32-bit unsigned integer specifically for Vulkan*/
typedef unsigned long long	VkUint64;		/* 64-bit unsigned integer specifically for Vulkan*/

#define VK_ERROR_STRING(x) case static_cast<int>(x): return #x

/*I don't why but C++ wants we to put inline here or it says UNDEFINED FUNCTION??
	I seriously don't fucking understand what the compiler is talking about
	It's clearly defined here!
*/
constexpr inline const char* VK_ErrorToString(VkResult result)								
{																
	switch (result)												
	{															
		VK_ERROR_STRING(VK_SUCCESS);							
		VK_ERROR_STRING(VK_NOT_READY);							
		VK_ERROR_STRING(VK_TIMEOUT);							
		VK_ERROR_STRING(VK_EVENT_SET);							
		VK_ERROR_STRING(VK_EVENT_RESET);						
		VK_ERROR_STRING(VK_INCOMPLETE);							
		VK_ERROR_STRING(VK_ERROR_OUT_OF_HOST_MEMORY);			
		VK_ERROR_STRING(VK_ERROR_OUT_OF_DEVICE_MEMORY);			
		VK_ERROR_STRING(VK_ERROR_INITIALIZATION_FAILED);		
		VK_ERROR_STRING(VK_ERROR_DEVICE_LOST);					
		VK_ERROR_STRING(VK_ERROR_MEMORY_MAP_FAILED);			
		VK_ERROR_STRING(VK_ERROR_LAYER_NOT_PRESENT);			
		VK_ERROR_STRING(VK_ERROR_EXTENSION_NOT_PRESENT);		
		VK_ERROR_STRING(VK_ERROR_FEATURE_NOT_PRESENT);			
		VK_ERROR_STRING(VK_ERROR_INCOMPATIBLE_DRIVER);			
		VK_ERROR_STRING(VK_ERROR_TOO_MANY_OBJECTS);				
		VK_ERROR_STRING(VK_ERROR_FORMAT_NOT_SUPPORTED);			
		VK_ERROR_STRING(VK_ERROR_SURFACE_LOST_KHR);				
		VK_ERROR_STRING(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);		
		VK_ERROR_STRING(VK_SUBOPTIMAL_KHR);						
		VK_ERROR_STRING(VK_ERROR_OUT_OF_DATE_KHR);				
		VK_ERROR_STRING(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);		
		VK_ERROR_STRING(VK_ERROR_VALIDATION_FAILED_EXT);		
		VK_ERROR_STRING(VK_ERROR_INVALID_SHADER_NV);			
	default:													
		return "UNKNOWN";										
	}															
}
	
#define VK_CHECK(x)												\
{																\
	VkResult ret = x;											\
	if (ret != VK_SUCCESS)										\
	{															\
		fmt::print("VK: {} - {}", VK_ErrorToString(ret), #x);	\
	}															\
}

#define VK_VALIDATE(x, msg)						\
{												\
	if (!(x))									\
	{											\
		fmt::print("VK: {} - {}", msg, #x);		\
	}											\
}

using std::string, std::vector, std::ifstream, std::ios, std::set;

namespace engine::vulkan
{
	enum VulkanMemoryUsage : VkUint8
	{
		VMU_GPU_ONLY	= 1,
		VMU_CPU_ONLY	= 2,
		VMU_CPU_TO_GPU	= 3,
		VMU_GPU_TO_CPU	= 4,
		VMU_UNDEFINED	= 0
	};

	enum VulkanAllocationType : VkUint8
	{
		VAT_FREE			= 1,
		VAT_BUFFER			= 2,
		VAT_IMAGE			= 3,
		VAT_IMAGE_LINEAR	= 4,
		VAT_IMAGE_OPTIMAL	= 5,
		VAT_UNDEFINED		= 0
	};

	struct VulkanAllocation
	{
		MetalVulkanBlock* block;
		VkUint32			id;
		VkDevice			devicememory;
		VkDeviceSize		offset;
		VkDeviceSize		size;
		Byte* data;
		VulkanAllocation() :
			block(nullptr),
			id(0),
			devicememory(VK_NULL_HANDLE),
			offset(0),
			size(0),
			data(nullptr) {
		}
	};

	struct VkChunk
	{
		VkUint32				id;
		VkDeviceSize			size;
		VkDeviceSize			offset;
		VkChunk*				previous;
		VkChunk*				next;
		VulkanAllocationType	type;
		VulkanMemoryUsage		usage;
		VkChunk() :
			id(0),
			size(0),
			offset(0),
			previous(nullptr),
			next(nullptr),
			type(VulkanAllocationType::VAT_UNDEFINED),
			usage(VulkanMemoryUsage::VMU_UNDEFINED) {
		}
	};

	struct MetalQueueFamilyIndices
	{
		VkUint32 graphics_family;
		VkUint32 present_family;
		bool does_graphics_has_value		= false;
		bool does_present_family_has_value	= false;
		bool IsComplete() { return does_graphics_has_value && does_present_family_has_value; }
	};

	struct MetalSwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		vector<VkSurfaceFormatKHR> formats;
		vector<VkPresentModeKHR> present_modes;
	};
	
	class MetalVulkanBlock;

	class MetalVulkanPipeline
	{
	public:
		MetalVulkanPipeline(const string& vertexfilepath, const string& fragmentfilepath);
		~MetalVulkanPipeline();
		static vector<char> ReadShaderFile(const string& filepath);
		void CreateGraphicsPipeline(const string& vertexfilepath, const string& fragmentfilepath);
	};

	class MetalVulkanWindow
	{
	public:
		~MetalVulkanWindow();

		/**
		* @brief Gets the SDL Window and returns it
		* @returns SDL_Window*
		*/
		SDL_Window* GetWindow() const;

		/**
		* @brief Gets the SDL Windows flags and returns it 
		* @returns SDL_WindowFlags
		*/
		SDL_WindowFlags GetWindowFlags() const;

		/**
		* @brief Sets the SDL Window
		* @param dest The pointer to the SDL_Window
		* @returns void
		*/
		void SetWindow(SDL_Window* dest);

		/**
		* @brief Sets the SDL Window flags
		* @param dest The SDL_WindowFlags
		* @returns void
		*/
		void SetWindowsFlags(SDL_WindowFlags dest);

		/**
		* @brief Creates the SDL window
		* @param height The height of the window
		* @param width The width of the window
		* @param windowtitle The title of the SDL window
		* @returns 0 if successed 1 if failure
		*/
		int CreateSDLWindow(int height, int width, const char* windowtitle);
	protected:
		SDL_Window* window;
		SDL_WindowFlags winflags;
	public:
		float main_scale;
	};


	inline VkAllocationCallbacks*		m_allocator			= nullptr;
	inline VkInstance					m_instance			= VK_NULL_HANDLE;
	inline VkPhysicalDevice				m_physicaldevice	= VK_NULL_HANDLE;
	inline VkDevice						m_device			= VK_NULL_HANDLE;
	inline VkDeviceMemory				m_devicememory		= VK_NULL_HANDLE;
	inline VkDeviceSize					m_size				= NULL;
	inline VkDeviceSize					m_allocated			= NULL;
	inline VkSurfaceKHR					m_surface			= VK_NULL_HANDLE;
	inline VkUint32						m_queuefamily		= (VkUint32)-1;
	inline VkUint32						m_memorytypeindex	= (VkUint32)-1;
	inline VkUint32						m_nextblockid		= (VkUint32)-1;
	inline VkUint32						m_extensioncount	= (VkUint32)-1;
	inline VkQueue						m_queue				= VK_NULL_HANDLE;
	inline VkPipelineCache				m_pipelinecache		= VK_NULL_HANDLE;
	inline VkDescriptorPool				m_descriptorpool	= VK_NULL_HANDLE;
	inline VkPhysicalDeviceProperties	m_properties;	
	const inline vector<const char*>	m_deviceextension	= {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	inline VkUint32						m_minimagecount		= 2;
	inline Byte*						m_data				= nullptr;
	inline VulkanMemoryUsage			m_usage;	/*I doubt you can really enforce safety on a enumerator*/
	inline VkChunk*						m_head				= nullptr;
	inline vector<const char*>			m_instance_extensions;
	inline VkQueue						m_graphicsqueue		= VK_NULL_HANDLE;
	inline VkQueue						m_presentqueue		= VK_NULL_HANDLE;

	static bool IsExtensionAvailable(const vector<VkExtensionProperties>& properties, const char* extension);

	static bool IsDeviceSuitable(VkPhysicalDevice device);

	MetalQueueFamilyIndices FindQueueFamiles(VkPhysicalDevice device);

	MetalSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

	MetalQueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamiles(m_physicaldevice); }

	static bool CheckDeviceExtensionsSupport(VkPhysicalDevice device);

	/**
	* @brief Cleans up and Shutdowns the Renderer for Vulkan (REQUIRES a Vulkan compatible GPU)
	* @returns void
	*/
	static void VulkanRendererShutdown(void);

	/**
	* @brief Setups up the Vulkan renderer (REQUIRES a Vulkan compatible GPU)
	* @returns void
	*/
	static int VulkanSetupRenderer(vector<const char*> instance_extension);

	/**
	* @brief Initializes the Vulkan renderer (REQUIRES a Vulkan compatible GPU)
	* @returns void
	*/
	static bool VulkanInitRenderer();

	static bool IsHostVisible() { return m_usage != VMU_GPU_ONLY; }

}