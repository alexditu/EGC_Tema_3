#pragma once
class TpsObject
{
public:
	unsigned int vao, vbo, ibo, num_indices;
	float alpha, betha, radius, x, y, z;
public:
	TpsObject(void);
	~TpsObject(void);
};

