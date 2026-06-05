#include "shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
// compile shader with vertex, frag, and (optional) geo shader
// takes in the entire source code as a string
void Shader::Compile(const char* vertexShaderSrc, const char* fragmentShaderSrc)
{
	// IDs for each shader
	GLuint vertShader, fragShader;
	// Vertex //
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertexShaderSrc, NULL);
	glCompileShader(vertShader);
	checkCompileErrors(vertShader, "VERTEX");
	// Fragment //
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragmentShaderSrc, NULL);
	glCompileShader(fragShader);
	checkCompileErrors(fragShader, "FRAGMENT");

	// Create program
	this->ID = glCreateProgram();
	// Attach shaders
	glAttachShader(this->ID, vertShader);
	glAttachShader(this->ID, fragShader);
	// Linking
	glLinkProgram(this->ID);
	checkCompileErrors(this->ID, "PROGRAM");
	// Delete leftover shaders
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
}
// activate shader
Shader& Shader::use()
{
	glUseProgram(ID);
	return *this;
}
// set uniform int
void Shader::setInt(const char* name, GLint value)
{
	glUniform1i(getLocation(name), value);
}
// set uniform float
void Shader::setFloat(const char* name, GLfloat value)
{
	glUniform1f(getLocation(name), value);
}
// set uniform vec2 from glm::dvec2
// This is a lossy conversion from double to float but since the rest of my program uses
void Shader::setVec2f(const char* name, glm::vec2 vec2)
{
	glUniform2f(getLocation(name), vec2.x, vec2.y);
}
// set uniform vec3
void Shader::setVec3f(const char* name, GLfloat a, GLfloat b, GLfloat c)
{
	glUniform3f(getLocation(name), a, b, c);
}
// set uniform vec3 from glm::vec3
void Shader::setVec3f(const char* name, glm::vec3 vec3)
{
	glUniform3f(getLocation(name), vec3.x, vec3.y, vec3.z);
}
// vec4
void Shader::setVec4f(const char* name, glm::vec4 vec4) {
	glUniform4f(getLocation(name), vec4.x, vec4.y, vec4.z, vec4.w);
}
// set uniform mat4
void Shader::setMat4(const char* name, glm::mat4 mat4)
{
	glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, glm::value_ptr(mat4));
}
// print any compile errors to console
void Shader::checkCompileErrors(GLuint shaderObject, const char* type) {
	int success;
	char infoLog[1024];
	if (type != "PROGRAM") {
		glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shaderObject, 1024, NULL, infoLog);
			std::cout << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
				<< infoLog << "\n -- --------------------------------------------------- -- "
				<< std::endl;
		}
	}
	else {
		glGetProgramiv(shaderObject, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shaderObject, 1024, NULL, infoLog);
			std::cout << "| ERROR::Shader: Link-time error: Type: " << type << "\n"
				<< infoLog << "\n -- --------------------------------------------------- -- "
				<< std::endl;
		}
	}
}
// get a uniform's location; helper function
GLuint Shader::getLocation(const char* name) const
{
	return glGetUniformLocation(ID, name);
}
