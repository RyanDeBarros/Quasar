#version 440 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

uniform float uOutlineWidth = 8.0;
uniform vec2 uScreenSize;

in float tLineOrientation[];

out vec2 tAntDir;

void main() {
    if (tLineOrientation[0] == 0) {
        EndPrimitive();
        return;
    }

    vec2 p0 = gl_in[0].gl_Position.xy;
    vec2 p1 = gl_in[1].gl_Position.xy;
    
    tAntDir = normalize(p1 - p0);
    vec2 lineThickness = vec2(uOutlineWidth) / uScreenSize;
    vec2 perpendicular = vec2(-tAntDir.y, tAntDir.x) * lineThickness * 0.5;
    
    gl_Position.xy = p0 - (1 + tLineOrientation[0]) * perpendicular;
    EmitVertex();
    gl_Position.xy = p0 + (1 - tLineOrientation[0]) * perpendicular;
    EmitVertex();
    gl_Position.xy = p1 - (1 + tLineOrientation[0]) * perpendicular;
    EmitVertex();
    gl_Position.xy = p1 + (1 - tLineOrientation[0]) * perpendicular;
    EmitVertex();
    EndPrimitive();
}
