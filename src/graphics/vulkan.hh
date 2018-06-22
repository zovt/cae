#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstring>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../err.hh"
#include "../unit.hh"
#include "window.hh"

namespace vulkan {

struct VulkanResources {
	VkInstance instance;
	VkDebugReportCallbackEXT dbg_cb;
	VkPhysicalDevice phys_dev;
	VkDevice log_dev;
	VkQueue gfx_queue;
	VkQueue pres_queue;
	VkSurfaceKHR surf;


	static err::Result<VulkanResources> create(window::Window const& window);

	err::Result<Unit> destroy();
};

}
