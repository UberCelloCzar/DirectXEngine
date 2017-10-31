#pragma once
#include <DirectXMath.h>
class GameObject; // Eliminates circular dependency issue
using namespace DirectX;

class Collider
{
public:
	Collider(void);

	bool collidesWith(GameObject &object, GameObject &other); // Checks if this object collides with another, returns 0=no collision, 1=right collision, 2=left collision, 3=up collision, 4=down collision, [5=top collision, 6=bottom collision (these are for later 3d use and not used in this project)] 
	bool checkBounds(GameObject &object); // Checks if object is igoing out of bounds or not

	~Collider(void);

private:
	XMFLOAT3 dist; // Distance between centers of colliders being checked
	float distanceSq; // Unrelated to dist, this is the square of the distance between a sphere collider's center and the closest point on an aabb collider
};

