#pragma once

#include <tuple>
#include <glm/glm.hpp>

namespace graphics { namespace opengl { namespace uniforms {

struct GlobalDrawingUniforms {
	glm::mat4 proj;
	glm::mat4 world;

	GlobalDrawingUniforms(float window_w, float window_h);

	void activate() const;
	void regen_proj(float window_w, float window_h);
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

template <typename HUniform, typename... RUniforms>
struct UniformGroup<std::reference_wrapper<HUniform>, RUniforms...> {
	std::reference_wrapper<HUniform> uni_ref;
	UniformGroup<RUniforms...> rest;

	UniformGroup(std::reference_wrapper<HUniform> uni_ref, RUniforms... rest)
	: uni_ref(uni_ref), rest(rest...) {}

	void activate() const {
		this->uni_ref.get().activate();
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

template <typename HUniform>
struct UniformGroup<std::reference_wrapper<HUniform>> {
	std::reference_wrapper<HUniform> uni_ref;

	UniformGroup(std::reference_wrapper<HUniform> uni_ref) : uni_ref(uni_ref) {}

	void activate() const {
		this->uni_ref.get().activate();
	}
};


} } }
