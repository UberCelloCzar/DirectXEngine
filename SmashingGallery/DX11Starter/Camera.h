#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <DirectXMath.h>
#include "Vertex.h"
using namespace DirectX;

class Camera
{
public:
	Camera(unsigned int width, unsigned int height);
	~Camera();
	void Update();

	/*Getters*/
	XMFLOAT4X4 GetViewMatrix();
	XMFLOAT4X4 GetProjectionMatrix();
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetDirection();
	XMFLOAT4 GetRotationQuaternion();

	/*Transforms*/
	int Translate(float xOffset, float yOffset, float zOffset); // Translate by an amount
	int Rotate(float xEuler, float yEuler, float zEuler); // Rotate by an amount

	int SetPosition(float x, float y, float z); // Set the position
	int SetRotation(float xEuler, float yEuler, float zEuler); // Set the rotation

	int MoveRelative(float forward, float strafe, float vert); // Move relative to the direction faced
	int MouseRotate(int mouseXOffset, int mouseYOffset); // Rotate using mouse movement

	int UpdateProjectionMatrix(unsigned int width, unsigned int height);

private:

	/*Transform Vars*/
	XMFLOAT4X4 viewMatrix; // Keeps track of the view into the 3D scene
	XMFLOAT4X4 projectionMatrix; // Maps the 3D scene to a 2D screen
	XMFLOAT3 position; // The camera's position
	XMFLOAT3 direction; // Normalized forward vector representing view direction
	XMFLOAT4 rotationQuaternion; // Rotation around each axis in degrees, Z axis will always be 0 for first person
	XMFLOAT3 up; // The camera's up vector
	bool changed; // Whether a transform parameter has been modified this frame
};

