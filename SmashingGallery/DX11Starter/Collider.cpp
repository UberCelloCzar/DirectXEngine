#include "Collider.h"
#include "GameObject.h"


Collider::Collider()
{
}

bool Collider::collidesWith(GameObject &object, GameObject &other)
{
	XMVECTOR objPos = XMLoadFloat3(&object.GetPosition());
	XMVECTOR othPos = XMLoadFloat3(&other.GetPosition());

	XMVECTOR dist = XMVectorSubtract(objPos, othPos); // Distance between centers

	if (XMVectorGetX(XMVector3Dot(dist, dist)) > ((object.GetScale().x + other.GetScale().x)*(object.GetScale().x + other.GetScale().x)))
	{
		return false; // If the distance between sphere centers is not less than the radiuses, they are not colliding
	}
	return true; // Otherwise, they're colliding
}

bool Collider::checkBounds(GameObject &object) // Returns true if out of bounds
{
	XMFLOAT3 pos = object.GetPosition();
	if (pos.y > 15 || pos.y < -15 || pos.x > 15 || pos.x < -15 || pos.z > 15 || pos.z < -15) // Check against greater bounds
	{
		return true; // Out of bounds
	}
	return false; // In bounds
}


Collider::~Collider()
{
}
