#include "vulkan.hh"

#include <vector>
#include <set>
#include <iostream>
#include <string_view>
#include <utility>
#include <algorithm>

using namespace vulkan;
using namespace err;
using namespace window;

static std::vector<const char*> const validation_layers = {
	"VK_LAYER_LUNARG_standard_validation",
};

static std::vector<char const*> const device_extensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

#ifdef NDEBUG
static const bool enable_validation = false;
#else
static const bool enable_validation = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT obj_type,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layer_prefix,
	const char* msg,
	void* user_data
) {
	(void)flags;
	(void)obj_type;
	(void)obj;
	(void)location;
	(void)code;
	(void)layer_prefix;
	(void)user_data;

	std::cerr << "validation layer: " << msg << std::endl;

	return VK_FALSE;
}

bool operator==(VkSurfaceFormatKHR lhs, const VkSurfaceFormatKHR rhs) {
	return lhs.format == rhs.format && lhs.colorSpace == rhs.colorSpace;
}

// FIXME: Break this up into sub-functions for each necessary part of the process
// FIXME: Determine exactly what is necessary in this function
err::Result<VulkanResources> VulkanResources::create(Window const& window) {
	using namespace std::literals;

	VulkanResources result = {};

	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "cae";
	app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	app_info.pEngineName = "cae UI";
	app_info.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	uint32_t extension_count = 0;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);

	std::vector<const char*> extensions(glfw_extensions, glfw_extensions + extension_count);
	if (enable_validation) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	create_info.enabledExtensionCount = extensions.size();
	create_info.ppEnabledExtensionNames = extensions.data();

	if (enable_validation) {
		uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		std::vector<VkLayerProperties> available_layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

		const size_t wanted_layers = validation_layers.size();
		size_t found_layers = 0;
		for (auto layer : validation_layers) {
			for (auto properties : available_layers) {
				if (strcmp(layer, properties.layerName) == 0) {
					found_layers++;
				}
			}
		}

		if (found_layers < wanted_layers) {
			return "Wanted validation layers could not be loaded"sv;
		}
	}

	create_info.enabledLayerCount = 0;
	if (enable_validation) {
		create_info.enabledLayerCount = (uint32_t)validation_layers.size();
		create_info.ppEnabledLayerNames = validation_layers.data();
	}

	if (vkCreateInstance(&create_info, nullptr, &result.instance) != VK_SUCCESS) {
		return "Failed to create vulkan instance"sv;
	}

	if (enable_validation) {
		VkDebugReportCallbackCreateInfoEXT debug_info = {};
		debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debug_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		debug_info.pfnCallback = &debug_callback;

		auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(
			result.instance,
			"vkCreateDebugReportCallbackEXT"
		);
		if (func == nullptr) {
			return "Failed to load callback creation func address"sv;
		}
		if (func(result.instance, &debug_info, nullptr, &result.dbg_cb) != VK_SUCCESS) {
			return "Failed to create debug report callback"sv;
		}
	}

	if (glfwCreateWindowSurface(result.instance, window.handle, nullptr, &result.surf) != VK_SUCCESS) {
		return "Failed to create window surface"sv;
	}

	uint32_t dev_count = 0;
	vkEnumeratePhysicalDevices(result.instance, &dev_count, nullptr);
	if (dev_count == 0) {
		return "0 vulkan devices found"sv;
	}
	std::vector<VkPhysicalDevice> devs(dev_count);
	vkEnumeratePhysicalDevices(result.instance, &dev_count, devs.data());

	// FIXME: better heuristic to determine vulkan device
	// For now, use the first device
	result.phys_dev = devs[0];
	if (result.phys_dev == VK_NULL_HANDLE) {
		return "Couldn't get device handle"sv;
	}
	uint32_t dev_extension_count = 0;
	vkEnumerateDeviceExtensionProperties(result.phys_dev, nullptr, &dev_extension_count, nullptr);
	std::vector<VkExtensionProperties> available_extensions(dev_extension_count);
	vkEnumerateDeviceExtensionProperties(result.phys_dev, nullptr, &dev_extension_count, available_extensions.data());

	std::set<std::string_view> required_extensions(device_extensions.begin(), device_extensions.end());
	for (auto const& extension : available_extensions) {
		required_extensions.erase(extension.extensionName);
	}
	if (!required_extensions.empty()) {
		return "Selected device does not support all required extensions";
	};

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(result.phys_dev, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(result.phys_dev, &queue_family_count, queue_families.data());

	int32_t gfx_queue_idx = -1;
	int32_t pres_queue_idx = -1;
	for (size_t i = 0; i < queue_families.size(); ++i) {
		auto const& queue_family = queue_families[i];

		VkBool32 pres_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(result.phys_dev, i, result.surf, &pres_support);

		if (queue_family.queueCount > 0) {
			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				gfx_queue_idx = i;
			}
			if (pres_support) {
				pres_queue_idx = i;
			}
		}

		if (gfx_queue_idx != -1 && pres_queue_idx != -1) {
			break;
		}
	}
	if (gfx_queue_idx == -1) {
		return "Selected device has no suitable gfx queue"sv;
	}
	if (pres_queue_idx == -1) {
		return "Selected device has no suitable pres queue"sv;
	}

	std::set<int> queue_idxs = {gfx_queue_idx, pres_queue_idx};
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos(queue_idxs.size());

	float queue_pri = 1.0f;
	size_t queue_i = 0;
	for (auto const& queue_idx : queue_idxs) {
		VkDeviceQueueCreateInfo queue_create_info = {};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_idx;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_pri;
		queue_create_infos[queue_i] = queue_create_info;
		++queue_i;
	}

	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(result.phys_dev, result.surf, &capabilities);

	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(result.phys_dev, result.surf, &format_count, nullptr);
	if (format_count == 0) {
		return "Selected device does not support any surface formats"sv;
	}
	std::vector<VkSurfaceFormatKHR> formats(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(result.phys_dev, result.surf, &format_count, formats.data());

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(result.phys_dev, result.surf, &present_mode_count, nullptr);
	if (present_mode_count == 0) {
		return "Selected device does not support any surface present modes"sv;
	}
	std::vector<VkPresentModeKHR> present_modes(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(result.phys_dev, result.surf, &present_mode_count, present_modes.data());

	VkSurfaceFormatKHR best_format = {};
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		best_format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	} else if (
		auto const& wanted = std::find(formats.begin(), formats.end(), VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR});
		wanted != formats.end()
	) {
		best_format = *wanted;
	} else {
		best_format = formats[0];
	}

	VkPresentModeKHR best_present_mode = {};
	// FIXME: Use for-loop here
	// FIXME: Determine if we really need to use immediate mode over FIFO
	if (std::find(present_modes.begin(), present_modes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != present_modes.end()) {
		best_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
	} else if (std::find(present_modes.begin(), present_modes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != present_modes.end()) {
		best_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	} else if (std::find(present_modes.begin(), present_modes.end(), VK_PRESENT_MODE_FIFO_KHR) != present_modes.end()) {
		best_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	} else {
		return "Couldn't find suitable present mode"sv;
	}

	VkExtent2D extent = {};
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		extent = capabilities.currentExtent;
	} else {
		extent.width = std::max(
			capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, (uint32_t)window.width)
		);
		extent.height = std::max(
			capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, (uint32_t)window.height)
		);
	}

	uint32_t image_count = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
		image_count = capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR swapchain_create_info = {};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = result.surf;
	swapchain_create_info.minImageCount = image_count;
	swapchain_create_info.imageFormat = best_format.format;
	swapchain_create_info.imageColorSpace = best_format.colorSpace;
	swapchain_create_info.imageExtent = extent;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	uint32_t const queue_idxs_32[] = {(uint32_t)gfx_queue_idx, (uint32_t)pres_queue_idx};
	if (gfx_queue_idx != pres_queue_idx) {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = queue_idxs_32;
	} else {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	swapchain_create_info.preTransform = capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = best_present_mode;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

	// create swapchain after creating logical dev

	VkPhysicalDeviceFeatures dev_features = {};

	VkDeviceCreateInfo dev_create_info = {};
	dev_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_create_info.pQueueCreateInfos = queue_create_infos.data();
	dev_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	dev_create_info.pEnabledFeatures = &dev_features;
	dev_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
	dev_create_info.ppEnabledExtensionNames = device_extensions.data();
	dev_create_info.enabledLayerCount = 0;
	if (enable_validation) {
		dev_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		dev_create_info.ppEnabledLayerNames = validation_layers.data();
	}
	if (vkCreateDevice(result.phys_dev, &dev_create_info, nullptr, &result.log_dev) != VK_SUCCESS) {
		return "Failed to create logical device"sv;
	}

	if (vkCreateSwapchainKHR(result.log_dev, &swapchain_create_info, nullptr, &result.swapchain) != VK_SUCCESS) {
		return "Failed to create swapchain"sv;
	}

	vkGetSwapchainImagesKHR(result.log_dev, result.swapchain, &image_count, nullptr);
	result.swapchain_imgs.resize(image_count);
	vkGetSwapchainImagesKHR(result.log_dev, result.swapchain, &image_count, result.swapchain_imgs.data());
	result.swapchain_format = best_format.format;
	result.swapchain_extent = extent;

	vkGetDeviceQueue(result.log_dev, gfx_queue_idx, 0, &result.gfx_queue);
	vkGetDeviceQueue(result.log_dev, pres_queue_idx, 0, &result.pres_queue);

	// We made it!

	return result;
}

Result<Unit> VulkanResources::destroy() {
	using namespace std::literals;

	vkDestroySwapchainKHR(this->log_dev, this->swapchain, nullptr);
	vkDestroyDevice(this->log_dev, nullptr);

	if (enable_validation) {
		auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(
			instance,
			"vkDestroyDebugReportCallbackEXT"
			);
		if (func != nullptr) {
			func(this->instance, this->dbg_cb, nullptr);
		} else {
			return "failed to vkDestroyReportCallbackEXT"sv;
		}
	}

	vkDestroySurfaceKHR(this->instance, this->surf, nullptr);
	vkDestroyInstance(this->instance, nullptr);

	return unit;
}
