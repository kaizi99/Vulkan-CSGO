#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <GLFW/glfw3.h>

struct camera {
	glm::vec3 position;
	glm::quat rotation;
	float fov;
	float aspectRatio;
};

glm::mat4 calculateViewProjection(camera c);
void updateCamera(camera* c, GLFWwindow* window);