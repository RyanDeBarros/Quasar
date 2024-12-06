#version 440 core

layout(location=0) out vec4 oColor;

uniform float uTime;
uniform vec4 uColor1 = vec4(0.0, 0.0, 0.0, 1.0);
uniform vec4 uColor2 = vec4(1.0, 1.0, 1.0, 1.0);

void main() {
    float pattern = mod(gl_FragCoord.x + gl_FragCoord.y + uTime * 10.0, 20.0) < 10.0 ? 1.0 : 0.0;
    oColor = mix(uColor1, uColor2, pattern);
    //oColor = vec4(1.0, 1.0, 1.0, 1.0);
}