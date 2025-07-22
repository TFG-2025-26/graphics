#ifndef _H_Texture_H_
#define _H_Texture_H_

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Texture
{
public:
	Texture() = default;
	~Texture();

	Texture(const Texture& tex) = delete;            // no copy constructor
	Texture& operator=(const Texture& tex) = delete; // no copy assignment

	void load(const std::string& filename,
	          GLubyte alpha = 255); // load from file and upload to GPU

	void bind();
	void unbind() const { glBindTexture(GL_TEXTURE_2D, 0); };

	GLuint width() const { return mWidth; };
	GLuint height() const { return mHeight; };

	void setWrap(GLuint wp); // GL_REPEAT, GL_CLAMP_TO_EDGE, ...

protected:
	void init();

	GLuint mWidth = 0;
	GLuint mHeight = 0;
	GLuint mId = 0;
};

#endif //_H_Texture_H_
