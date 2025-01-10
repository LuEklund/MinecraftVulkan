#version 450

layout(location = 0) in vec3 positions;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 ProjectionViewMatrix;
    vec3 directionalLight;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;

const vec3 lightDir = normalize(vec3(1.f, 0.f, 0.f));
const float ambient = 0.1;

void main()
{
    gl_Position = ubo.ProjectionViewMatrix * push.modelMatrix * vec4(positions, 1.0);

    vec3 normaWorldSpace = normalize(vec3(push.modelMatrix) * normal);

    float intensity = ambient + max(dot(normaWorldSpace, ubo.directionalLight), 0);

    fragColor = intensity * color;

}