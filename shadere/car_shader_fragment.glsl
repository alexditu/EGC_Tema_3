#version 330
layout(location = 0) out vec4 out_color;

uniform int color;
in vec3 vertex_to_fragment_color;

void main(){

	if (color == 0) {
		out_color = vec4(vertex_to_fragment_color.y, 0, 0, 1);
	} else {
		if (color == 1) {
			out_color = vec4(0, vertex_to_fragment_color.y, 0, 1);
		} else {
			out_color = vec4(0, 0, vertex_to_fragment_color.y, 1);
		}
	}
}