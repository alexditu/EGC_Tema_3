#pragma once
#include "VerticesAndColorStruct.h"
#include <vector>
using namespace std;
class Flag
{
public:
	vector<glm::uvec3> indices;
	vector<VerticesAndColor> vertices;
	int n, m;
	unsigned int vao, vbo, ibo, num_indices;


public:
	Flag(void);
	Flag(int n, int m);
	~Flag(void);
	void createObject();
};

