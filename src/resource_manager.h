#pragma once
#include <map>
#include <string>
#include <glad/glad.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include "stb_image.h"
#include "shader.h"
#include "texture.h"

class ResourceManager {
public:
	static std::map<std::string, Shader> Shaders;
	static std::map<std::string, Texture2D> Textures;
	static Shader loadShader(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath, std::string name);
	static Shader& getShader(std::string name);
	static Texture2D loadTexture(const char* file, bool alpha, std::string name, int nChannels = 0);
	static Texture2D& getTexture(std::string name);
	static std::string loadTextFileFromPath(const char* path, const char* pathPrefix);
	static void clear();

private:
	ResourceManager() {}
	static Shader loadShaderFromFile(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath = nullptr);
	static Texture2D loadTextureFromFile(const char* file, bool alpha, int desiredChannels = 0);
};