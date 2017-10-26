#include "Camera.h"
#include <iostream>

Camera::Camera(unsigned int width, unsigned int height)
{
	position = XMFLOAT3(0.0f, 0.0f, -5.0f);
	direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMStoreFloat4(&rotationQuaternion, XMQuaternionIdentity());
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(W));
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(W));
	up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	UpdateProjectionMatrix(width, height);
	changed = true;
}

/*Getters*/
XMFLOAT4X4 Camera::GetViewMatrix()
{
	return viewMatrix;
}
XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projectionMatrix;
}
XMFLOAT3 Camera::GetPosition()
{
	return position;
}
XMFLOAT3 Camera::GetDirection()
{
	return direction;
}
XMFLOAT4 Camera::GetRotationQuaternion()
{
	return rotationQuaternion;
}

/*Transforms*/
int Camera::Translate(float xOffset, float yOffset, float zOffset) // Translate by an amount
{
	position.x += xOffset;
	position.y += yOffset;
	position.z += zOffset;
	changed = true;
	return 1;
}

int Camera::Rotate(float xEulerOffset, float yEulerOffset, float zEulerOffset) // Rotate by an amount
{
	XMStoreFloat4(&rotationQuaternion, XMQuaternionNormalize(XMQuaternionMultiply(XMQuaternionRotationRollPitchYaw(xEulerOffset, yEulerOffset, zEulerOffset), XMLoadFloat4(&rotationQuaternion))));
	XMStoreFloat3(&direction, XMVector3Rotate(XMLoadFloat3(&direction), XMLoadFloat4(&rotationQuaternion)));
	changed = true;
	return 1;
}

int Camera::SetPosition(float x, float y, float z) // Set the position
{
	position = XMFLOAT3(x, y, z);
	changed = true;
	return 1;
}

int Camera::SetRotation(float xEuler, float yEuler, float zEuler) // Set the rotation
{
	XMStoreFloat4(&rotationQuaternion, XMQuaternionRotationRollPitchYaw(xEuler, yEuler, zEuler));
	XMStoreFloat3(&direction, XMVector3Rotate(XMLoadFloat3(&direction), XMLoadFloat4(&rotationQuaternion)));
	changed = true;
	return 1;
}

int Camera::MoveRelative(float forward, float strafe, float vert) // Move relative to the direction faced
{
	XMVECTOR total = XMVectorScale(XMLoadFloat3(&direction), forward); // Add in the direction faced
	total = XMVectorAdd(total, XMVectorScale(XMVector3Cross(XMLoadFloat3(&up), XMLoadFloat3(&direction)), strafe)); // Calculate the right vector then scale it and add it
	XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), total));
	position.y += vert;
	//std::cout << position.x << ", " << position.y << ", " << position.z << std::endl;
	changed = true;
	return 1;
}

int Camera::MouseRotate(int mouseXOffset, int mouseYOffset) // Rotate using mouse movement
{
	// rotQuat = normalize(quatRPY * rotQuat)
	// direction = direction rotated by rotQuat
	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(mouseYOffset*((float)(.005)), mouseXOffset*((float)(.005)), 0.0f);
	XMStoreFloat3(&direction, XMVector3Rotate(XMLoadFloat3(&direction), quat));
	XMStoreFloat4(&rotationQuaternion, XMQuaternionNormalize(XMQuaternionMultiply(quat, XMLoadFloat4(&rotationQuaternion))));
	changed = true;
	return 1;
}

int Camera::UpdateProjectionMatrix(unsigned int width, unsigned int height)
{
	XMStoreFloat4x4(&projectionMatrix, XMMatrixTranspose(XMMatrixPerspectiveFovLH(0.25f*3.1415926535f, (float)width/height,	0.1f, 100.0f))); // Update with new w/h
	return 1;
}

void Camera::Update()
{
	if (changed)
	{
		XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(XMMatrixLookToLH(XMLoadFloat3(&position), XMLoadFloat3(&direction), XMLoadFloat3(&up)))); // Update the view matrix if the camera has changed
		changed = false;
	}
}


Camera::~Camera()
{
}
