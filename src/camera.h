#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <GLFW/glfw3.h>

struct camera {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::quat rotationQuat;
    float fov;
    float aspectRatio;
};

constexpr glm::vec3 forward(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 right(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 up(0.0f, 0.0f, 1.0f);

glm::mat4 calculateViewProjection(camera c);
void updateCamera(camera* c, GLFWwindow* window);