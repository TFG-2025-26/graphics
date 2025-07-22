#include "Light.h"

#include <glm/gtc/type_ptr.hpp>

using namespace glm;

Light::Light(std::string name)
 : lightID(std::move(name))
{
}

Light::Light(const std::string& name, int id)
 : lightID(name + "[" + std::to_string(id) + "]")
{
}

Light::~Light()
{
}

void Light::upload(Shader& shader, glm::dmat4 const& modelViewMat) const
{
	// Transfer light properties to the GPU
	shader.setUniform(lightID + ".ambient", ambient);
	shader.setUniform(lightID + ".diffuse", diffuse);
	shader.setUniform(lightID + ".specular", specular);
}

void Light::disable(Shader& shader) {
	shader.setUniform(lightID + ".enabled", false);
}

void Light::enable(Shader& shader) {
	shader.setUniform(lightID + ".enabled", true);
}

void
Light::setAmb(const glm::vec3& ind)
{
	ambient = ind;
}

void
Light::setDiff(const glm::vec3& ind)
{
	diffuse = ind;
}

void
Light::setSpec(const glm::vec3& ind)
{
	specular = ind;
}

DirLight::DirLight(int id)
 : Light("dirLights", id)
{
}

void DirLight::upload(Shader& shader, glm::dmat4 const& modelViewMat) const
{
	Light::upload(shader, modelViewMat);
	shader.setUniform(lightID + ".direction", vec3(modelViewMat * posDir));
}

PosLight::PosLight(int id)
 : Light("posLights", id)
{
}


void PosLight::upload(Shader& shader, glm::dmat4 const& modelViewMat) const
{
	Light::upload(shader, modelViewMat);

	shader.setUniform(lightID + ".position", vec3(modelViewMat * posDir));
	shader.setUniform(lightID + ".constant", constant);
	shader.setUniform(lightID + ".linear", linear);
	shader.setUniform(lightID + ".quadratic", quadratic);
}

SpotLight::SpotLight(const glm::vec3& pos, int id)
{
	lightID = "spotLights[" + std::to_string(id) + "]";
	posDir = glm::vec4(pos, 1.0);
};

void SpotLight::upload(Shader& shader, glm::dmat4 const& modelViewMat) const
{
	PosLight::upload(shader, modelViewMat);

	shader.setUniform(lightID + ".direction", vec3(modelViewMat * vec4(direction, 0.0)));
	shader.setUniform(lightID + ".cutOff", cutoff);
	shader.setUniform(lightID + ".outerCutOff", outerCutoff);
	shader.setUniform(lightID + ".exponent", exp);
}
