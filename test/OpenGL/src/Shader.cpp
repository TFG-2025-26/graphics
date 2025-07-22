#include <fstream>
#include <map>
#include <memory>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

using namespace std;

// Shader factory
map<string, unique_ptr<Shader>> shaders;
constexpr const char* SHADERS_ROOT = "assets/shaders/";

Shader::Shader(const string& name)
{
	GLuint vertex, fragment;

	// Name can be either a plain identifier or a colon-separated name pair
	string vertexName = name, fragmentName = name;

	if (auto pos = name.find(':'); pos != string::npos) {
		vertexName = name.substr(0, pos);
		fragmentName = name.substr(pos + 1);
	}

	// Compile vertex shader
	buildShader(vertex, GL_VERTEX_SHADER, SHADERS_ROOT + vertexName + "_vertex.glsl");
	// Compile fragment shader
	buildShader(fragment, GL_FRAGMENT_SHADER, SHADERS_ROOT + fragmentName + "_fragment.glsl");

	// Link shader program
	mProgram = glCreateProgram();

	glAttachShader(mProgram, vertex);
	glAttachShader(mProgram, fragment);
	glLinkProgram(mProgram);

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

Shader::~Shader()
{
	glDeleteProgram(mProgram);
}

bool
Shader::buildShader(GLuint& shader, GLuint type, const std::string& filename)
{
	ifstream source(filename);

	if (source.is_open()) {
		// Read the buffer in memory
		string buffer(istreambuf_iterator<char>(source), {});
		const char* bufferp = buffer.data();

		shader = glCreateShader(type);
		glShaderSource(shader, 1, &bufferp, NULL);
		glCompileShader(shader);

		// Check if everything went fine
		int compileStatus;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus == GL_FALSE)
			throw std::logic_error("error while compiling shader " + filename);

		return true;
	}

	return false;
}

void
Shader::use()
{
	glUseProgram(mProgram);
}

void
Shader::setUniform(const string& name, bool value)
{
	glUniform1i(glGetUniformLocation(mProgram, name.c_str()), value);
}

void
Shader::setUniform(const string& name, float value)
{
	glUniform1f(glGetUniformLocation(mProgram, name.c_str()), value);
}

void
Shader::setUniform(const string& name, const glm::vec3& value)
{
	glUniform3f(glGetUniformLocation(mProgram, name.c_str()), value.r, value.g, value.b);
}

void
Shader::setUniform(const string& name, const glm::vec4& value)
{
	glUniform4f(glGetUniformLocation(mProgram, name.c_str()), value.r, value.g, value.b, value.a);
}

void
Shader::setUniform(const string& name, const glm::mat4& value)
{
	glUniformMatrix4fv(glGetUniformLocation(mProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

Shader*
Shader::get(const string& name)
{
	auto it = shaders.find(name);

	if (it != shaders.end())
		return it->second.get();

	Shader* shader = new Shader(name);
	shaders[name] = unique_ptr<Shader>(shader);

	return shader;
}

void
Shader::setUniform4All(const string& name, const glm::mat4& value)
{
	for (const auto& [_, shader] : shaders) {
		shader->use();
		shader->setUniform(name, value);
	}
}
