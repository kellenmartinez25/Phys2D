#include "Texture.h"

// turn singleWhite to true if you just need a 1x1 white pixel as the texture
Texture2D::Texture2D(bool singleWhite) :
	// default sizes and formats
	Width(0),
	Height(0),
	internalFormat(GL_RGB),
	imagFormat(GL_RGB),
	wrap_S(GL_REPEAT),
	wrap_T(GL_REPEAT),
	filterMin(GL_LINEAR),
	filterMax(GL_LINEAR)
{
	// init with the ID
	glGenTextures(1, &this->ID);
	// generate single white if needed
	if (singleWhite) {
		generate();
	}
}
// create Texture2D object using provided image data/width/height
void Texture2D::generate(GLuint width, GLuint height, unsigned char* data) {
	// update size
	this->Width = width;
	this->Height = height;
	// create & bind OGL texture
	glBindTexture(GL_TEXTURE_2D, ID);

	// wrapping + filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_S);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_T);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMax);
	// if we don't do this then it will appear slanted idk why
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// configure texture object
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, imagFormat, GL_UNSIGNED_BYTE, data);
	// mippymappy
	glGenerateMipmap(GL_TEXTURE_2D);
	
	// unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

}
// Blank white texture
void Texture2D::generate() {
	// literally just 0xFFFFFF
	unsigned char texData[] = { 255, 255, 255 };
	generate(1, 1, texData);
}
// bind texture
void Texture2D::bind() const {
	glBindTexture(GL_TEXTURE_2D, ID);
}
