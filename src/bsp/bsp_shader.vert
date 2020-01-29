#version 450 core

layout(location = 0) in vec3 pos;

layout(push_constant) uniform PushConsantBlock {
    mat4 mvp;
} PushConstant;

void main() {
    gl_Position = PushConstant.mvp * vec4(pos, 1);
}