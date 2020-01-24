#version 450 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in uint color;

layout(location = 0) out struct {
    vec4 Color;
    vec2 Uv;
} Out;

void main() {
    uvec4 colorVec = uvec4(color >> 24, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    Out.Color = colorVec.wzyx / vec4(255);

    Out.Uv = uv;
    gl_Position = vec4(pos.x / 800, pos.y / 600, 0, 1);
}