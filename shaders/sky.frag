#version 450

layout(location = 0) in vec3 forwards;

layout(set = 0, binding = 1) uniform samplerCube material;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(material, forwards);
}