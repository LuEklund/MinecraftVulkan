#version 450

layout(location = 0) in vec3 positions;
layout(location = 1) in vec2 uv;
layout(location = 2) in float light;
layout(location = 3) in float aoFactor;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out float fragLight;
layout(location = 2) out float fragAO;
layout(location = 3) out vec3 fragPosition;


layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 ProjectionViewMatrix;
    vec3 directionalLight;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;


void main()
{
    gl_Position = ubo.ProjectionViewMatrix * push.modelMatrix * vec4(positions, 1.0);

    fragTexCoord = uv;
    fragLight = light;
    fragAO = aoFactor;
    fragPosition = positions;
}