#version 420
#extension GL_ARB_fragment_shader_interlock : require
layout(pixel_interlock_ordered) in;

layout(rgba8) uniform image2D f_colorAttachment;
// uniform sampler2D screenTexture;

out vec4 FragColor;
in vec2 TexCoords;

void main()
{
    beginInvocationInterlockARB();
    // Uncomment interlock and use these commented line still give flickering results
    //vec3 col = texture(screenTexture, TexCoords).rgb;
    //FragColor = vec4(col, 1.0);
    FragColor = imageLoad(f_colorAttachment, ivec4(gl_FragCoord).xy);
    endInvocationInterlockARB();
} 