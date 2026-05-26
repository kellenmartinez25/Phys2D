#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <glad/glad.h>
#include "Shader.h"



class DebugDraw {
private:

	struct DebugVertex {
		glm::vec2 pos;
		glm::vec3 color;
	};
	struct DebugPoint {
		DebugVertex p;
	};
	struct DebugLine {
		DebugVertex a, b;
	};
	std::vector<DebugLine> lines;
	std::vector<DebugPoint> points;

	Shader shader;
	GLuint vao;
	GLuint vbo;
	

public:

	static DebugDraw& get() {
		static DebugDraw inst;
		return inst;
	}
	
	void drawLine(glm::vec2 a, glm::vec2 b, glm::vec3 color) {
		lines.push_back({ { a, color }, { b, color } });
	}
	void drawPoint(glm::vec2 p, glm::vec3 color) {
		points.push_back({p, color});
	}

	void clear() {
		lines.clear();
		points.clear();
	}

	void render();
	void init();
};