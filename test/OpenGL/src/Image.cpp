#include "Image.h"

#include <stdexcept>

// stb_image and stb_image_write are using for reading
// and writing images (https://github.com/nothings/stb)
#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

Image::Image()
 : data_(nullptr)
 , width_(0)
 , height_(0)
 , fromSTB(false)
{
}

Image::Image(const Image& other)
{
	copy(other);
}

Image::Image(Image&& other)
 : data_(other.data_)
 , width_(other.width_)
 , height_(other.height_)
 , fromSTB(other.fromSTB)
{
	other.data_ = nullptr;
	other.width_ = 0;
	other.height_ = 0;
	other.fromSTB = false;
}

Image&
Image::operator=(const Image& other)
{
	destroy();
	copy(other);
	return *this;
}

Image&
Image::operator=(Image&& other)
{
	destroy();
	data_ = other.data_;
	width_ = other.width_;
	height_ = other.height_;
	fromSTB = other.fromSTB;
	return *this;
}

void
Image::destroy()
{
	if (fromSTB) stbi_image_free(data_);
	else delete data_;

	data_ = nullptr;
	width_ = 0;
	height_ = 0;
	fromSTB = false;
}

void
Image::copy(const Image& other)
{
	width_ = other.width_;
	height_ = other.height_;
	fromSTB = false;

	GLsizei size = width_ * height_;

	if (size != 0) {
		data_ = new rgba_color[size];
		memcpy(data_, other.data_, size * sizeof(rgba_color));
	}
	else {
		data_ = nullptr;
	}
}


void
Image::load(const string& filename)
{
	destroy(); // clean previous image

	// Loads pixel data with stb_image into the attributes

	int width, height, channels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 4);

	if (data == nullptr)
		throw std::logic_error("Image::load(string&): ERROR: cannot load image: "s + stbi_failure_reason());

	width_ = width;
	height_ = height;
	fromSTB = true;
	data_ = reinterpret_cast<rgba_color*>(data);
}


void
Image::load(const rgba_color* data, GLsizei width, GLsizei height)
{

	destroy(); // clean previous image

	width_ = width;
	height_ = height;
	fromSTB = false;

	memcpy(data_, data, width * height * sizeof(rgba_color));
}

void
Image::setAlpha(GLubyte alpha)
{
	for (GLsizei i = 0; i < width_ * height_; i++)
		data_[i].a = alpha;
}

void
Image::setColorkeyAlpha(glm::u8vec3 colorKey, GLubyte alpha)
{
	for (GLsizei i = 0; i < width_ * height_; i++) {
		if (data_[i].r == colorKey.r && data_[i].g == colorKey.g &&
		    data_[i].b == colorKey.b)
			data_[i].a = alpha;
	}
}

void
Image::save(const std::string& name)
{
	string extension = name.substr(name.rfind(".") + 1);

	for (char& c : extension)
		c = tolower(c);

	int result = 0;

	if (extension == "png")
		result = stbi_write_png(name.c_str(), width_, height_, 4, data_, width_ * sizeof(rgba_color));
	else if (extension == "bmp")
		result = stbi_write_bmp(name.c_str(), width_, height_, 4, data_);
	else if (extension == "tga")
		result = stbi_write_tga(name.c_str(), width_, height_, 4, data_);
	else if (extension == "jpg")
		result = stbi_write_jpg(name.c_str(), width_, height_, 4, data_, 80);
	else
		throw std::logic_error("Image::load(string&): ERROR: unknown image format: " + extension);

	if (result == 0)
		throw std::logic_error("Image::load(string&): ERROR: cannot save image.");

}
