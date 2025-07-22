#ifndef _H_Material_H_
#define _H_Material_H_

#include "Shader.h"
#include <glm/glm.hpp>

class Material
{
public:
	Material() = default;
	Material(glm::vec3 color, float shininess=8.0); // color material

	virtual ~Material() = default;
	void upload(Shader& shader) const;

	void setAmb(const glm::vec3& v);
	void setDiff(const glm::vec3& v);
	void setSpec(const glm::vec3& v);
	void setExp(GLfloat exp);

	void setCopper();

protected:
	// Coeficientes de reflexión
	glm::vec3 ambient = {0.2, 0.2, 0.2};
	glm::vec3 diffuse = {0.8, 0.8, 0.8};
	glm::vec3 specular = {0.0, 0.0, 0.0};
	// Exponente para la reflexión especular
	GLfloat expF = 0;
	// GLuint face = GL_FRONT_AND_BACK;
};

inline void
Material::setAmb(const glm::vec3& v)
{
	ambient = v;
}

inline void
Material::setDiff(const glm::vec3& v)
{
	diffuse = v;
}

inline void
Material::setSpec(const glm::vec3& v)
{
	specular = v;
}

inline void
Material::setExp(GLfloat exp)
{
	expF = exp;
}

#endif //_H_Material_H_

