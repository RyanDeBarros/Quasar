#version 440 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

uniform float uOutlineWidth = 2.0;
uniform mat3 uVP;
uniform vec2 uScreenSize;

in float tExcludeNext[];

out vec2 tAntDir;

void main() {
    if (tExcludeNext[0] > 0) {
        EndPrimitive();
        return;
    }

    vec2 lineThickness = vec2(2) / uScreenSize;
    vec2 p0 = gl_in[0].gl_Position.xy;
    vec2 p1 = gl_in[1].gl_Position.xy;
    
    tAntDir = normalize(p1 - p0);
    vec2 perpendicular = normalize(vec2(-tAntDir.y, tAntDir.x)) * lineThickness * 0.5;
    
    gl_Position.xy = p0 + perpendicular;
    EmitVertex();
    
    gl_Position.xy = p0 - perpendicular;
    EmitVertex();
    
    gl_Position.xy = p1 + perpendicular;
    EmitVertex();
    
    gl_Position.xy = p1 - perpendicular;
    EmitVertex();
    
    EndPrimitive();
}
