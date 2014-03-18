#version 330

layout(location = 0) in vec3 in_position;		
layout(location = 1) in vec3 in_color;		

uniform mat4 model_matrix, view_matrix, projection_matrix;
uniform float time;
uniform int win_color;

out vec3 vertex_to_fragment_color;

void main(){

	vec3 v = in_position;
	v.z = sin(v.x + time*0.5);

	vertex_to_fragment_color = vec3(0,0,0);

	if (win_color == 0) {
		vertex_to_fragment_color = vec3(1,0,0);
	}
	
	if (win_color == 1) {
		vertex_to_fragment_color = vec3(0,1,0);
	}
	if (win_color == 2) {
		vertex_to_fragment_color = vec3(0,0,1);
	}


	//vertex_to_fragment_color = in_color;
	
	gl_Position = projection_matrix*view_matrix*model_matrix*vec4(v,1);
}
