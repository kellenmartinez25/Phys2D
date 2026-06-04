#include "Renderer.h"
#include <optional>
Renderer::Renderer(Shader &Shader) {
	this->shader = Shader;
	initRenderData();
}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &VAO);
}

void Renderer::initRenderData()
{
	GLuint VBO;	// buffer
	// we're just using a rectangle for now
	float vertices[] = {
		// Bottom-left triangle
		// Pos			
		0.0f, 1.0f,		// Top Left			|*		(0, 0)
		1.0f, 0.0f,		// Bottom Right		|  *	(1, 1)
		0.0f, 0.0f,		// Bottom Left		|____*	
		// Top Right Triangle
		// Pos								_____
		0.0f, 1.0f,		// Top Left			*    |
		1.0f, 1.0f,		// Top Right		  *  |
		1.0f, 0.0f,		// Bottom Right		    *|
	};
	// do the generating objs
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &VBO);
	// bind vbo
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// configure vbo with our vertices
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// bind our VAO
	glBindVertexArray(this->VAO);
	// this will be in location 0
	glEnableVertexAttribArray(0);
	// configure our VAO with our vertex array, made up of a series of vec2s
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	// unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}
// draw box to screen with color
void Renderer::drawSprite(glm::dvec2 pos, glm::dvec2 size, float rotate, glm::vec3 color, bool drawBorder)
{
	// activate shader
	this->shader.use();
	// the model matrix, to transform our position
	glm::mat4 model = glm::mat4(1.0f);
	// move to the given position
	model = glm::translate(model, glm::vec3(pos, 0.0f));

	// move rectangle to its center before rotating, then move it back to the corner
	model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
	model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
	// scaling
	model = glm::scale(model, glm::vec3(size, 1.0f));
	// update uniform
	shader.setMat4("model", model);
	// and color too
	shader.setVec3f("spriteColor", color);
	// push size for border rendering
	shader.setVec2f("size", size);
	// toggle border
	shader.setInt("drawBorder", drawBorder);

	// bind VAO
	glBindVertexArray(this->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// we're only using 1 VAO for now so no need to unbind it
	//glBindVertexArray(0);
}