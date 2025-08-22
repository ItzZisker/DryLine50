#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;
layout (location = 5) in int aBoneIds[4];
layout (location = 6) in float aWeights[4];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 screenSize = vec2(320.0, 240.0);

out vec3 Normal;
out vec2 TexCoord;
noperspective out vec2 affineTexCoord; // affine

void main() {
    vec4 clip = projection * view * model * vec4(aPos, 1.0);

    // Optional pixel snapping for PS1 wobble
    vec3 ndc = clip.xyz / clip.w;
    vec2 screen = (ndc.xy * 0.5 + 0.5) * screenSize;
    screen = floor(screen + 0.5);
    vec2 snappedNDC = (screen / screenSize) * 2.0 - 1.0;
    gl_Position = vec4(snappedNDC * clip.w, clip.z, clip.w);

    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;       // perspective version (default)
    affineTexCoord = aTexCoord; // affine version (noperspective)
}
