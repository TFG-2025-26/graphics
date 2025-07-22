#ifndef _H_Shader_H_
#define _H_Shader_H_

#include <string>

#include <glm/fwd.hpp>
#include <GL/glew.h>

class Shader
{
public:
	Shader(const std::string& name);
	~Shader();

	Shader(const Shader& m) = delete;            // no copy constructor
	Shader& operator=(const Shader& m) = delete; // no copy assignment

	void use();

	void setUniform(const std::string& name, bool value);
	void setUniform(const std::string& name, float value);
	void setUniform(const std::string& name, const glm::vec3& value);
	void setUniform(const std::string& name, const glm::vec4& value);
	void setUniform(const std::string& name, const glm::mat4& value);

	static Shader* get(const std::string& name);

	static void setUniform4All(const std::string& name, const glm::mat4& value);

protected:
	GLuint mProgram;

	bool buildShader(GLuint& shader, GLuint type, const std::string& filename);
};

#endif // _H_Shader_H_
