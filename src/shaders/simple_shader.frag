#version 450

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform Push {
    mat2 m_transform;
    vec2 m_offset;
    vec3 m_color;
} push;

void main() {
    outColor = vec4(push.m_color, 1.0);
}
