#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <DirectXMath.h>
#include "Vertex.h"

class Mesh
{
public:
	Mesh(Vertex vertexArray[], int vertexArrCount, int indexArray[], int indexArrCount, ID3D11Device* device);
	Mesh(char* objFile, ID3D11Device* device);
	~Mesh();

	void FillBuffers(Vertex vertexArray[], int vertexArrCount, int indexArray[], int indexArrCount, ID3D11Device* device);

	ID3D11Buffer* GetVertexBuffer();
	ID3D11Buffer* GetIndexBuffer();
	int GetIndexCount();

private:
	ID3D11Buffer* vertexBuffer; // Pointer to the buffer of vertices
	ID3D11Buffer* indexBuffer; // Pointer to the buffer of indices of verts to be used to draw an object
	int indexCount; // Number of indices in the buffer
};

