#version 450 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in uint color;

layout(push_constant) uniform PushConsantBlock {
    vec2 scale;
    vec2 translate;
} PushConstant;

layout(location = 0) out struct {
    vec4 Color;
    vec2 Uv;
} Out;

void main() {
    uvec4 colorVec = uvec4(color >> 24, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    Out.Color = colorVec.wzyx / vec4(255);

    Out.Uv = uv;
    gl_Position = vec4(pos * PushConstant.scale + PushConstant.translate, 0, 1);
}