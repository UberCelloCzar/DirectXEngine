#include "GameObject.h"
#include <iostream>

GameObject::GameObject(Mesh* meshPtr, Material* matPtr, std::vector<Script*> pScripts)
{
	mesh = meshPtr;
	material = matPtr;
	position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMStoreFloat4(&rotationQuaternion, XMQuaternionIdentity());
	scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMMATRIX W = XMMatrixIdentity();
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(W));
	changed = true;
	// Initialize collider here
	scripts = pScripts;

	if (scripts.size() > 0) // Call the update for each script each cycle
	{
		for each (Script* s in scripts)
		{
			s->Start(this);
		}
	}
}


/*Getters*/
XMFLOAT4X4 GameObject::GetWorldMatrix()
{
	return worldMatrix;
}
XMFLOAT3 GameObject::GetPosition()
{
	return position;
}
XMFLOAT4 GameObject::GetRotationQuaternion()
{
	return rotationQuaternion;
}
XMFLOAT3 GameObject::GetScale()
{
	return scale;
}
XMFLOAT3 GameObject::GetVelocity()
{
	return velocity;
}
bool GameObject::GetChanged()
{
	return changed;
}

/*Transform Methods*/
int GameObject::Translate(float xOffset, float yOffset, float zOffset) // Translate by an amount
{
	position.x += xOffset;
	position.y += yOffset;
	position.z += zOffset;
	changed = true;
	return 1;
}

int GameObject::Rotate(float xEulerOffset, float yEulerOffset, float zEulerOffset) // Rotate by an amount
{
	XMStoreFloat4(&rotationQuaternion, XMQuaternionNormalize(XMQuaternionMultiply(XMQuaternionRotationRollPitchYaw(xEulerOffset, yEulerOffset, zEulerOffset), XMLoadFloat4(&rotationQuaternion))));
	changed = true;
	return 1;
}

int GameObject::Scale(float xScaleOffset, float yScaleOffset, float zScaleOffset) // Scale by an amount
{
	scale.x += xScaleOffset;
	scale.y += yScaleOffset;
	scale.z += zScaleOffset;
	changed = true;
	return 1;
}

int GameObject::SetPosition(float x, float y, float z) // Set the position
{
	position = XMFLOAT3(x, y, z);
	changed = true;
	return 1;
}

int GameObject::SetRotation(float xEuler, float yEuler, float zEuler) // Set the rotation
{
	XMStoreFloat4(&rotationQuaternion, XMQuaternionRotationRollPitchYaw(xEuler, yEuler, zEuler));
	changed = true;
	return 1;
}

int GameObject::SetScale(float x, float y, float z) // Set the scale
{
	scale = XMFLOAT3(x, y, z);
	changed = true;
	return 1;
}

int GameObject::SetVelocity(float x, float y, float z) // Set the velocity
{
	velocity.x = x;
	velocity.y = y;
	velocity.z = z;
	return 1;
}

int GameObject::AddVelocity(float x, float y, float z) // Add to the existing velocity
{
	velocity.x += x;
	velocity.y += y;
	velocity.z += z;
	return 1;
}

void GameObject::Update(float deltaTime)
{
	if (scripts.size() > 0) // Call the update for each script each cycle
	{
		for each (Script* s in scripts)
		{
			s->Update();
		}
	}

	if (velocity.x != 0 && velocity.y != 0 && velocity.z != 0)
	{
		Translate(velocity.x*deltaTime, velocity.y*deltaTime, velocity.z*deltaTime);
	}
}

int GameObject::Draw(ID3D11DeviceContext* context)
{
	material->GetVertexShader()->SetShader(); // Don't do this here, when we have time we should optimize this by moving this to game so we're not doing a bunch of times with the same shaders
	material->GetPixelShader()->SetShader();
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	material->GetVertexShader()->SetMatrix4x4("world", worldMatrix);
	material->GetVertexShader()->CopyBufferData("perObjectData");
	material->GetPixelShader()->SetSamplerState("basicSampler", material->GetSamplerState());
	material->GetPixelShader()->SetShaderResourceView("diffuseTexture", material->GetShaderResourceView());
	ID3D11Buffer* vertBuff = mesh->GetVertexBuffer(); // Set the second object
	context->IASetVertexBuffers(0, 1, &vertBuff, &stride, &offset);
	context->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(
		mesh->GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices
	return 1;
}

int GameObject::CalculateWorldMatrix() // Recalculates the world matrix and spits out the new one (also sets the stored one)
{
	XMMATRIX world = XMMatrixScaling(scale.x, scale.y, scale.z) *
		XMMatrixRotationQuaternion(XMLoadFloat4(&rotationQuaternion)) *
		XMMatrixTranslation(position.x, position.y, position.z);
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(world));
	changed = false;
		/*std::cout << "[" << worldMatrix(0, 0) << ", " << worldMatrix(0, 1) << ", " << worldMatrix(0, 2) << ", " << worldMatrix(0, 3) << "]" << std::endl;
		std::cout << "[" << worldMatrix(1, 0) << ", " << worldMatrix(1, 1) << ", " << worldMatrix(1, 2) << ", " << worldMatrix(1, 3) << "]" << std::endl;
		std::cout << "[" << worldMatrix(2, 0) << ", " << worldMatrix(2, 1) << ", " << worldMatrix(2, 2) << ", " << worldMatrix(2, 3) << "]" << std::endl;
		std::cout << "[" << worldMatrix(3, 0) << ", " << worldMatrix(3, 1) << ", " << worldMatrix(3, 2) << ", " << worldMatrix(3, 3) << "]" << std::endl;*/
	return 1;
}

GameObject::~GameObject()
{
	mesh = NULL; // Remove the pointer
	material = NULL;
	for each (Script* s in scripts)
	{
		delete s;
	}
}
