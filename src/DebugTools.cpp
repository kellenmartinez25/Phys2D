#include "DebugTools.h"
#include "resource_manager.h"

// 
void DebugDraw::render() {
	std::vector<DebugPoint> vertices;
	// put both points in line to vertices to be drawn
	for (auto& l : lines) {
		vertices.push_back(l.a);
		vertices.push_back(l.b);
	}
	
	// put all vertices to buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER,
		vertices.size() * sizeof(DebugPoint),
		vertices.data(),
		GL_DYNAMIC_DRAW);
	glBindVertexArray(vao);
	// activate shader
	this->shader.use();

	// line draw
	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, vertices.size());

	// all points to be drawn
	std::vector<DebugPoint> pointVertices;
	for (auto& p : points) {
		pointVertices.push_back(p);
	}
	// Fill with points
	glBufferData(GL_ARRAY_BUFFER,
		pointVertices.size() * sizeof(DebugPoint),
		pointVertices.data(),
		GL_DYNAMIC_DRAW);
	glDrawArrays(GL_POINTS, 0, pointVertices.size());
	// delete all points and lines after every render call
	clear();
}

void DebugDraw::init() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, 256 * sizeof(DebugPoint), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(DebugPoint), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugPoint), (void*)(sizeof(glm::vec2)));

	this->shader = ResourceManager::loadShader("debugShader.vert", "debugShader.frag", "debugShader");
}