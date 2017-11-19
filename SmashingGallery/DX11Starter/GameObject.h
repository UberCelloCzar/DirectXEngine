#pragma once
#include "Mesh.h"
#include "Material.h"
#include <d3d11.h>
#include <string>
#include <DirectXMath.h>
#include "Collider.h"
#include "Script.h"
#include <vector>

using namespace DirectX;

class GameObject
{
public:
	GameObject(Mesh* meshPtr, Material* matPtr, std::vector<Script*> pScripts);
	~GameObject();

	/*Getters*/
	XMFLOAT4X4 GetWorldMatrix();
	XMFLOAT3 GetPosition();
	XMFLOAT4 GetRotationQuaternion();
	XMFLOAT3 GetScale();
	XMFLOAT3 GetVelocity();

	bool GetChanged();
	
	/*Transform Methods*/
	int Translate(float xOffset, float yOffset, float zOffset); // Translate by an amount
	int Rotate(float xEuler, float yEuler, float zEuler); // Rotate by an amount
	int Scale(float xScale, float yScale, float zScale); // Scale by an amount

	int SetPosition(float x, float y, float z); // Set the position
	int SetRotation(float xEuler, float yEuler, float zEuler); // Set the rotation
	int SetScale(float x, float y, float z); // Set the scale

	int SetVelocity(float x, float y, float z); // Set the velocity
	int AddVelocity(float x, float y, float z); // Add to the existing velocity

	/*Components*/
	Collider collider;
	std::vector<Script*> scripts;

	void Update(float deltaTime);
	int Draw(ID3D11DeviceContext* context);
	int CalculateWorldMatrix(); // Recalculates the world matrix



protected:

	/*Transform Vars*/
	XMFLOAT4X4 worldMatrix; // The current transform, calculated using pos, rot, and sca
	XMFLOAT3 position;
	XMFLOAT4 rotationQuaternion;
	XMFLOAT3 scale;
	XMFLOAT3 velocity;
	bool changed; // Whether a transform parameter has been modified this frame

	/*Rendering Vars*/
	Mesh* mesh;
	Material* material;
};

