#include "resource_manager.h"
#include <iostream>

// location of shader sources
#define SHADER_PREFIX "resources/shaders/"

// loaded shaders
std::map<std::string, Shader> ResourceManager::Shaders;
// loaded textures
std::map<std::string, Texture2D> ResourceManager::Textures;

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
	for (auto iter : Textures) {
		glDeleteTextures(1, &iter.second.ID);
	}
}
// load texture from file, providing channels
// name will be used to identify and get texture objects
Texture2D ResourceManager::loadTexture(const char* file, bool alpha, std::string name, int nChannels)
{
	Textures[name] = loadTextureFromFile(file, alpha, nChannels);
	return Textures[name];
}
// get texture from name
Texture2D& ResourceManager::getTexture(std::string name)
{
	return Textures[name];
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
// load texture from file path
Texture2D ResourceManager::loadTextureFromFile(const char* file, bool alpha, int desiredChannels) {
	// idk i use these a lot
	using std::cout, std::endl;
	Texture2D texture;
	// use RGBA if there is alpha channel
	if (alpha) {
		texture.internalFormat = GL_RGBA;
		texture.imagFormat = GL_RGBA;
	}
	
	int width, height, nChannels;
	
	//stbi_set_flip_vertically_on_load(true);
	// load img data using stbi
	unsigned char* data = stbi_load(file, &width, &height, &nChannels, desiredChannels);
	// generate or print error
	if (data) {
		texture.generate(width, height, data);
	}
	else {
		cout << "Failed to load texture " << file << endl;
	}
	//cout << stbi_failure_reason() << endl;
	// no need to have this loaded into memory anymore
	stbi_image_free(data);
	return texture;
}
