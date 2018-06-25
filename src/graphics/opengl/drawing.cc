#include "drawing.hh"

#include <iostream>

#include "../../defer.hh"

using namespace graphics::opengl::drawing;

GLVertexData::GLVertexData(GLVertexData&& other) {
	this->vao = other.vao;
	this->vbo = other.vbo;
	this->ebo = other.ebo;
	this->n_indices = other.n_indices;

	other.ebo = 0;
	other.vao = 0;
	other.ebo = 0;
}

GLVertexData& GLVertexData::operator=(GLVertexData&& other) {
	this->vao = other.vao;
	this->vbo = other.vbo;
	this->ebo = other.ebo;
	this->n_indices = other.n_indices;

	other.ebo = 0;
	other.vao = 0;
	other.ebo = 0;

	return *this;
}

GLVertexData::~GLVertexData() {
	glDeleteVertexArrays(1, &this->vao);
	glDeleteBuffers(1, &this->ebo);
	glDeleteBuffers(1, &this->vbo);
}

void GLVertexData::activate() const {
	glBindVertexArray(this->vao);
}

void GLVertexData::deactivate() const {
	glBindVertexArray(0);
}

void GLVertexData::draw() const {
	this->activate();
	defer([&]() { this->deactivate(); });
	glDrawElements(GL_TRIANGLES, this->n_indices, GL_UNSIGNED_INT, 0);
}
