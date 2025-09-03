#version 330 core

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 Normal;
in vec2 TexCoord;                  // smooth = perspective
noperspective in vec2 affineTexCoord; // affine
in vec3 WorldPos;

uniform vec3  fogColor = vec3(0.5, 0.4, 0.3);
uniform float fogStart = 10.0;
uniform float fogEnd = 50.0;

uniform vec3 cameraPos;
uniform sampler2D texture_diffuse1;
uniform float affineStrength = 0.1; // 0.0 = correct, 1.0 = PS1 affine

uniform DirLight dirLight;

out vec4 FragColor;

float linearizeDepth(float d, float zNear, float zFar) {
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

float calculateFogFactor() {
    float dist = length(WorldPos - cameraPos);
    float fogRange = fogEnd - fogStart;
    float fogDist = fogEnd - dist;
    float fogFactor = fogDist / fogRange;
    return clamp(fogFactor, 0.0, 1.0);
}

vec3 calculateDirectionalLight(DirLight light, vec3 normal, vec2 texCoords) {
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, texCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, texCoords));

    return (ambient + diffuse);
}

void main() {
    // Blend between both interpolations
    vec2 finalUV = mix(TexCoord, affineTexCoord, affineStrength);
    vec4 fragColor = texture(texture_diffuse1, TexCoord);

    if (fragColor.a < 0.8)
        discard;

    vec3 outputColor = calculateDirectionalLight(dirLight, Normal, TexCoord);
    vec3 fogOutColor = mix(fogColor, outputColor, calculateFogFactor());

    FragColor = vec4(fogOutColor, fragColor.a);
}
