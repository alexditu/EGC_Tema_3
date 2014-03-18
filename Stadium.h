#pragma once
#include "VerticesAndColorStruct.h"
#include <vector>
using namespace std;

class Stadium
{
public:
	vector <VerticesAndColor> vertices;
	vector <glm::uvec3> indices;
	float w, h, l;
	glm::vec3 color;
	unsigned int mesh_vao, mesh_vbo, mesh_ibo, mesh_num_indices;
	
public:
	Stadium(void);
	Stadium(float w, float h, float l, glm::vec3 color);
	~Stadium(void);
	void createStadium();
	vector<VerticesAndColor> getVertices();
	vector<glm::uvec3> getIndices();
	float getWidth();
	float getLength();
};

