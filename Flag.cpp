#include "Flag.h"


Flag::Flag(void)
{
}


Flag::~Flag(void)
{
}
Flag::Flag(int n, int m) {
	this->n = n;
	this->m = m;

	createObject();
}

void Flag::createObject() {
	VerticesAndColor vc;
	glm::vec3 color[2];
	color[0] = glm::vec3(1,0,0);
	color[1] = glm::vec3(0,0,1);
	int cl = 1;
	glm::vec3 pos;
	vc.color = glm::vec3(1,0,0);
	float init = -5;
	float pas;
	pas = 10.0 / n;

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {

			vc.pos = glm::vec3(init +  pas * j, init + pas * i, 0);
			vertices.push_back(vc);

		}
	}

	//for (int i = n - 1; i >= 0; i--) {
	//	for (int j = m - 1; j >= 0; j--) {

	//		if (i % 2 == 0) {
	//			cl = (cl + 1) % 2;
	//			vc.color = color[cl];
	//		}
	//		vc.pos = glm::vec3(j, i, 0);
	//		vertices.push_back(vc);
	//		
	//	}
	//}

	for (int i = 0; i < (n-1) * m; i++) {
		if (i % m != m-1) {
			indices.push_back(glm::uvec3(i, i+m, i+m+1));
			indices.push_back(glm::uvec3(i, i+m+1, i+1));
		}
	}
}
