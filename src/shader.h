#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
	GLuint ID;
	Shader() { }
	void Compile(const char* vertexShaderPath, const char* fragmentShaderPath);
	Shader& use();
	void setInt(const char* name, GLint value);
	void setFloat(const char* name, GLfloat value);
	void setVec2f(const char* name, glm::vec2 vec2);
	void setVec3f(const char* name, GLfloat a, GLfloat b, GLfloat c);
	void setVec3f(const char* name, glm::vec3 vec3);
	void setVec4f(const char* name, glm::vec4 vec4);
	void setMat4(const char* name, glm::mat4 mat4);
	
private:
	GLuint getLocation(const char* name) const;
	void checkCompileErrors(GLuint shaderObject, const char* type);
};