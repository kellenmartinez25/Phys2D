#include "Renderer.h"
#include <optional>
Renderer::Renderer(Shader &Shader) {
	this->shader = Shader;
	initRenderData();
}

Renderer::~Renderer() {
	glDeleteVertexArrays(1, &VAO);
}
// the texture of a blank white pixel
// its not garunteed that OpenGL will be loaded yet so it is an optional
std::optional<Texture2D> BLANK_WHITE;
void Renderer::initRenderData()
{
	GLuint VBO;	// buffer
	// we're just using a rectangle for now
	float vertices[] = {
		// Bottom-left triangle
		// Pos			// Texture
		0.0f, 1.0f,		0.0f, 1.0f,		// Top Left			|*		(0, 0)
		1.0f, 0.0f,		1.0f, 0.0f,		// Bottom Right		|  *	(1, 1)
		0.0f, 0.0f,		0.0f, 0.0f,		// Bottom Left		|____*	
		// Top Right Triangle
		// Pos			// Texture							_____
		0.0f, 1.0f,		0.0f, 1.0f,		// Top Left			*    |
		1.0f, 1.0f,		1.0f, 1.0f,		// Top Right		  *  |
		1.0f, 0.0f,		1.0f, 0.0f		// Bottom Right		    *|
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
	// configure our VAO with our vertex array, made up of a series of vec4s
	// x and y are positions, z & w are texture coordinates
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	// unbind buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	// are we doing a blank white rectangle?
	BLANK_WHITE = Texture2D(true);

}
// draw sprite to screen using a provided texture
void Renderer::drawSprite(Texture2D& texture, glm::vec2 pos, glm::vec2 size, float rotate, glm::vec3 color)
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
	// activate/bind texture
	glActiveTexture(GL_TEXTURE0);
	texture.bind();
	// bind VAO
	glBindVertexArray(this->VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// we're only using 1 VAO for now so no need to unbind it
	//glBindVertexArray(0);
}
// Draw a solid color rectangle
void Renderer::drawSprite(glm::vec2 pos, glm::vec2 size, float rotate, glm::vec3 color)
{
	drawSprite(BLANK_WHITE.value(), pos, size, rotate, color);
}