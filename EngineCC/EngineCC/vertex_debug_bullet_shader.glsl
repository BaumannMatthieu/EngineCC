#version 450 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

uniform mat4 view;
uniform mat4 projection;

out vec4 vert_color;

void main() {
	gl_Position = projection * view * vec4(in_position, 1.0f);
	vert_color = in_color;
}