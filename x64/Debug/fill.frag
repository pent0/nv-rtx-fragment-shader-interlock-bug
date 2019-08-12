#version 420
#extension GL_ARB_fragment_shader_interlock : require
layout(pixel_interlock_ordered) in;

uniform sampler2D uTex;
layout(rgba8) uniform image2D f_colorAttachment;

in vec4 v_Color0;

void main()
{
    beginInvocationInterlockARB();
    vec4 lastFragColor = imageLoad(f_colorAttachment, ivec4(gl_FragCoord).xy);
    vec4 tintedColor = v_Color0;

    vec4 blended = vec4(0);
    blended.xyz = tintedColor.www * tintedColor.xyz + (1.0 - tintedColor.www) * lastFragColor.xyz;
    blended.w = tintedColor.w * tintedColor.w;

    imageStore(f_colorAttachment, ivec4(gl_FragCoord).xy, blended);

    endInvocationInterlockARB();
}
