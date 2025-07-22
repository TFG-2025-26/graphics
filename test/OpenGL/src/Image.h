#ifndef _H_Image_H
#define _H_Image_H

#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

class Image
{
public:
	using rgba_color = glm::u8vec4;

	// Images are movable and (expensively) copyable
	Image();
	Image(const Image& other);
	Image(Image&& other);

	Image& operator=(const Image& other);
	Image& operator=(Image&& other);

	~Image() { destroy(); }

	// Loading
	void load(const std::string& file_name);
	void load(const rgba_color* data, GLsizei width, GLsizei height);

	// Modifying
	void setAlpha(GLubyte alpha);
	void setColorkeyAlpha(glm::u8vec3 colorKey, GLubyte alpha);

	// Saving
	void save(const std::string& file_name);

	// Properties
	GLsizei width() const { return width_; }
	GLsizei height() const { return height_; }

	const rgba_color* data() const { return data_; }

	bool isNull() const { return width_ == 0 || height_ == 0; }

	// Pixel accessors
	const rgba_color& operator()(GLsizei x, GLsizei y) const
	{
		return data_[y * width_ + x];
	}
	rgba_color& operator()(const GLsizei x, const GLsizei y)
	{
		return data_[y * width_ + x];
	}

	#ifdef __cpp_multidimensional_subscript
	const rgba_color& operator[](GLsizei x, GLsizei y) const { return operator()(x, y); }
	rgba_color& operator[](GLsizei x, GLsizei y) { return operator()(x, y); }
	#endif

private:
	rgba_color* data_;
	GLsizei width_;
	GLsizei height_;
	bool fromSTB;

	void destroy();
	void copy(const Image& other);
};

#endif //_H_Image_H
