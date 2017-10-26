#pragma once

#include <DirectXMath.h>

// --------------------------------------------------------
// A custom light definition
//
// You will eventually ADD TO this, and/or make more of these!
// --------------------------------------------------------
struct DirectionalLight
{
	DirectX::XMFLOAT4 AmbientColor;
	DirectX::XMFLOAT4 DiffuseColor;
	DirectX::XMFLOAT3 Direction;
};