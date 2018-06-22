#include "vulkan.hh"

#include <vector>
#include <iostream>
#include <string_view>
#include <utility>

using namespace vulkan;
using namespace err;

static const std::vector<const char*> validation_layers = {
	"VK_LAYER_LUNARG_standard_validation",
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

err::Result<VulkanResources> VulkanResources::create() {
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

	uint32_t dev_count = 0;
	vkEnumeratePhysicalDevices(result.instance, &dev_count, nullptr);
	if (dev_count == 0) {
		return "0 vulkan devices found"sv;
	}
	std::vector<VkPhysicalDevice> devs(dev_count);
	vkEnumeratePhysicalDevices(result.instance, &dev_count, devs.data());

	return std::move(result);
}

Result<Unit> VulkanResources::destroy() {
	using namespace std::literals;
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

	vkDestroyInstance(this->instance, nullptr);

	return unit;
}
