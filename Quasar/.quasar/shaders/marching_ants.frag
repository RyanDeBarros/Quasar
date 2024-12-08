#version 440 core

layout(location=0) out vec4 oColor;

in vec2 tAntDir;

uniform float uTime;
uniform vec4 uColor1 = vec4(0.0, 0.0, 0.0, 1.0);
uniform vec4 uColor2 = vec4(1.0, 1.0, 1.0, 1.0);
uniform float uSpeed = 10.0;
uniform float uAntLength = 10.0;

void main() {
    if (mod(gl_FragCoord.x * tAntDir.x + gl_FragCoord.y * tAntDir.y + uTime * uSpeed, 2 * uAntLength) < uAntLength)
        oColor = uColor1;
    else
        oColor = uColor2;
}
