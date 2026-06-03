#pragma once
#include <map>
#include <string>
#include <glad/glad.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "shader.h"

class ResourceManager {
public:
	static std::map<std::string, Shader> Shaders;
	// Create a shader from by supplying a path (in resources/shaders) to the vertex and frag shader.
	static Shader loadShader(const char* vertexShaderPath, const char* fragmentShaderPath, std::string name);
	static Shader& getShader(std::string name);
	static std::string loadTextFileFromPath(const char* path, const char* pathPrefix);
	static void clear();

private:
	ResourceManager() {}
	static Shader loadShaderFromFile(const char* vertexShaderPath, const char* fragmentShaderPath);
};