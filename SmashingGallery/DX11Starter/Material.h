#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>

class Material
{
public:
	Material(SimpleVertexShader* vShader, SimplePixelShader* pShader, ID3D11ShaderResourceView* SRV, ID3D11ShaderResourceView* normalSRV, ID3D11SamplerState* SS);
	~Material();

	SimpleVertexShader* GetVertexShader();
	SimplePixelShader* GetPixelShader();
	ID3D11ShaderResourceView* GetShaderResourceView();
	ID3D11ShaderResourceView* GetNormalShaderResourceView();
	ID3D11SamplerState* GetSamplerState();

private:
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	ID3D11ShaderResourceView* shaderResourceView;
	ID3D11ShaderResourceView* normalShaderResourceView;
	ID3D11SamplerState* samplerState;
};

