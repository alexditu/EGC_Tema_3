#pragma once
class Enemy
{
public:
	unsigned int vao, vbo, ibo, num_indices;
	float alpha, betha, radius, x, y, z;
	int lap;
	float lap_count;
	float speed;
public:
	Enemy(void);
	~Enemy(void);
	unsigned int getVao();
};

