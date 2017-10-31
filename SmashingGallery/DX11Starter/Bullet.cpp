#include "Bullet.h"



Bullet::Bullet()
{
	isActive = false;
}


Bullet::~Bullet()
{
}

void Bullet::Start(GameObject* parent)
{
	gameObject = parent;
}

void Bullet::Update()
{

}
