#include "Emitter.h"

using namespace DirectX;

Emitter::Emitter(
	int maxParticles,
	int particlesPerSecond,
	float lifetime,
	float startSize,
	float endSize,
	DirectX::XMFLOAT4 startColor,
	DirectX::XMFLOAT4 endColor,
	DirectX::XMFLOAT3 startVelocity,
	DirectX::XMFLOAT3 emitterPosition,
	DirectX::XMFLOAT3 emitterAcceleration,
	ID3D11Device* device,
	SimpleVertexShader* vs,
	SimplePixelShader* ps,
	ID3D11ShaderResourceView* texture
)
{
	this->vs = vs;
	this->ps = ps;
	this->texture = texture;

	this->maxParticles = maxParticles;
	this->lifetime = lifetime;
	this->startColor = startColor;
	this->endColor = endColor;
	this->startVelocity = startVelocity;
	this->startSize = startSize;
	this->endSize = endSize;
	this->particlesPerSecond = particlesPerSecond;
	this->secondsPerParticle = 1.0f / particlesPerSecond;

	this->emitterPosition = emitterPosition;
	this->emitterAcceleration = emitterAcceleration;

	timeSinceEmit = 0;
	livingParticleCount = 0;
	firstAliveIndex = 0;
	firstDeadIndex = 0;

	particles = new Particle[maxParticles];

	localParticleVertices = new ParticleVertex[4 * maxParticles];
	for (int i = 0; i < maxParticles * 4; i += 4)
	{
		localParticleVertices[i + 0].UV = XMFLOAT2(0, 0);
		localParticleVertices[i + 1].UV = XMFLOAT2(1, 0);
		localParticleVertices[i + 2].UV = XMFLOAT2(1, 1);
		localParticleVertices[i + 3].UV = XMFLOAT2(0, 1);
	}

	D3D11_BUFFER_DESC vertBuffDesc = {};
	vertBuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertBuffDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertBuffDesc.ByteWidth = sizeof(ParticleVertex) * 4 * maxParticles;
	device->CreateBuffer(&vertBuffDesc, 0, &vertexBuffer);

	unsigned int* indices = new unsigned int[maxParticles * 6];
	int indexCount = 0;
	for (int i = 0; i < maxParticles * 4; i += 4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * maxParticles * 6;
	device->CreateBuffer(&ibDesc, &indexData, &indexBuffer);

	delete[] indices;
}


Emitter::~Emitter()
{
	delete[] particles;
	delete[] localParticleVertices;
	vertexBuffer->Release();
	indexBuffer->Release();
}

void Emitter::Update(float dt)
{
	if (firstAliveIndex < firstDeadIndex)
	{
		for (int i = firstAliveIndex; i < firstDeadIndex; i++)
			UpdateSingleParticle(dt, i);
	}
	else
	{
		for (int i = firstAliveIndex; i < maxParticles; i++)
			UpdateSingleParticle(dt, i);

		for (int i = 0; i < firstDeadIndex; i++)
			UpdateSingleParticle(dt, i);
	}

	timeSinceEmit += dt;

	while (timeSinceEmit > secondsPerParticle)
	{
		SpawnParticle();
		timeSinceEmit -= secondsPerParticle;
	}
}

void Emitter::UpdateSingleParticle(float dt, int index)
{
	if (particles[index].Age >= lifetime)
		return;

	particles[index].Age += dt;
	if (particles[index].Age >= lifetime)
	{
		firstAliveIndex++;
		firstAliveIndex %= maxParticles;
		livingParticleCount--;
		return;
	}

	float agePercent = particles[index].Age / lifetime;

	XMStoreFloat4(
		&particles[index].Color,
		XMVectorLerp(
			XMLoadFloat4(&startColor),
			XMLoadFloat4(&endColor),
			agePercent));

	particles[index].Size = startSize + agePercent * (endSize - startSize);


	XMVECTOR startPos = XMLoadFloat3(&emitterPosition);
	XMVECTOR startVel = XMLoadFloat3(&particles[index].StartVelocity);
	XMVECTOR accel = XMLoadFloat3(&emitterAcceleration);
	float t = particles[index].Age;

	XMStoreFloat3(
		&particles[index].Position,
		accel * t * t / 2.0f + startVel * t + startPos);
}

void Emitter::SpawnParticle()
{
	if (livingParticleCount == maxParticles)
		return;

	particles[firstDeadIndex].Age = 0;
	particles[firstDeadIndex].Size = startSize;
	particles[firstDeadIndex].Color = startColor;
	particles[firstDeadIndex].Position = emitterPosition;
	particles[firstDeadIndex].StartVelocity = startVelocity;
	particles[firstDeadIndex].StartVelocity.x += ((float)rand() / RAND_MAX) * 0.4f - 0.2f;
	particles[firstDeadIndex].StartVelocity.y += ((float)rand() / RAND_MAX) * 0.4f - 0.2f;
	particles[firstDeadIndex].StartVelocity.z += ((float)rand() / RAND_MAX) * 0.4f - 0.2f;

	firstDeadIndex++;
	firstDeadIndex %= maxParticles;

	livingParticleCount++;
}

void Emitter::CopyParticlesToGPU(ID3D11DeviceContext* context)
{
	if (firstAliveIndex < firstDeadIndex)
	{
		for (int i = firstAliveIndex; i < firstDeadIndex; i++)
			CopyOneParticle(i);
	}
	else
	{
		for (int i = firstAliveIndex; i < maxParticles; i++)
			CopyOneParticle(i);
		for (int i = 0; i < firstDeadIndex; i++)
			CopyOneParticle(i);
	}

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, localParticleVertices, sizeof(ParticleVertex) * 4 * maxParticles);

	context->Unmap(vertexBuffer, 0);
}

void Emitter::CopyOneParticle(int index)
{
	int i = index * 4;

	localParticleVertices[i + 0].Position = particles[index].Position;
	localParticleVertices[i + 1].Position = particles[index].Position;
	localParticleVertices[i + 2].Position = particles[index].Position;
	localParticleVertices[i + 3].Position = particles[index].Position;

	localParticleVertices[i + 0].Size = particles[index].Size;
	localParticleVertices[i + 1].Size = particles[index].Size;
	localParticleVertices[i + 2].Size = particles[index].Size;
	localParticleVertices[i + 3].Size = particles[index].Size;

	localParticleVertices[i + 0].Color = particles[index].Color;
	localParticleVertices[i + 1].Color = particles[index].Color;
	localParticleVertices[i + 2].Color = particles[index].Color;
	localParticleVertices[i + 3].Color = particles[index].Color;
}

void Emitter::Draw(ID3D11DeviceContext* context, Camera* camera)
{
	CopyParticlesToGPU(context);

	UINT stride = sizeof(ParticleVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vs->SetShader();
	vs->CopyAllBufferData();

	ps->SetShaderResourceView("particle", texture);
	ps->SetShader();
	ps->CopyAllBufferData();

	if (firstAliveIndex < firstDeadIndex)
	{
		context->DrawIndexed(livingParticleCount * 6, firstAliveIndex * 6, 0);
	}
	else
	{
		context->DrawIndexed(firstDeadIndex * 6, 0, 0);
		context->DrawIndexed((maxParticles - firstAliveIndex) * 6, firstAliveIndex * 6, 0);
	}

}
