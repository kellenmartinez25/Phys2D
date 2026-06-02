#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

class Renderer {
public:
	Renderer(Shader &shader);
	~Renderer();
	
	/* Draw a solid color rectangle.
	* @param pos Position of the sprite, relative to the top-left corner.
	* @param size Size of the sprite
	* @param rotate CCW Rotation, in degrees
	* @param color RGB color value of the texture
	*/
	void drawSprite(glm::dvec2 pos,
		glm::dvec2 size = glm::dvec2(10.0f, 10.0f),
		float rotate = 0.0f,
		glm::vec3 color = glm::vec3(1.0f));
	void initRenderData();
private:
	Shader shader;
	GLuint VAO;
};