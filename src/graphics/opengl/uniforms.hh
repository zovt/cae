#pragma once

#include <tuple>
#include <glm/glm.hpp>

namespace graphics { namespace opengl { namespace uniforms {

struct GlobalDrawingUniforms {
	glm::mat4 proj;
	glm::mat4 world;

	GlobalDrawingUniforms(float window_w, float window_h);

	void activate() const;
	void regen_proj(int window_w, int window_h);
};

struct TransformUniform {
	glm::mat4 transform;

	void activate() const;
};

template <typename HUniform, typename... RUniforms>
struct UniformGroup {
	HUniform uni;
	UniformGroup<RUniforms...> rest;

	UniformGroup(HUniform uni, RUniforms... rest) : uni(uni), rest(rest...) {}

	void activate() const {
		this->uni.activate();
		this->rest.activate();
	}
};

template <typename HUniform>
struct UniformGroup<HUniform> {
	HUniform uni;

	UniformGroup(HUniform uni) : uni(uni) {}

	void activate() const {
		this->uni.activate();
	}
};

} } }
