#include "Material.h"

#include <glm/gtc/type_ptr.hpp>

using namespace glm;

Material::Material(vec3 color, float shininess)
 : ambient(color)
 , diffuse(color)
 , specular(color)
 , expF(shininess)
{
}

void
Material::upload(Shader& lighting) const
{
	lighting.setUniform("material.ambient", ambient);
	lighting.setUniform("material.diffuse", diffuse);
	lighting.setUniform("material.specular", specular);
	lighting.setUniform("material.shininess", expF);

	// glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

void
Material::setCopper()
{
	ambient = {0.19125, 0.0735, 0.0225};
	diffuse = {0.7038, 0.27048, 0.0828};
	specular = {0.256777, 0.137622, 0.086014};
	expF = 12.8;
}
