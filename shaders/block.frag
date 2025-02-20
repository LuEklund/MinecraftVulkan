#version 450

layout (location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in float fragAO;


layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;





void main()
{
    //	outColor = texture(texSampler, fragTexCoord);


    vec4 sampledColor = texture(texSampler, fragTexCoord); // Sample the texture

    // Apply ambient occlusion
    // AO darkens the fragment based on the factor (0.0 = fully shadowed, 1.0 = unshadowed)
    vec4 aoColor = fragAO * sampledColor;

    outColor = vec4(aoColor.rgb, 1.0); // Output final color

}