#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include "Shader.h"



class DebugDraw {
private:
	// Represents a single point
	struct DebugPoint {
		glm::vec2 pos;
		glm::vec3 color;
	};
	// just 2 points
	struct DebugLine {
		DebugPoint a, b;
	};
	std::vector<DebugLine> lines;
	std::vector<DebugPoint> points;
	// the debug shader
	Shader shader;
	// the vao for rendering the dots
	GLuint vao;
	GLuint vbo;
	

public:
	// Get the DebugDraw instance.
	static DebugDraw& get() {
		static DebugDraw inst;
		return inst;
	}
	// Draw a line to the screen.
	void drawLine(glm::vec2 a, glm::vec2 b, glm::vec3 color) {
		lines.push_back({ { a * 100.0f, color }, { b * 100.0f, color } });
	}
	// Draw a single point to the screen.
	void drawPoint(glm::vec2 p, glm::vec3 color) {
		points.push_back({p * 100.0f, color});
	}
	// Clear all queued lines and points to the screen
	void clear() {
		lines.clear();
		points.clear();
	}
	// Render all queued lines and points
	void render();
	// Initialize the DebugDraw instance. Call only once.
	void init();
};