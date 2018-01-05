#version 450 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_texcoords;
layout(location = 4) in ivec4 in_id;
layout(location = 5) in vec4 in_weight;

uniform mat4 modelview;
uniform mat4 projection;

uniform mat4 bonesTransform[32];
uniform bool animated;

out vec4 vert_color;
out vec3 vert_texcoords;

void main() {
	mat4 transform = mat4(1);
	if(animated) {
		transform = bonesTransform[in_id[0]] * in_weight[0] + \
				bonesTransform[in_id[1]] * in_weight[1] + \
				bonesTransform[in_id[2]] * in_weight[2] + \
				bonesTransform[in_id[3]] * in_weight[3];
	}
	gl_Position = projection * modelview * transform * vec4(in_position, 1.0f);

	vert_color = in_color;
	vert_texcoords = in_texcoords;
}