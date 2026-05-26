#include "DebugTools.h"
#include "resource_manager.h"


void DebugDraw::render() {
	std::vector<DebugVertex> vertices;

	for (auto& l : lines) {
		vertices.push_back(l.a);
		vertices.push_back(l.b);
	}
	
	// put to gpu
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER,
		vertices.size() * sizeof(DebugVertex),
		vertices.data(),
		GL_DYNAMIC_DRAW);
	glBindVertexArray(vao);
	this->shader.use();

	// line draw
	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, vertices.size());

	// points
	std::vector<DebugVertex> pointVertices;
	for (auto& p : points) {
		pointVertices.push_back(p.p);
	}

	glBufferData(GL_ARRAY_BUFFER,
		pointVertices.size() * sizeof(DebugVertex),
		pointVertices.data(),
		GL_DYNAMIC_DRAW);
	glDrawArrays(GL_POINTS, 0, pointVertices.size());
	
	clear();
}

void DebugDraw::init() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, 256 * sizeof(DebugVertex), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)(sizeof(glm::vec2)));

	this->shader = ResourceManager::loadShader("debugShader.vert", "debugShader.frag", nullptr, "debugShader");
	


}