// Copyright (C) 2020 Kai-Uwe Zimdars
/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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