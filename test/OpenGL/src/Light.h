#ifndef _H_Light_H
#define _H_Light_H

#include <string>

#include "Shader.h"
#include <glm/glm.hpp>

class Light { // Abstract class
public:
	virtual ~Light();

	void disable(Shader& shader);
	void enable(Shader& shader);

	virtual void upload(Shader& shader, glm::dmat4 const& modelViewMat) const;

	void setAmb(const glm::vec3& ind);
	void setDiff(const glm::vec3& ind);
	void setSpec(const glm::vec3& ind);

protected:
	Light(std::string name);
	Light(const std::string& name, int id);

	// Identifier
	std::string lightID;
	// Atributos lumínicos y geométrico de la fuente de luz
	glm::vec3 ambient = {0.1, 0.1, 0.1};
	glm::vec3 diffuse = {0.5, 0.5, 0.5};
	glm::vec3 specular = {0.5, 0.5, 0.5};
	glm::vec4 posDir = {0, 0, 0, 1};
};

class DirLight : public Light {
public:
	explicit DirLight(int id = 0);

	virtual void upload(Shader& shader, glm::dmat4 const& modelViewMat) const override;
	void setDirection(const glm::vec3& dir);
};

inline void
DirLight::setDirection(const glm::vec3& dir) {
	posDir = glm::vec4(dir, 0.0);
}

class PosLight : public Light {
public:
	explicit PosLight(int id = 0);

	virtual void upload(Shader& shader, glm::dmat4 const& modelViewMat) const override;

	void setPosition(const glm::fvec3& dir);
	void setAttenuation(GLfloat kc, GLfloat kl, GLfloat kq);

protected:
	// Factores de atenuación
	GLfloat constant = 1, linear = 0, quadratic = 0;
};

inline void
PosLight::setPosition(const glm::vec3& dir) {
	posDir = glm::vec4(dir, 1.0);
}

inline void
PosLight::setAttenuation(GLfloat nkc, GLfloat nkl, GLfloat nkq) {
	constant = nkc;
	linear = nkl;
	quadratic = nkq;
}

class SpotLight : public PosLight {
public:
	SpotLight(const glm::vec3& pos = {0, 0, 0}, int id = 0);

	virtual void upload(Shader& shader, glm::dmat4 const& modelViewMat) const override;
	void setDirection(const glm::vec3& dir) { direction = dir; }
	void setCutoff(float inner, float outer) { cutoff = inner; outerCutoff = outer; }
	void setExp(float e) { exp = e; }

protected:
	// Atributos del foco
	glm::vec3 direction = {0, 0, -1};
	GLfloat cutoff = 180, outerCutoff = 200;
	GLfloat exp = 0;
};

#endif //_H_Light_H_
