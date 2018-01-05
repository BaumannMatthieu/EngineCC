#version 450 core

in vec4 vert_color;
in vec3 vert_texcoords;

in vec2 frag_coord;

layout(location=0) out vec4 frag_color;

void main() {
	float size = 1;
	if(abs(size*frag_coord.x - round(size*frag_coord.x)) < size*0.05) {
		frag_color = vec4(1, 0, 0, 1);
	} else if(abs(size*frag_coord.y - round(size*frag_coord.y)) < size*0.05) {
		frag_color = vec4(1, 0, 0, 1);
	} else {
		frag_color = vec4(1, 1, 1, 0);
	}
}