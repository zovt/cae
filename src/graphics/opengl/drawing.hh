#pragma once

#include <vector>

#include "index.hh"
#include "shaders.hh"
#include "../../defer.hh"

namespace graphics { namespace opengl { namespace drawing {

template<typename DataType>
struct PrimitiveData {
	using vertex_data = DataType;

	std::vector<DataType> const& vertices;
	std::vector<GLuint> const& indices;

	size_t get_n_indices() const {
		return this->indices.size();
	}

	GLuint get_indices_sz() const {
		return this->indices.size() * sizeof(GLuint);
	}

	GLuint const* get_indices_ptr() const {
		return this->indices.data();
	}

	GLuint get_vertices_sz() const {
		return this->vertices.size() * sizeof(DataType);
	}

	DataType const* get_vertices_ptr() const {
		return this->vertices.data();
	}
};

struct GLVertexData  {
	GLuint vao;
	GLuint vbo;
	GLuint ebo;

	size_t n_indices;

	void activate() const;
	void deactivate() const;
	void draw() const;

	template <
		typename Data,
		typename VertIn,
		typename FragOut
	>
	GLVertexData(
		Data const& data,
		shaders::Program<VertIn, FragOut> const& shdr
	) : n_indices(data.get_n_indices()) {
		static_assert(std::is_same<
			typename Data::vertex_data,
			typename VertIn::vertex_data
		>::value);

		glGenVertexArrays(1, &this->vao);
		glGenBuffers(1, &this->ebo);
		glGenBuffers(1, &this->vbo);

		this->activate();
		defer([&]() { this->deactivate(); });

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
		glBufferData(
			GL_ARRAY_BUFFER,
			data.get_vertices_sz(),
			data.get_vertices_ptr(),
			GL_STATIC_DRAW
		);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			data.get_indices_sz(),
			data.get_indices_ptr(),
			GL_STATIC_DRAW
		);

		shdr.activate_vert_attr_arrays();
		shdr.activate();
	}

	GLVertexData(GLVertexData&& other);
	GLVertexData(GLVertexData const& other) = delete;
	GLVertexData& operator=(GLVertexData const& other) = delete;
	GLVertexData& operator=(GLVertexData&& other);
	~GLVertexData();
};

} } }
