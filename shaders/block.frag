#version 450

layout (location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in float fragAO;


layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;




//void main()
//{
//    vec4 sampledColor = texture(texSampler, fragTexCoord); // Sample the texture
//
//    // Apply debug visualization for AO:
//    // Compare smaller AO values first for better clarity in shading transitions.
//
//    if (fragAO <= 0.2) {
//        // Lowest AO (strong occlusion) - Debug as red
//        outColor = vec4(1.0, 0.0, 0.0, 1.0); // Bright red for strong occlusion
//    } else if (fragAO <= 0.4) {
//        // Moderate occlusion
//        outColor = vec4(0.7 * sampledColor.rgb * fragAO, 1.0); // Darkened more
//    } else if (fragAO <= 0.7) {
//        // Slight occlusion
//        outColor = vec4(sampledColor.rgb * fragAO * 0.7, 1.0); // Slightly darkened
//    } else {
//        // Fully lit or almost fully lit
//        outColor = vec4(sampledColor.rgb * fragAO, 1.0); // Regular AO fade
//    }
//}

void main()
{
    //	outColor = texture(texSampler, fragTexCoord);


    vec4 sampledColor = texture(texSampler, fragTexCoord); // Sample the texture

    // Apply ambient occlusion
    // AO darkens the fragment based on the factor (0.0 = fully shadowed, 1.0 = unshadowed)
    vec4 aoColor = fragAO * sampledColor;

    outColor = vec4(aoColor.rgb, 1.0); // Output final color

}