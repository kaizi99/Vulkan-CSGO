#include "camera.h"

#include <glm/gtx/quaternion.hpp>

glm::mat4 calculateViewProjection(camera c)
{
	glm::mat4 rotationMatrix = glm::toMat4(-c.rotation);
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), -c.position);
	glm::mat4 projectionMatrix = glm::perspectiveRH(glm::radians(c.fov), c.aspectRatio, 0.1f, 10000.0f);

	return projectionMatrix * translationMatrix * rotationMatrix;
}

void updateCamera(camera* c, GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		c->position += glm::vec3(0.0f, -1.0f, 0.0f);
	}	
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		c->position += glm::vec3(0.0f, 1.0f, 0.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		c->position += glm::vec3(-1.0f, 0.0f, 0.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		c->position += glm::vec3(1.0f, 0.0f, 0.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		c->position += glm::vec3(0.0f, 0.0f, -1.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		c->position += glm::vec3(0.0f, 0.0f, 1.0f);
	}
}