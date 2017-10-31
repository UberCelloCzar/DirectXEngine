#include "Collider.h"



Collider::Collider()
{
}

bool Collider::collidesWith(GameObject & object, GameObject & other)
{
	return false;
}

bool Collider::checkBounds(GameObject & object)
{
	return false;
}


Collider::~Collider()
{
}
