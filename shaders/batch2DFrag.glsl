#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture2D;

void main()
{
    FragColor = texture(texture2D, TexCoords);
}