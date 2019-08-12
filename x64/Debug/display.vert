#version 420

uniform vec2 uScreen;
vec2 uTexture;

in vec4 aPosition;
in vec4 aTexCoord;
in vec4 aColor;
out vec4 v_Color0;
out vec4 v_TexCoord0;
out float v_Psize;

void main()
{
    uTexture = vec2(128.0);
    vec2 halfScreen = uScreen / 2;

    gl_Position = aPosition;
    gl_Position.x = (gl_Position.x * 1.0 / halfScreen.x) - 1;
    gl_Position.y = 1 - (gl_Position.y * 1.0 / halfScreen.y);
    gl_Position.z = 1 - (gl_Position.z * 0.000015);
    gl_Position.w = 1.0;
    v_TexCoord0.xy = vec2(1.0) / uTexture * aTexCoord.xy;
    v_Psize = 1.0;
    v_Color0 = aColor;
}