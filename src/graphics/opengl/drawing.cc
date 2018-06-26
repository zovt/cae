#include "drawing.hh"

#include <iostream>

using namespace graphics::opengl::drawing;
using namespace graphics::opengl::shaders;

void do_nothing(GLsizei size, const GLuint* ptr) {
	(void)size;
	(void)ptr;
}

GLResource::GLResource(GLResource::create_func create, GLResource::destroy_func destroy) {
	this->destroy = destroy;
	(*create)(1, &this->id);
}

GLResource::~GLResource() {
	(*this->destroy)(1, &this->id);
}

GLResource::GLResource(GLResource&& other) {
	this->id = other.id;
	this->destroy = other.destroy;

	other.id = 0;
	other.destroy = do_nothing;
}

GLResource& GLResource::operator=(GLResource&& other) {
	if (this != &other) {
		this->id = other.id;
		this->destroy = other.destroy;

		other.id = 0;
		other.destroy = do_nothing;
	}

	return *this;
}

VAO::VAO() : vao(glGenVertexArrays, glDeleteVertexArrays) {}

void VAO::activate() const {
	glBindVertexArray(this->vao.id);
}

void VAO::deactivate() const {
	glBindVertexArray(0);
}

EBO::EBO(std::vector<GLuint> const& indices) : ebo(glGenBuffers, glDeleteBuffers) {
	this->n_indices = indices.size();
	this->activate();
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(GLuint) * this->n_indices,
		indices.data(),
		GL_STATIC_DRAW
	);
}

void EBO::activate() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo.id);
}

void DrawInfo::draw(Program const& shdr) const {
	this->vao.use([&]() {
		shdr.activate();
		glDrawElements(GL_TRIANGLES, this->n_indices, GL_UNSIGNED_INT, 0);
	});
}
