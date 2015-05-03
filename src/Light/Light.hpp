#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/glm.hpp>

struct Light {
	glm::vec4 position;
	glm::vec4 color;
	float radius;
	float brightness;
	float fade;
};

struct LightArray {
	int light_count;
	int _padding0[3];
	Light lights[128];
};

#endif