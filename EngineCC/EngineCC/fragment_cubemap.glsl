#version 450 core

in vec4 vert_color;
in vec3 vert_texcoords;

layout(location=0) out vec4 frag_color;

uniform samplerCube tex;

void main() {
	frag_color = texture(tex, vert_texcoords.xyz).rgba;
	//frag_color = vec4(1);
}