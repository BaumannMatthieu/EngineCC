#version 450 core

in vec4 vert_color;
in vec2 vert_texcoords;

layout(location=0) out vec4 frag_color;

uniform sampler2D texture;

void main() {
	frag_color = vert_color;
}