#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

 layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
  mat4 m_transform;
  vec3 m_color;
} push;

void main() {
    gl_Position = push.m_transform * vec4(position, 1.0);
    fragColor = color;
}
