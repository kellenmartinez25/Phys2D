#include "resource_manager.h"
#include <iostream>

// location of shader sources
#define SHADER_PREFIX "resources/shaders/"

// loaded shaders
std::map<std::string, Shader> ResourceManager::Shaders;

// load shader from the filenames of vert/frag/optional geo. name will be used to identify and manage your created shader program
Shader ResourceManager::loadShader(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath, std::string name)
{
	Shaders[name] = loadShaderFromFile(vertexShaderPath, fragmentShaderPath, geometryShaderPath);
	return Shaders[name];
}
// get shader object from name
Shader& ResourceManager::getShader(std::string name)
{
	return Shaders[name];
}
// release all resources
void ResourceManager::clear() {
	for (auto iter : Shaders) {
		glDeleteProgram(iter.second.ID);
	}
}

// load a text file from path
// pathprefix is the (relative) folder where it is stored, such as a shader/resource folder, will be appended to the start of the path argument
std::string ResourceManager::loadTextFileFromPath(const char* path, const char* pathPrefix = nullptr) {
	std::string fileString;
	try {
		// create input file stream & string stream
		std::ifstream fileStream;
		std::stringstream ss;
		// handle path prefix
		if (pathPrefix != nullptr) {
			// append path prefix if provided and open file
			fileStream.open(std::string(pathPrefix).append(path));
		}
		else {
			// open using provided path without prefix
			fileStream.open(path);
		}
		// load entire file into stringstream
		ss << fileStream.rdbuf();
		// close file
		fileStream.close();
		// convert sstream to normal string
		fileString = ss.str();
	}
	// handle file exceptions
	catch (std::exception& e) {
		std::cerr << "ERROR: Failed to read file " << path << std::endl;
	}
	return fileString;
}
// internal method, creates a new shader object using provided source files
Shader ResourceManager::loadShaderFromFile(const char* vertexShaderPath, const char* fragmentShaderPath, const char* geometryShaderPath)
{
	std::string vertShaderCode, fragShaderCode, geometryShaderCode;
	// load shader source code files
	vertShaderCode = loadTextFileFromPath(vertexShaderPath, SHADER_PREFIX);
	fragShaderCode = loadTextFileFromPath(fragmentShaderPath, SHADER_PREFIX);
	const char* geometryShaderCodeCS;
	if (geometryShaderPath != nullptr) {
		geometryShaderCode = loadTextFileFromPath(geometryShaderPath, SHADER_PREFIX);
		geometryShaderCodeCS = geometryShaderCode.c_str();
	}
	// create shader object from source
	Shader shader;
	// compile them
	shader.Compile(vertShaderCode.c_str(), fragShaderCode.c_str(), geometryShaderPath != nullptr ? geometryShaderCodeCS : nullptr);
	return shader;
}
