#include "Stadium.h"
#define PI 3.14159265358979323846

Stadium::Stadium(void)
{
}


Stadium::~Stadium(void)
{
}

Stadium::Stadium(float w, float h, float l, glm::vec3 color) {
	this->w = w;
	this->h = h;
	this->l = l;
	this->color = color;
	createStadium();
}

void Stadium::createStadium() {

	glm::vec3 C1(0,0,l), C2(0,0,-l);
	int c1, c2;
	float r = w;
	int N = 20;

	float angle = 0;
	VerticesAndColor vc;
	
	for (int i = 0; i < N; i++) {
		if (i == N-1) {
			angle = PI;
		} else {
			angle = (PI/(N-1)) * (float)i;
		}
		float x, y, z;
		x = C1.x + r * cos(angle);
		y = h;
		z = C1.z + r * sin(angle);
		vc.pos = glm::vec3(x,y,z);
		vc.color = color;
		vertices.push_back(vc);
	}


	angle = PI;
	for (int i = 0; i < N; i++) {		
		float x, y, z;
		x = C2.x + r * cos(angle);
		y = h;
		z = C2.z + r * sin(angle); 
		vc.pos = glm::vec3(x,y,z);
		vc.color = color;
		vertices.push_back(vc);
		angle += PI/(N-1);
		if (i == N - 1) {
			angle = 2*PI;
		}
	}

	vc.pos = C1;
	vc.color = color;
	vertices.push_back(vc);
	c1 = vertices.size() - 1;

	vc.pos = C2;
	vc.color = color;
	vertices.push_back(vc);
	c2 = vertices.size() - 1;

	for (int i = 0; i < N - 1; i++) {
		float x,y,z;
		x = i;
		y = c1;
		z = i + 1;
		indices.push_back(glm::uvec3(x, y, z));
	}

	indices.push_back(glm::uvec3(N-1,0, 2*N - 1));
	indices.push_back(glm::uvec3(2*N - 1, N, N-1));

	for (int i = N; i < 2*N - 1; i++) {
		float x,y,z;
		x = i;
		y = c2;
		z = i + 1;
		indices.push_back(glm::uvec3(x, y, z));
	}

}

vector<VerticesAndColor> Stadium::getVertices() {
	return vertices;
}
vector<glm::uvec3> Stadium::getIndices() {
	return indices;
}

float Stadium::getWidth() {
	return w;
}

float Stadium::getLength() {
	return l;
}