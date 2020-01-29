#version 450 core

layout(location = 0) out vec4 color;

layout(binding=0) uniform sampler2D Texture;

layout(location=0) in struct {
    vec4 Color;
    vec2 Uv;
} In;

void main() {
    color = In.Color * texture(Texture, In.Uv.st);
}