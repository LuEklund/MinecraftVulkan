#version 450

layout(location = 0) in vec3 positions;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
    mat4 transform;
    mat4 modelMatrix;
} push;

const vec3 lightDir = normalize(vec3(1.f, 0.f, 0.f));
const float ambient = 0.1;

void main()
{
    // gl_Position = vec4(push.transform * positions + push.offset, 0.0, 1.0);
    gl_Position = push.transform * vec4(positions, 1.0);
    fragColor = color;
    // fragColor = vec3(1.0, 1.0, 1.0);
    // gl_PointSize = 1.f;
    vec3 normaWorldSpace = normalize(vec3(push.modelMatrix) * normal);

    float intensity = ambient + max(dot(normaWorldSpace, lightDir), 0);

    fragColor = intensity * color;

}