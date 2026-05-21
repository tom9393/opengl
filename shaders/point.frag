#version 450 core
layout(location = 0) in vec4 v_Color;
layout(location = 0) out vec4 o_Color;

void main() {
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    if (dot(coord, coord) > 1.0)
        discard;
    o_Color = v_Color;
}
