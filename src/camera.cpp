#include "camera.h"
#include "dearimgui/imgui.h"

#include <glm/gtx/quaternion.hpp>
#include <iostream>

glm::mat4 calculateViewProjection(camera c)
{
    glm::vec3 f = glm::rotate(c.rotationQuat, forward);

    glm::mat4 viewMatrix = glm::lookAt(c.position, c.position + f, glm::rotate(c.rotationQuat, up));
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(c.fov), c.aspectRatio, 0.1f, 10000.0f);
	projectionMatrix[1][1] *= -1;

	return projectionMatrix * viewMatrix;
}

void updateCamera(camera* c, GLFWwindow* window) {
    glm::vec3 moveVector(0.0f);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        moveVector += up * 5.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        moveVector += -up * 5.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        moveVector += right * 5.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        moveVector += -right * 5.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        moveVector += forward * 5.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        moveVector += -forward * 5.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        c->rotation += up * 0.05f;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        c->rotation += -up * 0.05f;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        c->rotation += -right * 0.05f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        c->rotation += right * 0.05f;
    }

    c->rotationQuat = glm::quat(c->rotation);

    ImGui::Begin("Camera");
    ImGui::Text("Position:");
    ImGui::Text("x: %f", c->position.x);
    ImGui::Text("y: %f", c->position.y);
    ImGui::Text("z: %f", c->position.z);
    ImGui::Text("Rotation:");
    ImGui::Text("x: %f", c->rotation.x);
    ImGui::Text("y: %f", c->rotation.y);
    ImGui::Text("z: %f", c->rotation.z);
    ImGui::End();

    c->position += glm::rotate(c->rotationQuat, moveVector);
}