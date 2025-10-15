#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoords;

uniform float z_static = 0.0;
uniform mat4 model;

void main()
{
    TexCoords = aTexCoord;
    vec4 pos = model * vec4(aPos, 0.0, 1.0);
    gl_Position = vec4(pos.x, pos.y, z_static, 1.0);
}