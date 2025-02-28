#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float fragLight;
layout(location = 2) in float fragAO;
layout(location = 3) in vec3 fragPosition;


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
    vec4 sampledColor = texture(texSampler, fragTexCoord); // Sample the texture

    vec3 lightScale = vec3(fragLight / 16, fragLight / 16, fragLight / 16);
    vec3 sunlightIntensity = max(lightScale, 0.02f);

    vec4 aoColor = fragAO * sampledColor;

    vec2 triangleCoord = fract(fragTexCoord * 2.0); // Adjust multiplier to control pattern

    if ((fragPosition.x == 0 || fragPosition.x == 16
    || fragPosition.z == 16 || fragPosition.z == 0
    || fragPosition.y == 0 || fragPosition.y == 16)
    && triangleCoord.x + triangleCoord.y > 0.9f &&
    triangleCoord.x + triangleCoord.y < 1.1f) {
        // Apply red border
        outColor = vec4(1.0, 0.0, 0.0, 1.0); // Bright red for border
    } else {
        // Normal rendering
        outColor = vec4(aoColor.rgb * sunlightIntensity, 1.0);
    }



//    // Apply ambient occlusion
//    vec4 aoColor = fragAO * sampledColor;
////    outColor = vec4(redScale, 1.f);
//    outColor = vec4(aoColor.rgb * sunlightIntensity, 1.0); // Output final color
////    outColor = vec4(aoColor.rgb, 1.0); // Output final color


}