#pragma once
#include <glad/glad.h>

class Texture2D {
public:
	GLuint ID;
	GLuint Width, Height;
	GLuint internalFormat;
	GLuint imagFormat;
	GLuint wrap_S;
	GLuint wrap_T;
	GLuint filterMin;
	GLuint filterMax;
	Texture2D(bool singleWhite = false);
	// generate a 1x1 white texture
	void generate();
	void generate(GLuint width, GLuint height, unsigned char* data);
	void bind() const;
};