#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 UV;

layout(location = 0) out vec2 FragUV;

void main() {
    gl_Position = vec4(inPosition, 0, 1.0);
    FragUV = UV;
}

//
//#version 450
//
//// Declare output color going to the fragment shader
//layout(location = 0) out vec4 fragColor;
//
//void main() {
//    vec2 positions[4] = vec2[](
//    vec2(-0.5, -0.5), // Bottom-left
//    vec2( 0.5, -0.5), // Bottom-right
//    vec2( 0.5,  0.5), // Top-right
//    vec2(-0.5,  0.5)  // Top-left
//    );
//
//    vec4 colors[4] = vec4[](
//    vec4(1.0, 0.0, 0.0, 1.0), // Red
//    vec4(0.0, 1.0, 0.0, 1.0), // Green
//    vec4(0.0, 0.0, 1.0, 1.0), // Blue
//    vec4(1.0, 1.0, 0.0, 1.0)  // Yellow
//    );
//
//    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0); // Hardcode vertex positions
//    fragColor = colors[gl_VertexIndex]; // Hardcode vertex colors
//}
