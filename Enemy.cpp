#include "Enemy.h"


Enemy::Enemy(void)
{
	lap = -1;
}


Enemy::~Enemy(void)
{
}

unsigned int Enemy::getVao() {
	return vao;
}
